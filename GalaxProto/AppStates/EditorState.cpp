#include "pch.h"
#include "AppStates.h"
#include "EditorState.h"
#include "AppContext.h"
#include "AppResources.h"
#include "GameLogic.h"
#include "DebugDraw.h"
#include "UIDebugDraw.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
// TODO:
//------------------------------------------------------------------------------
// - Enemies mapEnemyToPath can now use indices to the pathPool
//
// - Handle deletion of formation that is in-use
// - Handle deletion of path that is in-use
//			*** How can we Delete without invalidating the level data? ***
//						- on delete, scan through all levels for uses of the formation
//						- give them all a special 'null' idx. I.e. to a formation that is
// never deleted.
//
// - Customise names for formations and paths
//
// - PATH EDITOR [special visual editor for waypoints]
//	- Select, Create, Delete, Move [Points]
//
// - Play current selection in background (i.e. level, formation, path)
//
// - Load/Save All  (Define human editable file format)
//------------------------------------------------------------------------------
namespace
{
constexpr float MIN_SPAWN_TIME = 0.01f;		 // zero is disabled
constexpr float CAMERA_SPEED_X = 1.0f;
constexpr float CAMERA_SPEED_Y = 1.0f;

const float MODE_BUTTON_WIDTH			 = 300.0f;
const float MODE_BUTTON_HEIGHT		 = 80.0f;
const float MODE_BUTTON_POSITION_Y = 40.0f;

const float MAIN_AREA_START_X = 20.0f;
const float MAIN_AREA_START_Y
	= MODE_BUTTON_POSITION_Y + (1.5f * MODE_BUTTON_HEIGHT);

const DirectX::XMVECTORF32 SELECTED_ITEM_COLOR = DirectX::Colors::White;
const DirectX::XMVECTORF32 NORMAL_ITEM_COLOR = DirectX::Colors::MediumVioletRed;

//------------------------------------------------------------------------------
// Current menu selections
size_t g_levelIdx			= 0;
size_t g_formationIdx = 0;
size_t g_pathIdx			= 0;

std::vector<Wave>&
currentLevelWavesRef(GameLogic& logic)
{
	auto& levels = logic.m_enemies.debug_getCurrentLevels();
	ASSERT(g_levelIdx < levels.size());
	return levels[g_levelIdx].waves;
}

Wave&
currentLevelWaveRef(size_t waveIdx, GameLogic& logic)
{
	auto& waves = currentLevelWavesRef(logic);
	ASSERT(waveIdx < waves.size());
	return waves[waveIdx];
}

Formation&
currentFormationRef(GameLogic& logic)
{
	auto& formations = logic.m_enemies.debug_getFormations();
	ASSERT(g_formationIdx < formations.size());
	return formations[g_formationIdx];
}

FormationSection&
currentFormationSectionRef(size_t sectionIdx, GameLogic& logic)
{
	auto& formation = currentFormationRef(logic);

	ASSERT(sectionIdx < formation.sections.size());
	return formation.sections[sectionIdx];
}

};		// anon namespace

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct Modes;

struct IMode
{
	Modes& m_modes;
	AppContext& m_context;
	AppResources& m_resources;
	GameLogic& m_gameLogic;

	ui::Text m_controlInfo;
	size_t m_menuSize		 = 0;
	size_t m_selectedIdx = 0;

	IMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: m_modes(modes)
			, m_context(context)
			, m_resources(resources)
			, m_gameLogic(logic)
	{
	}

	virtual ~IMode() = default;

	virtual void onEnterMode(bool isNavigatingForward = true)
	{
		if (isNavigatingForward)
		{
			m_selectedIdx = 0;
		}
		updateMenuSize();
	}

	virtual std::wstring controlInfoText() const				= 0;
	virtual std::wstring menuTitle() const							= 0;
	virtual std::wstring itemName(size_t itemIdx) const = 0;

	virtual void onBack() {}
	virtual void onCreate() {}
	virtual void onDeleteItem(size_t itemIdx) { UNREFERENCED_PARAMETER(itemIdx); }
	virtual void onItemSelected(size_t itemIdx)
	{
		UNREFERENCED_PARAMETER(itemIdx);
	};
	virtual void onPlus() {}
	virtual void onSubtract() {}
	virtual void onPgUp() {}
	virtual void onPgDn() {}
	virtual void onLeft() {}
	virtual void onRight() {}
	virtual void onHome() {}
	virtual void onEnd() {}

	virtual size_t numMenuItems() const = 0;

	void init();
	void handleInput(const DX::StepTimer& timer);
	void render();

	void updateMenuSize()
	{
		m_menuSize = numMenuItems() + 1;		// +1 for CREATE button
	}
};

//------------------------------------------------------------------------------
void
IMode::init()
{
	using DirectX::SimpleMath::Vector2;
	using DirectX::XMVECTOR;

	m_controlInfo.text = controlInfoText();
	m_controlInfo.font = m_resources.fontMono8pt.get();

	XMVECTOR dimensions
		= m_controlInfo.font->MeasureString(m_controlInfo.text.c_str());
	const float height = DirectX::XMVectorGetY(dimensions);
	m_controlInfo.position
		= Vector2(MAIN_AREA_START_X, MAIN_AREA_START_Y - (2 * height));
	m_controlInfo.origin = Vector2(0.0f, 0.0f);
	m_controlInfo.color	= DirectX::Colors::MediumVioletRed;
}

//------------------------------------------------------------------------------
void
IMode::handleInput(const DX::StepTimer& timer)
{
	TRACE
	UNREFERENCED_PARAMETER(timer);
	auto& kb = m_resources.kbTracker;
	using DirectX::Keyboard;

	if (kb.IsKeyPressed(Keyboard::Escape))
	{
		onBack();
	}

	if (kb.IsKeyPressed(Keyboard::Up))
	{
		const size_t lastItemIdx = (m_menuSize > 0) ? m_menuSize - 1 : 0;
		m_selectedIdx = (m_selectedIdx > 0) ? m_selectedIdx - 1 : lastItemIdx;
	}
	else if (kb.IsKeyPressed(Keyboard::Down))
	{
		const size_t lastItemIdx = (m_menuSize > 0) ? m_menuSize - 1 : 0;

		m_selectedIdx = (m_selectedIdx < lastItemIdx) ? m_selectedIdx + 1 : 0;
	}

	if (kb.IsKeyPressed(Keyboard::Left))
	{
		onLeft();
	}
	else if (kb.IsKeyPressed(Keyboard::Right))
	{
		onRight();
	}
	if (kb.IsKeyPressed(Keyboard::Add) || kb.IsKeyPressed(Keyboard::OemPlus))
	{
		onPlus();
	}
	else if (
		kb.IsKeyPressed(Keyboard::Subtract) || kb.IsKeyPressed(Keyboard::OemMinus))
	{
		onSubtract();
	}
	if (kb.IsKeyPressed(Keyboard::PageUp))
	{
		onPgUp();
	}
	else if (kb.IsKeyPressed(Keyboard::PageDown))
	{
		onPgDn();
	}
	if (kb.IsKeyPressed(Keyboard::Home))
	{
		onHome();
	}
	else if (kb.IsKeyPressed(Keyboard::End))
	{
		onEnd();
	}

	if (
		kb.IsKeyPressed(Keyboard::LeftControl) || kb.IsKeyPressed(Keyboard::Space)
		|| kb.IsKeyPressed(Keyboard::Enter))
	{
		const size_t createItemIdx = (m_menuSize > 0) ? m_menuSize - 1 : 0;
		if (m_selectedIdx == createItemIdx)
		{
			onCreate();
			updateMenuSize();
			m_selectedIdx = (m_menuSize > 0) ? m_menuSize - 1 : 0;
		}
		else
		{
			onItemSelected(m_selectedIdx);
		}
	}
	if (kb.IsKeyPressed(Keyboard::Delete))
	{
		const size_t createItemIdx = (m_menuSize > 0) ? m_menuSize - 1 : 0;
		if (m_selectedIdx < createItemIdx)
		{
			onDeleteItem(m_selectedIdx);
			updateMenuSize();
		}
	}
}

//------------------------------------------------------------------------------
void
IMode::render()
{
	TRACE
	m_controlInfo.draw(*m_resources.m_spriteBatch);

	auto monoFont = m_resources.fontMono8pt.get();

	float yPos = MAIN_AREA_START_Y;
	const float yAscent
		= ceil(DirectX::XMVectorGetY(monoFont->MeasureString(L"X")));

	ui::Text uiText;
	uiText.font				= monoFont;
	uiText.position.x = MAIN_AREA_START_X;

	auto drawMenuItem =
		[&yAscent, &yPos, &uiText, &spriteBatch = m_resources.m_spriteBatch](
			bool isSelected, const std::wstring text)
	{
		uiText.color			= (isSelected) ? SELECTED_ITEM_COLOR : NORMAL_ITEM_COLOR;
		uiText.text				= std::move(text);
		uiText.position.y = yPos;
		yPos += yAscent;
		uiText.draw(*spriteBatch);
	};

	std::wstring title = menuTitle();
	size_t titleSize	 = title.size();
	drawMenuItem(false, std::move(title));
	drawMenuItem(false, std::wstring(titleSize, '-'));

	size_t idx = 0;
	while (idx < numMenuItems())
	{
		drawMenuItem((idx == m_selectedIdx), itemName(idx));
		idx++;
	}
	drawMenuItem((idx == m_selectedIdx), L"CREATE");
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct LevelListMode : public IMode
{
	LevelListMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IMode(modes, context, resources, logic)
	{
	}

	std::wstring controlInfoText() const override
	{
		return L"Navigate(Up/Down), Select(Enter), Delete(Del)";
	}
	std::wstring menuTitle() const override { return L"Level List"; }
	std::wstring itemName(size_t itemIdx) const override;

	void onCreate() override;
	void onDeleteItem(size_t itemIdx) override;
	void onItemSelected(size_t itemIdx) override;
	size_t numMenuItems() const override;
};

//------------------------------------------------------------------------------
struct LevelEditorMode : public IMode
{
	LevelEditorMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IMode(modes, context, resources, logic)
	{
	}

	std::wstring controlInfoText() const override
	{
		return L"Navigate(Up/Down), Select(Enter), Delete(Del), "
					 "Time(-/+), Formation(PgUp/PgDn), Back(Esc)";
	}
	std::wstring menuTitle() const override;
	std::wstring itemName(size_t itemIdx) const override;

	void onBack() override;
	void onCreate() override;
	void onDeleteItem(size_t itemIdx) override;
	void onPlus() override;
	void onSubtract() override;
	void onPgUp() override;
	void onPgDn() override;

	size_t numMenuItems() const override;
};

//------------------------------------------------------------------------------
struct FormationListMode : public IMode
{
	FormationListMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IMode(modes, context, resources, logic)
	{
	}

	std::wstring controlInfoText() const override
	{
		return L"Navigate(Up/Down), Select(Enter), Delete(Del)";
	}
	std::wstring menuTitle() const override { return L"Formation List"; }
	std::wstring itemName(size_t itemIdx) const override;

	void onCreate() override;
	void onDeleteItem(size_t itemIdx) override;
	void onItemSelected(size_t itemIdx) override;
	size_t numMenuItems() const override;
};

//------------------------------------------------------------------------------
struct FormationSectionEditorMode : public IMode
{
	FormationSectionEditorMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IMode(modes, context, resources, logic)
	{
	}

	std::wstring controlInfoText() const override
	{
		return L"Navigate(Up/Down), Select(Enter), Delete(Del), "
					 "Model(Home/End), Num Ships(-/+), Path(PgUp/PgDn), Back(Esc)";
	}
	std::wstring menuTitle() const override { return L"Formation Section"; }
	std::wstring itemName(size_t itemIdx) const override;

	void onBack() override;
	void onCreate() override;
	void onDeleteItem(size_t itemIdx) override;
	void onPlus() override;
	void onSubtract() override;
	void onPgUp() override;
	void onPgDn() override;
	void onHome() override;
	void onEnd() override;

	size_t numMenuItems() const override;
};

//------------------------------------------------------------------------------
struct PathListMode : public IMode
{
	PathListMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IMode(modes, context, resources, logic)
	{
	}

	std::wstring controlInfoText() const override
	{
		return L"Navigate(Up/Down), Select(Enter), Delete(Del)";
	}
	std::wstring menuTitle() const override { return L"Path List"; }
	std::wstring itemName(size_t itemIdx) const override;

	void onCreate() override;
	void onDeleteItem(size_t itemIdx) override;
	void onItemSelected(size_t itemIdx) override;
	size_t numMenuItems() const override;
};

//------------------------------------------------------------------------------
struct Modes
{
	LevelListMode levelListMode;
	LevelEditorMode levelEditorMode;

	FormationListMode formationListMode;
	FormationSectionEditorMode formationSectionEditorMode;

	PathListMode pathListMode;

	IMode* pCurrentMode = &levelListMode;

	Modes(AppContext& context, AppResources& resources, GameLogic& logic)
			: levelListMode(*this, context, resources, logic)
			, levelEditorMode(*this, context, resources, logic)
			, formationListMode(*this, context, resources, logic)
			, formationSectionEditorMode(*this, context, resources, logic)
			, pathListMode(*this, context, resources, logic)
	{
	}

	void initModes()
	{
		levelListMode.init();
		levelEditorMode.init();
		formationListMode.init();
		formationSectionEditorMode.init();
		pathListMode.init();
	}

	void enterMode(IMode* pNewMode, bool isNavigatingForward = true)
	{
		TRACE
		ASSERT(pNewMode);
		pCurrentMode = pNewMode;
		pCurrentMode->onEnterMode(isNavigatingForward);
	}
};

//------------------------------------------------------------------------------
std::wstring
LevelListMode::itemName(size_t itemIdx) const
{
	return fmt::format(L"Level_{}", itemIdx);
}

//------------------------------------------------------------------------------
void
LevelListMode::onCreate()
{
	m_gameLogic.m_enemies.debug_getCurrentLevels().emplace_back();
}

//------------------------------------------------------------------------------
void
LevelListMode::onDeleteItem(size_t itemIdx)
{
	auto& levels = m_gameLogic.m_enemies.debug_getCurrentLevels();
	ASSERT(itemIdx < levels.size());
	levels.erase(levels.begin() + itemIdx);
}

//------------------------------------------------------------------------------
size_t
LevelListMode::numMenuItems() const
{
	return m_gameLogic.m_enemies.debug_getCurrentLevels().size();
}

//------------------------------------------------------------------------------
void
LevelListMode::onItemSelected(size_t itemIdx)
{
	g_levelIdx = itemIdx;
	m_modes.enterMode(&m_modes.levelEditorMode);
};

//------------------------------------------------------------------------------
std::wstring
LevelEditorMode::menuTitle() const
{
	return fmt::format(L"Level_{}  (Spawn Time (sec) - Wave)", g_levelIdx);
}

//------------------------------------------------------------------------------
std::wstring
LevelEditorMode::itemName(size_t itemIdx) const
{
	const auto& wave = currentLevelWaveRef(itemIdx, m_gameLogic);
	auto& formations = m_gameLogic.m_enemies.debug_getFormations();
	ASSERT(wave.formationIdx < formations.size());

	return fmt::format(
		L"{:4}   {}", wave.spawnTimeS, formations[wave.formationIdx].id);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onBack()
{
	m_modes.enterMode(&m_modes.levelListMode, false);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onCreate()
{
	auto& waves = currentLevelWavesRef(m_gameLogic);
	float t			= (waves.empty()) ? MIN_SPAWN_TIME : waves.back().spawnTimeS;

	Wave newWave{t, 0};
	waves.emplace_back(newWave);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onDeleteItem(size_t itemIdx)
{
	auto& waves = currentLevelWavesRef(m_gameLogic);

	ASSERT(itemIdx < waves.size());
	waves.erase(waves.begin() + itemIdx);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPlus()
{
	auto& t = currentLevelWaveRef(m_selectedIdx, m_gameLogic).spawnTimeS;
	t += 1.0f;
	t = round(t);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onSubtract()
{
	auto& t = currentLevelWaveRef(m_selectedIdx, m_gameLogic).spawnTimeS;
	t -= 1.0f;
	if (t < MIN_SPAWN_TIME)
	{
		t = MIN_SPAWN_TIME;
	};
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPgUp()
{
	auto& formations = m_gameLogic.m_enemies.debug_getFormations();
	auto& curIdx = currentLevelWaveRef(m_selectedIdx, m_gameLogic).formationIdx;
	const size_t lastIdx = (formations.size() > 0) ? formations.size() - 1 : 0;

	curIdx = (curIdx > 0) ? curIdx - 1 : lastIdx;
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPgDn()
{
	auto& formations = m_gameLogic.m_enemies.debug_getFormations();
	auto& curIdx = currentLevelWaveRef(m_selectedIdx, m_gameLogic).formationIdx;
	const size_t lastIdx = (formations.size() > 0) ? formations.size() - 1 : 0;

	curIdx = (curIdx < lastIdx) ? curIdx + 1 : 0;
}

//------------------------------------------------------------------------------
size_t
LevelEditorMode::numMenuItems() const
{
	auto& waves = currentLevelWavesRef(m_gameLogic);
	return waves.size();
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
std::wstring
FormationListMode::itemName(size_t itemIdx) const
{
	auto& formations = m_gameLogic.m_enemies.debug_getFormations();
	ASSERT(itemIdx < formations.size());

	return formations[itemIdx].id;
}

//------------------------------------------------------------------------------
void
FormationListMode::onCreate()
{
	Formation formation{L"New"};
	m_gameLogic.m_enemies.debug_getFormations().emplace_back(formation);
}

//------------------------------------------------------------------------------
void
FormationListMode::onDeleteItem(size_t itemIdx)
{
	auto& formations = m_gameLogic.m_enemies.debug_getFormations();
	ASSERT(itemIdx < formations.size());

	formations.erase(formations.begin() + itemIdx);
	// TODO(James): Handle levels which were using this formation
}

//------------------------------------------------------------------------------
size_t
FormationListMode::numMenuItems() const
{
	return m_gameLogic.m_enemies.debug_getFormations().size();
}

//------------------------------------------------------------------------------
void
FormationListMode::onItemSelected(size_t itemIdx)
{
	g_formationIdx = itemIdx;
	m_modes.enterMode(&m_modes.formationSectionEditorMode);
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
std::wstring
FormationSectionEditorMode::itemName(size_t itemIdx) const
{
	auto& section = currentFormationSectionRef(itemIdx, m_gameLogic);

	auto& paths = m_gameLogic.m_enemies.debug_getPaths();
	ASSERT(section.pathIdx < paths.size());
	auto& path = paths[section.pathIdx];

	return fmt::format(
		L"{}: Model:{}, numShips:{}, Path:{} ",
		itemIdx,
		static_cast<int>(section.model),
		section.numShips,
		path.id);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onBack()
{
	m_modes.enterMode(&m_modes.formationListMode, false);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onCreate()
{
	auto& formation = currentFormationRef(m_gameLogic);

	FormationSection section;
	section.pathIdx	= 0;
	section.numShips = 3;
	section.model		 = ModelResource::Enemy1;
	formation.sections.emplace_back(section);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onDeleteItem(size_t itemIdx)
{
	auto& formation = currentFormationRef(m_gameLogic);
	formation.sections.erase(formation.sections.begin() + itemIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onPlus()
{
	auto& section = currentFormationSectionRef(m_selectedIdx, m_gameLogic);
	static const int MAX_NUM_SHIPS = 10;
	section.numShips
		= (section.numShips < MAX_NUM_SHIPS) ? section.numShips + 1 : MAX_NUM_SHIPS;
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onSubtract()
{
	auto& section = currentFormationSectionRef(m_selectedIdx, m_gameLogic);
	if (section.numShips > 1)
	{
		--section.numShips;
	}
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onPgUp()
{
	auto& section = currentFormationSectionRef(m_selectedIdx, m_gameLogic);
	auto& paths		= m_gameLogic.m_enemies.debug_getPaths();

	auto& curIdx				 = section.pathIdx;
	const size_t lastIdx = (paths.size() > 0) ? paths.size() - 1 : 0;

	curIdx = (curIdx > 0) ? curIdx - 1 : lastIdx;
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onPgDn()
{
	auto& section = currentFormationSectionRef(m_selectedIdx, m_gameLogic);
	auto& paths		= m_gameLogic.m_enemies.debug_getPaths();

	auto& curIdx				 = section.pathIdx;
	const size_t lastIdx = (paths.size() > 0) ? paths.size() - 1 : 0;

	curIdx = (curIdx < lastIdx) ? curIdx + 1 : 0;
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onHome()
{
	auto& section		 = currentFormationSectionRef(m_selectedIdx, m_gameLogic);
	const int minIdx = static_cast<int>(ModelResource::Enemy1);
	const int maxIdx = static_cast<int>(ModelResource::Player);

	int curIdx		= static_cast<int>(section.model);
	curIdx				= (curIdx > minIdx) ? curIdx - 1 : maxIdx;
	section.model = static_cast<ModelResource>(curIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onEnd()
{
	auto& section		 = currentFormationSectionRef(m_selectedIdx, m_gameLogic);
	const int minIdx = static_cast<int>(ModelResource::Enemy1);
	const int maxIdx = static_cast<int>(ModelResource::Player);

	int curIdx		= static_cast<int>(section.model);
	curIdx				= (curIdx < maxIdx) ? curIdx + 1 : minIdx;
	section.model = static_cast<ModelResource>(curIdx);
}

//------------------------------------------------------------------------------
size_t
FormationSectionEditorMode::numMenuItems() const
{
	auto& formation = currentFormationRef(m_gameLogic);
	return formation.sections.size();
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
std::wstring
PathListMode::itemName(size_t itemIdx) const
{
	auto& paths = m_gameLogic.m_enemies.debug_getPaths();
	ASSERT(itemIdx < paths.size());

	return paths[itemIdx].id;
}

//------------------------------------------------------------------------------
void
PathListMode::onCreate()
{
	Path path{L"New"};
	m_gameLogic.m_enemies.debug_getPaths().emplace_back(path);
}

//------------------------------------------------------------------------------
void
PathListMode::onDeleteItem(size_t itemIdx)
{
	auto& paths = m_gameLogic.m_enemies.debug_getPaths();
	ASSERT(itemIdx < paths.size());

	paths.erase(paths.begin() + itemIdx);
	// TODO(James): Handle formations which were using this path
}

//------------------------------------------------------------------------------
size_t
PathListMode::numMenuItems() const
{
	return m_gameLogic.m_enemies.debug_getPaths().size();
}

//------------------------------------------------------------------------------
void
PathListMode::onItemSelected(size_t itemIdx)
{
	g_pathIdx = itemIdx;

	// TODO(James): Implement a path editor
	// m_modes.enterMode(&m_modes.formationSectionEditorMode);
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct EditorState::Impl
{
	struct ModeButton : public ui::Button
	{
		IMode* pGotoMode = nullptr;		 // TODO(James): <NOT_NULL>
	};

	AppContext& m_context;
	AppResources& m_resources;
	GameLogic& m_gameLogic;

	Modes m_modes;

	static constexpr size_t NUM_BUTTONS = 3;
	std::array<ModeButton, NUM_BUTTONS> m_modeButtons;

	Impl(AppContext& context, AppResources& resources, GameLogic& logic)
			: m_context(context)
			, m_resources(resources)
			, m_gameLogic(logic)
			, m_modes(context, resources, logic)
	{
	}
	void init()
	{
		m_modes.initModes();
		setupModeMenu();
	}

	void setupModeMenu();

	void handleInput(const DX::StepTimer& timer);
	void handleModeMenuInput(const DX::StepTimer& timer);

	void render();
	void renderModeMenu();
};

//------------------------------------------------------------------------------
void
EditorState::Impl::setupModeMenu()
{
	TRACE
	const float BUTTON_STEP_X			= m_context.screenWidth / NUM_BUTTONS;
	const float BUTTON_POSITION_X = BUTTON_STEP_X / 2.0f;

	float xPos					= BUTTON_POSITION_X;
	auto positionButton = [&](auto& button) {
		using DirectX::SimpleMath::Vector2;
		button.position.x = xPos - (MODE_BUTTON_WIDTH * 0.5f);
		button.position.y = MODE_BUTTON_POSITION_Y;
		button.size				= Vector2(MODE_BUTTON_WIDTH, MODE_BUTTON_HEIGHT);

		button.uiText.font	= m_resources.font32pt.get();
		button.uiText.color = DirectX::Colors::Yellow;
		button.centerText();
		xPos += BUTTON_STEP_X;
	};

	m_modeButtons[0].pGotoMode	 = &m_modes.levelListMode;
	m_modeButtons[0].uiText.text = L"Levels";

	m_modeButtons[1].pGotoMode	 = &m_modes.formationListMode;
	m_modeButtons[1].uiText.text = L"Formations";

	m_modeButtons[2].pGotoMode	 = &m_modes.pathListMode;
	m_modeButtons[2].uiText.text = L"Paths";
	for (auto& btn : m_modeButtons)
	{
		positionButton(btn);
	}

	m_modes.enterMode(m_modes.pCurrentMode);
	for (auto& btn : m_modeButtons)
	{
		if (btn.pGotoMode == m_modes.pCurrentMode)
		{
			btn.appearance = ui::Button::Appearance::Selected;
		}
	}
}

//------------------------------------------------------------------------------
void
EditorState::Impl::handleInput(const DX::StepTimer& timer)
{
	TRACE
	handleModeMenuInput(timer);
	m_modes.pCurrentMode->handleInput(timer);
}

//------------------------------------------------------------------------------
void
EditorState::Impl::handleModeMenuInput(const DX::StepTimer& timer)
{
	TRACE
	UNREFERENCED_PARAMETER(timer);

	const auto& mouseBtns	= m_resources.mouseTracker;
	const auto& mouseState = m_resources.m_mouse->GetState();
	for (auto& button : m_modeButtons)
	{
		if (button.isPointInside(
					static_cast<float>(mouseState.x), static_cast<float>(mouseState.y)))
		{
			using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;
			if (mouseBtns.leftButton == ButtonState::PRESSED)
			{
				for (auto& btn : m_modeButtons)
				{
					btn.appearance = ui::Button::Appearance::Normal;
				}
				button.appearance = ui::Button::Appearance::Selected;
				m_modes.enterMode(button.pGotoMode);
			}

			if (button.appearance != ui::Button::Appearance::Selected)
			{
				button.appearance = ui::Button::Appearance::HoverOver;
			}
		}
		else
		{
			if (button.appearance != ui::Button::Appearance::Selected)
			{
				button.appearance = ui::Button::Appearance::Normal;
			}
		}
	}

	auto& kb = m_resources.kbTracker;
	using DirectX::Keyboard;
	if (kb.IsKeyPressed(Keyboard::Tab))
	{
		bool getIteratorOnNextLoop = false;
		auto nextButtonIt					 = m_modeButtons.end();
		for (auto it = m_modeButtons.begin(); it != m_modeButtons.end(); ++it)
		{
			if (getIteratorOnNextLoop)
			{
				getIteratorOnNextLoop = false;
				nextButtonIt					= it;
			}

			auto& button = *it;
			if (button.appearance == ui::Button::Appearance::Selected)
			{
				button.appearance			= ui::Button::Appearance::Normal;
				getIteratorOnNextLoop = true;
			}
		}
		if (nextButtonIt == m_modeButtons.end())
		{
			nextButtonIt = m_modeButtons.begin();
		}
		auto& nextButton			= *nextButtonIt;
		nextButton.appearance = ui::Button::Appearance::Selected;
		m_modes.enterMode(nextButton.pGotoMode);
	}		 // Tab Key
}

//------------------------------------------------------------------------------
void
EditorState::Impl::render()
{
	TRACE
	renderModeMenu();
	m_modes.pCurrentMode->render();
}

//------------------------------------------------------------------------------
void
EditorState::Impl::renderModeMenu()
{
	TRACE
	for (auto& btn : m_modeButtons)
	{
		btn.draw(*m_resources.m_batch, *m_resources.m_spriteBatch);
	}
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
EditorState::EditorState(
	AppStates& states,
	AppContext& context,
	AppResources& resources,
	GameLogic& logic)
		: IAppState(states, context, resources, logic)
		, m_pImpl(std::make_unique<Impl>(context, resources, logic))
{
}

//------------------------------------------------------------------------------
EditorState::~EditorState() = default;

//------------------------------------------------------------------------------
void
EditorState::handleInput(const DX::StepTimer& timer)
{
	TRACE
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	auto& kb			= m_resources.kbTracker;
	auto& kbState = m_resources.kbTracker.lastState;
	using DirectX::Keyboard;

	// Debug Controls
	if (kb.IsKeyPressed(Keyboard::F2))
	{
		m_context.debugDraw = !m_context.debugDraw;
	}

	if (kb.IsKeyPressed(Keyboard::F3))
	{
		m_states.changeState(m_states.previousState());
	}

	if (kbState.W)
	{
		m_context.cameraRotationX -= elapsedTimeS * CAMERA_SPEED_X;
		m_context.updateViewMatrix();
	}
	else if (kbState.S)
	{
		m_context.cameraRotationX += elapsedTimeS * CAMERA_SPEED_X;
		m_context.updateViewMatrix();
	}

	if (kbState.A)
	{
		m_context.cameraRotationY -= elapsedTimeS * CAMERA_SPEED_Y;
		m_context.updateViewMatrix();
	}
	else if (kbState.D)
	{
		m_context.cameraRotationY += elapsedTimeS * CAMERA_SPEED_Y;
		m_context.updateViewMatrix();
	}
	if (kbState.R)
	{
		m_context.cameraRotationX = 0.0f;
		m_context.cameraRotationY = 0.0f;
		m_context.updateViewMatrix();
	}

	m_pImpl->handleInput(timer);
}

//------------------------------------------------------------------------------
void
EditorState::update(const DX::StepTimer& timer)
{
	TRACE
	m_resources.starField->update(timer);
}

//------------------------------------------------------------------------------
void
EditorState::render()
{
	TRACE
	renderStarField();

	ui::DebugDraw ui(m_context, m_resources);
	ui.begin2D();
	m_pImpl->render();
	ui.end2D();
}

//------------------------------------------------------------------------------
void
EditorState::renderStarField()
{
	TRACE
	auto& spriteBatch = m_resources.m_spriteBatch;

	spriteBatch->Begin();
	m_resources.starField->render(*spriteBatch);
	spriteBatch->End();
}

//------------------------------------------------------------------------------
void
EditorState::load()
{
	TRACE
}

//------------------------------------------------------------------------------
void
EditorState::unload()
{
	TRACE
}

//------------------------------------------------------------------------------
bool
EditorState::isLoaded() const
{
	return false;
}

//------------------------------------------------------------------------------
void
EditorState::enter()
{
	TRACE
	m_pImpl->init();
}

//------------------------------------------------------------------------------
void
EditorState::exit()
{
	TRACE
}

//------------------------------------------------------------------------------
