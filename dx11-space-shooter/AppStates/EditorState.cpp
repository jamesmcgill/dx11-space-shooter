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
// - PATH EDITOR [special visual editor for waypoints]
//	- Select, Create, Delete, Move [Points]
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
	size_t m_firstIdx		 = 0;
	size_t m_lastIdx		 = 0;
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
			m_selectedIdx = firstMenuIdx();
		}
		updateIndices();
		onItemSelected();
	}

	virtual std::wstring controlInfoText() const				= 0;
	virtual std::wstring menuTitle() const							= 0;
	virtual std::wstring itemName(size_t itemIdx) const = 0;
	virtual std::wstring itemNameToDisplay(size_t itemIdx) const
	{
		return itemName(itemIdx);
	};

	virtual void onBack() {}
	virtual void onCreate() {}
	virtual void onDeleteItem(size_t itemIdx) { UNREFERENCED_PARAMETER(itemIdx); }
	virtual void onItemSelected() = 0;
	virtual void onItemCommand() {}

	virtual void onPlus() {}
	virtual void onSubtract() {}
	virtual void onPgUp() {}
	virtual void onPgDn() {}
	virtual void onLeft() {}
	virtual void onRight() {}
	virtual void onHome() {}
	virtual void onEnd() {}

	virtual size_t firstMenuIdx() const { return 0; }
	virtual size_t lastItemIdx() const = 0;

	void init();
	virtual void update(const DX::StepTimer& timer)
	{
		UNREFERENCED_PARAMETER(timer);
	}
	virtual void handleInput(const DX::StepTimer& timer);
	void render();

	void updateIndices()
	{
		m_firstIdx = firstMenuIdx();
		m_lastIdx	= lastItemIdx() + 1;		 // +1 for CREATE button
	}

	void jumpToLevelWave(const size_t levelIdx, const size_t waveIdx)
	{
		m_gameLogic.reset();
		m_gameLogic.m_enemies.jumpToLevel(levelIdx);
		m_gameLogic.m_enemies.jumpToWave(waveIdx);
	}

	void spawnFormation(size_t formationIdx)
	{
		m_gameLogic.m_enemies.reset();
		m_gameLogic.m_enemies.spawnFormation(formationIdx, 0.0f);
	}

	void spawnPath(size_t pathIdx)
	{
		m_gameLogic.m_enemies.reset();
		m_gameLogic.m_enemies.spawnFormationSection(
			5, pathIdx, ModelResource::Enemy9, 0.0f);
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
	const auto& kb = m_resources.kbTracker;
	using DirectX::Keyboard;

	// Navigation Controls
	if (kb.IsKeyPressed(Keyboard::Escape))
	{
		onBack();
	}

	if (kb.IsKeyPressed(Keyboard::Up))
	{
		m_selectedIdx
			= (m_selectedIdx > m_firstIdx) ? m_selectedIdx - 1 : m_lastIdx;
		onItemSelected();
	}
	else if (kb.IsKeyPressed(Keyboard::Down))
	{
		m_selectedIdx
			= (m_selectedIdx < m_lastIdx) ? m_selectedIdx + 1 : m_firstIdx;
		onItemSelected();
	}

	if (
		kb.IsKeyPressed(Keyboard::LeftControl) || kb.IsKeyPressed(Keyboard::Space)
		|| kb.IsKeyPressed(Keyboard::Enter))
	{
		const size_t& createItemIdx = m_lastIdx;
		if (m_selectedIdx == createItemIdx)
		{
			onCreate();
			updateIndices();
			m_selectedIdx = m_lastIdx;
			onItemSelected();
		}
		else
		{
			onItemCommand();
		}
	}

	// Edit Item Controls
	const size_t& createItemIdx = m_lastIdx;
	if (m_selectedIdx < createItemIdx)
	{
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
			kb.IsKeyPressed(Keyboard::Subtract)
			|| kb.IsKeyPressed(Keyboard::OemMinus))
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

		if (kb.IsKeyPressed(Keyboard::Delete))
		{
			onDeleteItem(m_selectedIdx);
			updateIndices();
			onItemSelected();
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
	const float yAscent
		= ceil(DirectX::XMVectorGetY(monoFont->MeasureString(L"X")));

	using DirectX::SimpleMath::Vector2;
	Vector2 position = {MAIN_AREA_START_X, MAIN_AREA_START_Y};
	Vector2 origin	 = Vector2(0.f, 0.f);
	Vector2 scale		 = Vector2(1.f, 1.f);

	auto drawMenuItem = [&, &spriteBatch = m_resources.m_spriteBatch](
		bool isSelected, const std::wstring& text)
	{
		monoFont->DrawString(
			spriteBatch.get(),
			text.c_str(),
			position,
			(isSelected) ? SELECTED_ITEM_COLOR : NORMAL_ITEM_COLOR,
			0.0f,
			origin,
			scale,
			DirectX::SpriteEffects_None,
			ui::Layer::L5_Default);
		position.y += yAscent;
	};

	std::wstring title = menuTitle();
	size_t titleSize	 = title.size();
	drawMenuItem(false, title);
	drawMenuItem(false, std::wstring(titleSize, '-'));

	size_t idx = m_firstIdx;
	while (idx < m_lastIdx)
	{
		drawMenuItem((idx == m_selectedIdx), itemNameToDisplay(idx));
		idx++;
	}
	drawMenuItem((idx == m_selectedIdx), L"CREATE");
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct IRenameableMode : public IMode
{
	static const size_t RENAME_DISABLED = static_cast<size_t>(-1);

	size_t m_renamingIdx = RENAME_DISABLED;
	std::wstring m_renameBuffer;

	IRenameableMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IMode(modes, context, resources, logic)
	{
	}

	// IMode
	void onEnterMode(bool isNavigatingForward = true) override
	{
		m_renamingIdx = RENAME_DISABLED;
		IMode::onEnterMode(isNavigatingForward);
	}

	void handleInput(const DX::StepTimer& timer) override;

	// IRenameableMode
	void handleInput_renaming(const DX::StepTimer& timer);
	virtual void setItemName(size_t itemIdx, std::wstring newName) = 0;

	std::wstring renameText() const
	{
		std::wstring rename = m_renameBuffer;
		rename.push_back(L'|');		 // Mark that we are editting
		return rename;
	}
};

//------------------------------------------------------------------------------
void
IRenameableMode::handleInput(const DX::StepTimer& timer)
{
	TRACE
	const auto& kb = m_resources.kbTracker;
	using DirectX::Keyboard;

	// Steal all input if in renaming mode
	if (m_renamingIdx != RENAME_DISABLED)
	{
		handleInput_renaming(timer);
		return;		 // Don't process any other input
	}

	if (kb.IsKeyPressed(Keyboard::E))
	{
		const size_t& createItemIdx = m_lastIdx;
		if (m_selectedIdx != createItemIdx)
		{
			m_renamingIdx	= m_selectedIdx;
			m_renameBuffer = itemName(m_selectedIdx);
		}
	}

	IMode::handleInput(timer);
}

//------------------------------------------------------------------------------
void
IRenameableMode::handleInput_renaming(const DX::StepTimer& timer)
{
	TRACE
	UNREFERENCED_PARAMETER(timer);
	const auto& kb		 = m_resources.kbTracker;
	const auto kbState = m_resources.m_keyboard->GetState();
	using DirectX::Keyboard;

	auto isCapitalised = [&kbState]() -> bool {
		return kbState.IsKeyDown(Keyboard::CapsLock)
					 || kbState.IsKeyDown(Keyboard::LeftShift)
					 || kbState.IsKeyDown(Keyboard::RightShift);
	};

	for (int k = (int)Keyboard::A; k <= (int)Keyboard::Z; ++k)
	{
		if (kb.IsKeyPressed((Keyboard::Keys)k))
		{
			bool isCapital	 = isCapitalised();
			wchar_t baseChar = (isCapital) ? L'A' : L'a';
			m_renameBuffer.push_back(baseChar + wchar_t(k - (int)Keyboard::A));
		}
	}

	for (int k = (int)Keyboard::D0; k <= (int)Keyboard::D9; ++k)
	{
		if (kb.IsKeyPressed((Keyboard::Keys)k))
		{
			m_renameBuffer.push_back(L'0' + wchar_t(k - (int)Keyboard::D0));
		}
	}

	for (int k = (int)Keyboard::NumPad0; k <= (int)Keyboard::NumPad9; ++k)
	{
		if (kb.IsKeyPressed((Keyboard::Keys)k))
		{
			m_renameBuffer.push_back(L'0' + wchar_t(k - (int)Keyboard::NumPad0));
		}
	}

	if (kb.IsKeyPressed(Keyboard::OemMinus))
	{
		m_renameBuffer.push_back(isCapitalised() ? L'_' : L'-');
	}

	if (kb.IsKeyPressed(Keyboard::Escape))
	{
		m_renamingIdx = RENAME_DISABLED;
	}

	if (kb.IsKeyPressed(Keyboard::Delete) || kb.IsKeyPressed(Keyboard::Back))
	{
		if (!m_renameBuffer.empty())
		{
			m_renameBuffer.pop_back();
		}
	}

	if (kb.IsKeyPressed(Keyboard::Enter))
	{
		setItemName(m_renamingIdx, m_renameBuffer);
		m_renamingIdx = RENAME_DISABLED;
	}
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
	void onItemSelected() override;
	void onDeleteItem(size_t itemIdx) override;
	void onItemCommand() override;
	size_t lastItemIdx() const override;

	void update(const DX::StepTimer& timer) override
	{
		UNREFERENCED_PARAMETER(timer);
		m_gameLogic.m_enemies.updateLevel();
	}
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
	void onItemSelected() override;
	void onPlus() override;
	void onSubtract() override;
	void onPgUp() override;
	void onPgDn() override;

	size_t lastItemIdx() const override;

	void update(const DX::StepTimer& timer) override
	{
		UNREFERENCED_PARAMETER(timer);
		m_gameLogic.m_enemies.updateLevel();
	}
};

//------------------------------------------------------------------------------
struct FormationListMode : public IRenameableMode
{
	FormationListMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IRenameableMode(modes, context, resources, logic)
	{
	}

	std::wstring controlInfoText() const override
	{
		return L"Navigate(Up/Down), Select(Enter), Delete(Del), Edit Name(E)";
	}
	std::wstring menuTitle() const override { return L"Formation List"; }
	std::wstring itemName(size_t itemIdx) const override;
	std::wstring itemNameToDisplay(size_t itemIdx) const override;
	void setItemName(size_t itemIdx, std::wstring newName) override;

	void onCreate() override;
	void onDeleteItem(size_t itemIdx) override;
	void onItemSelected() override;
	void onItemCommand() override;

	size_t firstMenuIdx() const override;
	size_t lastItemIdx() const override;
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
	void onItemSelected() override;
	void onPlus() override;
	void onSubtract() override;
	void onPgUp() override;
	void onPgDn() override;
	void onHome() override;
	void onEnd() override;

	size_t lastItemIdx() const override;
};

//------------------------------------------------------------------------------
struct PathListMode : public IRenameableMode
{
	PathListMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IRenameableMode(modes, context, resources, logic)
	{
	}

	std::wstring controlInfoText() const override
	{
		return L"Navigate(Up/Down), Select(Enter), Delete(Del), Edit Name(E)";
	}
	std::wstring menuTitle() const override { return L"Path List"; }
	std::wstring itemName(size_t itemIdx) const override;
	std::wstring itemNameToDisplay(size_t itemIdx) const override;
	void setItemName(size_t itemIdx, std::wstring newName) override;

	void onCreate() override;
	void onDeleteItem(size_t itemIdx) override;
	void onItemSelected() override;
	void onItemCommand() override;

	size_t firstMenuIdx() const override;
	size_t lastItemIdx() const override;
};

//------------------------------------------------------------------------------

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
LevelListMode::lastItemIdx() const
{
	return m_gameLogic.m_enemies.debug_getCurrentLevels().size() - 1;
}

//------------------------------------------------------------------------------
void
LevelListMode::onItemSelected()
{
	const size_t& createItemIdx = m_lastIdx;
	if (m_selectedIdx != createItemIdx)
	{
		jumpToLevelWave(m_selectedIdx, 0);
	}
}

//------------------------------------------------------------------------------
void
LevelListMode::onItemCommand()
{
	g_levelIdx = m_selectedIdx;
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

	size_t formationIdx = m_gameLogic.m_enemies.nullFormationIdx + 1;
	Wave newWave{t, formationIdx};
	waves.emplace_back(newWave);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onItemSelected()
{
	const size_t& createItemIdx = m_lastIdx;
	if (m_selectedIdx != createItemIdx)
	{
		jumpToLevelWave(g_levelIdx, m_selectedIdx);
	}
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

	const size_t firstIdx = m_gameLogic.m_enemies.nullFormationIdx + 1;
	const size_t lastIdx	= (formations.size() > 0) ? formations.size() - 1 : 0;

	curIdx = (curIdx > firstIdx) ? curIdx - 1 : lastIdx;
	jumpToLevelWave(g_levelIdx, m_selectedIdx);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPgDn()
{
	auto& formations = m_gameLogic.m_enemies.debug_getFormations();
	auto& curIdx = currentLevelWaveRef(m_selectedIdx, m_gameLogic).formationIdx;

	const size_t firstIdx = m_gameLogic.m_enemies.nullFormationIdx + 1;
	const size_t lastIdx	= (formations.size() > 0) ? formations.size() - 1 : 0;

	curIdx = (curIdx < lastIdx) ? curIdx + 1 : firstIdx;
	jumpToLevelWave(g_levelIdx, m_selectedIdx);
}

//------------------------------------------------------------------------------
size_t
LevelEditorMode::lastItemIdx() const
{
	auto& waves = currentLevelWavesRef(m_gameLogic);
	return waves.size() - 1;
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
std::wstring
FormationListMode::itemNameToDisplay(size_t itemIdx) const
{
	return (itemIdx == m_renamingIdx) ? renameText() : itemName(itemIdx);
}

//------------------------------------------------------------------------------
void
FormationListMode::setItemName(size_t itemIdx, std::wstring newName)
{
	auto& formations = m_gameLogic.m_enemies.debug_getFormations();
	ASSERT(itemIdx < formations.size());

	formations[itemIdx].id = newName;
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

	// Refresh level indices
	auto& levels = m_gameLogic.m_enemies.debug_getCurrentLevels();
	for (auto& l : levels)
	{
		for (auto& w : l.waves)
		{
			if (w.formationIdx >= itemIdx)
			{
				--w.formationIdx;
			}
		}
	}
}

//------------------------------------------------------------------------------
size_t
FormationListMode::firstMenuIdx() const
{
	return m_gameLogic.m_enemies.nullFormationIdx + 1;
}

//------------------------------------------------------------------------------
size_t
FormationListMode::lastItemIdx() const
{
	return m_gameLogic.m_enemies.debug_getFormations().size() - 1;
}

//------------------------------------------------------------------------------
void
FormationListMode::onItemSelected()
{
	const size_t& createItemIdx = m_lastIdx;
	if (m_selectedIdx != createItemIdx)
	{
		spawnFormation(m_selectedIdx);
	}
}

//------------------------------------------------------------------------------
void
FormationListMode::onItemCommand()
{
	g_formationIdx = m_selectedIdx;
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
		static_cast<int>(section.model) + 1,
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
	section.pathIdx	= m_gameLogic.m_enemies.nullPathIdx + 1;
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
FormationSectionEditorMode::onItemSelected()
{
	spawnFormation(g_formationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onPlus()
{
	auto& section = currentFormationSectionRef(m_selectedIdx, m_gameLogic);
	static const int MAX_NUM_SHIPS = 10;
	section.numShips
		= (section.numShips < MAX_NUM_SHIPS) ? section.numShips + 1 : MAX_NUM_SHIPS;

	spawnFormation(g_formationIdx);
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

	spawnFormation(g_formationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onPgUp()
{
	auto& section = currentFormationSectionRef(m_selectedIdx, m_gameLogic);
	auto& paths		= m_gameLogic.m_enemies.debug_getPaths();

	auto& curIdx					= section.pathIdx;
	const size_t firstIdx = m_gameLogic.m_enemies.nullPathIdx + 1;
	const size_t lastIdx	= (paths.size() > 0) ? paths.size() - 1 : 0;

	curIdx = (curIdx > firstIdx) ? curIdx - 1 : lastIdx;

	spawnFormation(g_formationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onPgDn()
{
	auto& section = currentFormationSectionRef(m_selectedIdx, m_gameLogic);
	auto& paths		= m_gameLogic.m_enemies.debug_getPaths();

	auto& curIdx					= section.pathIdx;
	const size_t firstIdx = m_gameLogic.m_enemies.nullPathIdx + 1;
	const size_t lastIdx	= (paths.size() > 0) ? paths.size() - 1 : 0;

	curIdx = (curIdx < lastIdx) ? curIdx + 1 : firstIdx;

	spawnFormation(g_formationIdx);
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

	spawnFormation(g_formationIdx);
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

	spawnFormation(g_formationIdx);
}

//------------------------------------------------------------------------------
size_t
FormationSectionEditorMode::lastItemIdx() const
{
	auto& formation = currentFormationRef(m_gameLogic);
	return formation.sections.size() - 1;
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
std::wstring
PathListMode::itemNameToDisplay(size_t itemIdx) const
{
	return (itemIdx == m_renamingIdx) ? renameText() : itemName(itemIdx);
}

//------------------------------------------------------------------------------
void
PathListMode::setItemName(size_t itemIdx, std::wstring newName)
{
	auto& paths = m_gameLogic.m_enemies.debug_getPaths();
	ASSERT(itemIdx < paths.size());

	paths[itemIdx].id = newName;
}

//------------------------------------------------------------------------------
void
PathListMode::onCreate()
{
	using DirectX::SimpleMath::Vector3;
	Path path{L"New", {Waypoint()}};
	m_gameLogic.m_enemies.debug_getPaths().emplace_back(path);
}

//------------------------------------------------------------------------------
void
PathListMode::onDeleteItem(size_t itemIdx)
{
	auto& paths = m_gameLogic.m_enemies.debug_getPaths();
	ASSERT(itemIdx < paths.size());

	paths.erase(paths.begin() + itemIdx);

	// Refresh formation indices
	auto& formations = m_gameLogic.m_enemies.debug_getFormations();
	for (auto& f : formations)
	{
		for (auto& s : f.sections)
		{
			if (s.pathIdx >= itemIdx)
			{
				--s.pathIdx;
			}
		}
	}
}

//------------------------------------------------------------------------------
size_t
PathListMode::firstMenuIdx() const
{
	return m_gameLogic.m_enemies.nullPathIdx + 1;
}

//------------------------------------------------------------------------------
size_t
PathListMode::lastItemIdx() const
{
	return m_gameLogic.m_enemies.debug_getPaths().size() - 1;
}

//------------------------------------------------------------------------------
void
PathListMode::onItemSelected()
{
	const size_t& createItemIdx = m_lastIdx;
	if (m_selectedIdx != createItemIdx)
	{
		spawnPath(m_selectedIdx);
	}
}

//------------------------------------------------------------------------------
void
PathListMode::onItemCommand()
{
	g_pathIdx = m_selectedIdx;

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

	void update(const DX::StepTimer& timer)
	{
		m_modes.pCurrentMode->update(timer);
	}

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

	const auto& kb = m_resources.kbTracker;
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

	const auto& kb			= m_resources.kbTracker;
	const auto& kbState = m_resources.kbTracker.lastState;
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

	m_gameLogic.m_enemies.incrementCurrentTime(timer);
	m_pImpl->update(timer);
	m_gameLogic.m_enemies.performPhysicsUpdate();
}

//------------------------------------------------------------------------------
void
EditorState::render()
{
	TRACE
	renderStarField();
	m_gameLogic.renderEntities();
	if (m_context.debugDraw)
	{
		m_gameLogic.renderEntitiesDebug();
	}

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
	m_gameLogic.m_enemies.reset();
	m_pImpl->init();
}

//------------------------------------------------------------------------------
void
EditorState::exit()
{
	TRACE
}

//------------------------------------------------------------------------------
