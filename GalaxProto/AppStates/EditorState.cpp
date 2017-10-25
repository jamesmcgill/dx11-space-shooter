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
//------------------------------------------------------------------------------
// struct Waypoint
//{
//	DirectX::SimpleMath::Vector3 wayPoint = {};
//	DirectX::SimpleMath::Vector3 controlPoint = {};
//};
//
// struct EnemyFormationSection
//{
//	const std::vector<Waypoint> waypoints;
//	int numShips;
//	ModelResource model;
//};
//
// struct EnemyFormation {
//	std::wstring id;
//	std::vector<EnemyFormationSection> sections;
//};
//
// using EnemyFormationPool = std::vector<EnemyFormation>;
//------------------------------------------------------------------------------

//	3 distinct editors (3 buttons at top + load/save)
// Load/Save All  (Define human editable file format)

// 1) Level Editor:
// LEVEL LIST			- Create, Delete + Select (to WaveList)
// WAVE LIST			- Create, Delete, Nav Back(to LevelList)
//											- Inc/Dec Time
//											- Inc/Dec FormationID

// 2) Formation Editor
// FORMATION LIST - Create, Delete, + Select (to FormationEditor)+  [Type Name]
//			*** How can we Delete without invalidating the level data? ***
//
// FORMATION EDITOR - Create, Delete + Select Section(->FormationSectionEditor)
//
// FORMATION SECTION EDITOR
//		Inc/Dec ShipCount
//		Inc/Dec Model (from list)
//		Inc/Dec Path (from list)

// 3) Path Editor
// PATH LIST			- Select, Create, Delete Path, [Type Name]
//
// PATH EDITOR [special visual editor]
//	- Select, Create, Delete, Move Points

// List Controls
// Up/Down			-> Move Selection  / Inc/Decrement editable control
// Enter				-> Edit selected / 'Create' if selected
// Delete				-> Delete selected
// Left/Right		-> Cycle editables controls

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

size_t g_levelIdx			= 0;
size_t g_formationIdx = 0;
size_t g_pathIdx			= 0;

std::vector<EnemyWave>&
currentLevelWavesRef(GameLogic& logic)
{
	auto& levels = logic.m_enemies.debug_getCurrentLevels();
	ASSERT(g_levelIdx < levels.size());

	return levels[g_levelIdx].waves;
}

EnemyWave&
currentLevelWaveRef(size_t waveIdx, GameLogic& logic)
{
	auto& waves = currentLevelWavesRef(logic);
	ASSERT(waveIdx < waves.size());
	return waves[waveIdx];
}
};

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

	virtual void onEnterMode() { updateMenuSize(); }

	virtual std::wstring controlInfoText() const				= 0;
	virtual std::wstring menuTitle() const							= 0;
	virtual std::wstring itemName(size_t itemIdx) const = 0;

	virtual void onBack() {}
	virtual void onCreate() {}
	virtual void onDeleteItem(size_t itemIdx) { UNREFERENCED_PARAMETER(itemIdx); }
	virtual void onItemSelected(size_t itemIdx){};
	virtual void onAdd(size_t itemIdx) { UNREFERENCED_PARAMETER(itemIdx); }
	virtual void onSubtract(size_t itemIdx) { UNREFERENCED_PARAMETER(itemIdx); }
	virtual void onPgUp(size_t itemIdx) { UNREFERENCED_PARAMETER(itemIdx); }
	virtual void onPgDn(size_t itemIdx) { UNREFERENCED_PARAMETER(itemIdx); }

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
	}
	else if (kb.IsKeyPressed(Keyboard::Right))
	{
	}
	if (kb.IsKeyPressed(Keyboard::Add))
	{
		onAdd(m_selectedIdx);
	}
	else if (kb.IsKeyPressed(Keyboard::Subtract))
	{
		onSubtract(m_selectedIdx);
	}
	if (kb.IsKeyPressed(Keyboard::PageUp))
	{
		onPgUp(m_selectedIdx);
	}
	else if (kb.IsKeyPressed(Keyboard::PageDown))
	{
		onPgDn(m_selectedIdx);
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
		return L"Navigate(Up/Down), Select(Enter)";
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

	void onEnterMode() override
	{
		IMode::onEnterMode();
		m_selectedIdx = 0;
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
	void onAdd(size_t itemIdx) override;
	void onSubtract(size_t itemIdx) override;
	void onPgUp(size_t itemIdx) override;
	void onPgDn(size_t itemIdx) override;

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

	std::wstring controlInfoText() const override { return L""; }
	std::wstring menuTitle() const override { return L"Formation List"; }
	std::wstring itemName(size_t itemIdx) const override { return L"Formation"; }
	size_t numMenuItems() const override { return 0; }
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

	std::wstring controlInfoText() const override { return L""; }
	std::wstring menuTitle() const override { return L"Path List"; }
	std::wstring itemName(size_t itemIdx) const override { return L"Path"; }
	size_t numMenuItems() const override { return 0; }
};

//------------------------------------------------------------------------------
struct Modes
{
	LevelListMode levelListMode;
	LevelEditorMode levelEditorMode;

	FormationListMode formationListMode;
	PathListMode pathListMode;

	IMode* pCurrentMode = &levelListMode;

	Modes(AppContext& context, AppResources& resources, GameLogic& logic)
			: levelListMode(*this, context, resources, logic)
			, levelEditorMode(*this, context, resources, logic)
			, formationListMode(*this, context, resources, logic)
			, pathListMode(*this, context, resources, logic)
	{
	}

	void initModes()
	{
		levelListMode.init();
		levelEditorMode.init();
		formationListMode.init();
		pathListMode.init();
	}

	void enterMode(IMode* pNewMode)
	{
		TRACE
		ASSERT(pNewMode);
		pCurrentMode = pNewMode;
		pCurrentMode->onEnterMode();
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
	m_modes.enterMode(&m_modes.levelListMode);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onCreate()
{
	auto& waves = currentLevelWavesRef(m_gameLogic);
	float t			= (waves.empty()) ? MIN_SPAWN_TIME : waves.back().spawnTimeS;

	EnemyWave newWave{t, 0};
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
LevelEditorMode::onAdd(size_t itemIdx)
{
	auto& t = currentLevelWaveRef(itemIdx, m_gameLogic).spawnTimeS;
	t += 1.0f;
	t = round(t);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onSubtract(size_t itemIdx)
{
	auto& t = currentLevelWaveRef(itemIdx, m_gameLogic).spawnTimeS;
	t -= 1.0f;
	if (t < MIN_SPAWN_TIME)
	{
		t = MIN_SPAWN_TIME;
	};
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPgUp(size_t itemIdx)
{
	auto& formations		 = m_gameLogic.m_enemies.debug_getFormations();
	auto& curIdx				 = currentLevelWaveRef(itemIdx, m_gameLogic).formationIdx;
	const size_t lastIdx = (formations.size() > 0) ? formations.size() - 1 : 0;

	curIdx = (curIdx > 0) ? curIdx - 1 : lastIdx;
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPgDn(size_t itemIdx)
{
	auto& formations		 = m_gameLogic.m_enemies.debug_getFormations();
	auto& curIdx				 = currentLevelWaveRef(itemIdx, m_gameLogic).formationIdx;
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
