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
namespace
{
// Prevent displaying the dummy data at index[0]
constexpr size_t PATH_FIRST_IDX			 = Enemies::DUMMY_PATH_IDX + 1;
constexpr size_t FORMATION_FIRST_IDX = Enemies::DUMMY_FORMATION_IDX + 1;

static const int MAX_NUM_SHIPS = 10;

constexpr float MIN_SPAWN_TIME = 0.01f;		 // zero is disabled
constexpr float CAMERA_SPEED_X = 1.0f;
constexpr float CAMERA_SPEED_Y = 1.0f;

static constexpr size_t NUM_MODE_BUTTONS = 3;
const float MODE_BUTTON_WIDTH						 = 150.0f;
const float MODE_BUTTON_HEIGHT					 = 40.0f;
const float MODE_BUTTON_POSITION_Y			 = 20.0f;

const float MODE_BUTTON_START_X = 20.0f + (MODE_BUTTON_WIDTH * 0.5f);
const float MODE_BUTTON_STEP_X	= MODE_BUTTON_WIDTH + 20.f;

const float MAIN_AREA_START_X = 20.0f;
const float MAIN_AREA_START_Y
	= (1.5f * MODE_BUTTON_POSITION_Y) + MODE_BUTTON_HEIGHT;

const float LIST_START_Y = MAIN_AREA_START_Y + 40.0f;

const DirectX::XMVECTORF32 SELECTED_ITEM_COLOR = DirectX::Colors::White;
const DirectX::XMVECTORF32 NORMAL_ITEM_COLOR = DirectX::Colors::MediumVioletRed;

static float CAMERA_DIST_MULT = 1.25f;

//------------------------------------------------------------------------------
// Current menu selections
size_t g_levelIdx			= 0;
size_t g_formationIdx = 0;
size_t g_pathIdx			= 0;
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
	virtual void onExitMode() { m_gameLogic.m_enemies.save(); };

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
	virtual void render() {}
	virtual void renderUI();

	void updateIndices()
	{
		m_firstIdx = firstMenuIdx();
		m_lastIdx	= lastItemIdx() + 1;		 // +1 for CREATE button
	}

	//----------------------------------------------------------------------------
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

	//----------------------------------------------------------------------------
	LevelPool& levelsRef() const { return m_gameLogic.m_enemies.m_levels; }

	FormationPool& formationsRef() const
	{
		return m_gameLogic.m_enemies.m_formationPool;
	}

	PathPool& pathsRef() const { return m_gameLogic.m_enemies.m_pathPool; }

	Level& levelRef(const size_t idx) const
	{
		auto& levels = levelsRef();
		ASSERT(idx < levels.size());
		return levels[idx];
	}

	Wave& levelWaveRef(size_t levelIdx, size_t waveIdx) const
	{
		auto& waves = levelRef(levelIdx).waves;
		ASSERT(waveIdx < waves.size());
		return waves[waveIdx];
	}

	Formation& formationRef(size_t idx) const
	{
		auto& formations = formationsRef();
		ASSERT(idx < formations.size());
		return formations[idx];
	}

	FormationSection&
	formationSectionRef(size_t formationIdx, size_t sectionIdx) const
	{
		auto& formation = formationRef(formationIdx);
		ASSERT(sectionIdx < formation.sections.size());
		return formation.sections[sectionIdx];
	}

	Path& pathRef(size_t idx) const
	{
		auto& paths = pathsRef();
		ASSERT(idx < paths.size());
		return paths[idx];
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
	const float height		 = DirectX::XMVectorGetY(dimensions);
	m_controlInfo.position = Vector2(MAIN_AREA_START_X, MAIN_AREA_START_Y);
	m_controlInfo.origin	 = Vector2(0.0f, 0.0f);
	m_controlInfo.color		 = DirectX::Colors::MediumVioletRed;
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

		if (kb.IsKeyPressed(Keyboard::C))
		{
			onCreate();
			updateIndices();
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
IMode::renderUI()
{
	TRACE
	m_controlInfo.draw(*m_resources.m_spriteBatch);

	auto monoFont = m_resources.fontMono8pt.get();
	const float yAscent
		= ceil(DirectX::XMVectorGetY(monoFont->MeasureString(L"X")));

	using DirectX::SimpleMath::Vector2;
	Vector2 position = {MAIN_AREA_START_X, LIST_START_Y};
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
		return L"Navigate(Up/Down), Select(Enter), Create(C), Delete(Del)";
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
		return L"Navigate(Up/Down), Select(Enter), Create(C), Delete(Del), "
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
		return L"Navigate(Up/Down), Select(Enter), Create(C), Delete(Del),"
					 "Edit Name(E)";
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
		return L"Navigate(Up/Down), Select(Enter), Create(C), Delete(Del), "
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
		return L"Navigate(Up/Down), Select(Enter), "
					 "Create(C), Delete(Del), Edit Name(E)";
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
struct PathEditorMode : public IMode
{
	PathEditorMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IMode(modes, context, resources, logic)
	{
	}

	std::wstring controlInfoText() const override
	{
		return L"Select Point(Mouse Hover), Create(C), Delete(Del), "
					 "Move Points(Mouse Drag), Back(Esc)";
	}
	std::wstring menuTitle() const override { return L"Path Editor"; }
	std::wstring itemName(size_t itemIdx) const override;

	void onBack() override;
	void onCreate() override;
	void onDeleteItem(size_t itemIdx) override;
	void onItemSelected() override {}

	size_t lastItemIdx() const override;

	void onEnterMode(bool isNavigatingForward = true) override;
	void onExitMode() override;

	void render() override;
	void handleInput(const DX::StepTimer& timer) override;

	bool isControlSelected = false;
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
	PathEditorMode pathEditorMode;

	IMode* pCurrentMode = &levelListMode;

	Modes(AppContext& context, AppResources& resources, GameLogic& logic)
			: levelListMode(*this, context, resources, logic)
			, levelEditorMode(*this, context, resources, logic)
			, formationListMode(*this, context, resources, logic)
			, formationSectionEditorMode(*this, context, resources, logic)
			, pathListMode(*this, context, resources, logic)
			, pathEditorMode(*this, context, resources, logic)
	{
	}

	void initModes()
	{
		levelListMode.init();
		levelEditorMode.init();
		formationListMode.init();
		formationSectionEditorMode.init();
		pathListMode.init();
		pathEditorMode.init();
	}

	void enterMode(IMode* pNewMode, bool isNavigatingForward = true)
	{
		TRACE
		ASSERT(pCurrentMode);
		ASSERT(pNewMode);
		pCurrentMode->onExitMode();
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
	levelsRef().emplace_back();
}

//------------------------------------------------------------------------------
void
LevelListMode::onDeleteItem(size_t itemIdx)
{
	auto& levels = levelsRef();
	ASSERT(itemIdx < levels.size());
	levels.erase(levels.begin() + itemIdx);
}

//------------------------------------------------------------------------------
size_t
LevelListMode::lastItemIdx() const
{
	return levelsRef().size() - 1;
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
	const auto& wave = levelWaveRef(g_levelIdx, itemIdx);
	auto& id				 = formationRef(wave.formationIdx).id;

	return fmt::format(L"{:4}   {}", wave.spawnTimeS, id);
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
	auto& waves = levelRef(g_levelIdx).waves;
	float t			= (waves.empty()) ? MIN_SPAWN_TIME : waves.back().spawnTimeS;

	Wave newWave{t, FORMATION_FIRST_IDX};
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
	auto& waves = levelRef(g_levelIdx).waves;

	ASSERT(itemIdx < waves.size());
	waves.erase(waves.begin() + itemIdx);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPlus()
{
	auto& t = levelWaveRef(g_levelIdx, m_selectedIdx).spawnTimeS;
	t += 1.0f;
	t = round(t);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onSubtract()
{
	auto& t = levelWaveRef(g_levelIdx, m_selectedIdx).spawnTimeS;
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
	auto& formations		 = formationsRef();
	auto& curIdx				 = levelWaveRef(g_levelIdx, m_selectedIdx).formationIdx;
	const size_t lastIdx = (formations.size() > 0) ? formations.size() - 1 : 0;

	curIdx = (curIdx > FORMATION_FIRST_IDX) ? curIdx - 1 : lastIdx;
	jumpToLevelWave(g_levelIdx, m_selectedIdx);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPgDn()
{
	auto& formations		 = formationsRef();
	auto& curIdx				 = levelWaveRef(g_levelIdx, m_selectedIdx).formationIdx;
	const size_t lastIdx = (formations.size() > 0) ? formations.size() - 1 : 0;

	curIdx = (curIdx < lastIdx) ? curIdx + 1 : FORMATION_FIRST_IDX;
	jumpToLevelWave(g_levelIdx, m_selectedIdx);
}

//------------------------------------------------------------------------------
size_t
LevelEditorMode::lastItemIdx() const
{
	auto& waves = levelRef(g_levelIdx).waves;
	return waves.size() - 1;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
std::wstring
FormationListMode::itemName(size_t itemIdx) const
{
	return formationRef(itemIdx).id;
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
	formationRef(itemIdx).id = newName;
}

//------------------------------------------------------------------------------
void
FormationListMode::onCreate()
{
	formationsRef().emplace_back(Formation{L"New"});
}

//------------------------------------------------------------------------------
void
FormationListMode::onDeleteItem(size_t itemIdx)
{
	auto& formations = formationsRef();
	ASSERT(itemIdx < formations.size());

	formations.erase(formations.begin() + itemIdx);

	// Refresh level indices
	auto& levels = levelsRef();
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
	return FORMATION_FIRST_IDX;
}

//------------------------------------------------------------------------------
size_t
FormationListMode::lastItemIdx() const
{
	return formationsRef().size() - 1;
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
	auto& section = formationSectionRef(g_formationIdx, itemIdx);
	auto& path		= pathRef(section.pathIdx);

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
	auto& formation = formationRef(g_formationIdx);

	FormationSection section;
	section.pathIdx	= PATH_FIRST_IDX;
	section.numShips = 3;
	section.model		 = ModelResource::Enemy1;
	formation.sections.emplace_back(section);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onDeleteItem(size_t itemIdx)
{
	auto& formation = formationRef(g_formationIdx);
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
	auto& section = formationSectionRef(g_formationIdx, m_selectedIdx);
	section.numShips
		= (section.numShips < MAX_NUM_SHIPS) ? section.numShips + 1 : MAX_NUM_SHIPS;

	spawnFormation(g_formationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onSubtract()
{
	auto& section = formationSectionRef(g_formationIdx, m_selectedIdx);
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
	auto& section = formationSectionRef(g_formationIdx, m_selectedIdx);
	auto& paths		= pathsRef();

	auto& curIdx				 = section.pathIdx;
	const size_t lastIdx = (paths.size() > 0) ? paths.size() - 1 : 0;

	curIdx = (curIdx > PATH_FIRST_IDX) ? curIdx - 1 : lastIdx;

	spawnFormation(g_formationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onPgDn()
{
	auto& section = formationSectionRef(g_formationIdx, m_selectedIdx);
	auto& paths		= pathsRef();

	auto& curIdx				 = section.pathIdx;
	const size_t lastIdx = (paths.size() > 0) ? paths.size() - 1 : 0;

	curIdx = (curIdx < lastIdx) ? curIdx + 1 : PATH_FIRST_IDX;

	spawnFormation(g_formationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onHome()
{
	auto& section		 = formationSectionRef(g_formationIdx, m_selectedIdx);
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
	auto& section		 = formationSectionRef(g_formationIdx, m_selectedIdx);
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
	return formationRef(g_formationIdx).sections.size() - 1;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
std::wstring
PathListMode::itemName(size_t itemIdx) const
{
	return pathRef(itemIdx).id;
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
	pathRef(itemIdx).id = newName;
}

//------------------------------------------------------------------------------
void
PathListMode::onCreate()
{
	pathsRef().emplace_back(Path{L"New", {Waypoint()}});
}

//------------------------------------------------------------------------------
void
PathListMode::onDeleteItem(size_t itemIdx)
{
	auto& paths = pathsRef();
	ASSERT(itemIdx < paths.size());

	paths.erase(paths.begin() + itemIdx);

	// Refresh formation indices
	for (auto& f : formationsRef())
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
	return PATH_FIRST_IDX;
}

//------------------------------------------------------------------------------
size_t
PathListMode::lastItemIdx() const
{
	return pathsRef().size() - 1;
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
	m_modes.enterMode(&m_modes.pathEditorMode);
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
std::wstring
PathEditorMode::itemName(size_t itemIdx) const
{
	return fmt::format(L"Path:{}-{} ", pathRef(g_pathIdx).id, itemIdx);
}

//------------------------------------------------------------------------------
void
PathEditorMode::onBack()
{
	m_modes.enterMode(&m_modes.pathListMode, false);
}

//------------------------------------------------------------------------------
void
PathEditorMode::onCreate()
{
	pathRef(g_pathIdx).waypoints.emplace_back(Waypoint());
}

//------------------------------------------------------------------------------
void
PathEditorMode::onDeleteItem(size_t itemIdx)
{
	auto& path = pathRef(g_pathIdx);
	path.waypoints.erase(path.waypoints.begin() + itemIdx);
}

//------------------------------------------------------------------------------
size_t
PathEditorMode::lastItemIdx() const
{
	return pathRef(g_pathIdx).waypoints.size() - 1;
}

//------------------------------------------------------------------------------
void
PathEditorMode::onEnterMode(bool isNavigatingForward)
{
	m_gameLogic.m_enemies.reset();
	m_context.cameraDistance = m_context.defaultCameraDistance * CAMERA_DIST_MULT;
	m_context.updateViewMatrix();
	IMode::onEnterMode(isNavigatingForward);
}

//------------------------------------------------------------------------------
void
PathEditorMode::onExitMode()
{
	m_context.cameraDistance = m_context.defaultCameraDistance;
	m_context.updateViewMatrix();
	IMode::onExitMode();
}

//------------------------------------------------------------------------------
void
PathEditorMode::render()
{
	size_t pointIdx		= (isControlSelected) ? -1 : m_selectedIdx;
	size_t controlIdx = (isControlSelected) ? m_selectedIdx : -1;

	pathRef(g_pathIdx).debugRender(
		m_resources.m_batch.get(), pointIdx, controlIdx);
	m_gameLogic.renderPlayerBoundary();
}

//------------------------------------------------------------------------------
void
PathEditorMode::handleInput(const DX::StepTimer& timer)
{
	TRACE
	UNREFERENCED_PARAMETER(timer);

	IMode::handleInput(timer);

	auto& path = pathRef(g_pathIdx);

	using DirectX::SimpleMath::Vector3;
	DirectX::SimpleMath::Matrix worldToScreen = m_context.worldToView
																							* m_context.viewToProjection
																							* m_context.projectionToPixels;

	DirectX::SimpleMath::Matrix screenToView
		= m_context.pixelsToProjection * m_context.viewToProjection.Invert();

	DirectX::SimpleMath::Matrix screenToWorld
		= m_context.pixelsToProjection * m_context.viewToProjection.Invert()
			* m_context.worldToView.Invert();

	const auto& mouseBtns	= m_resources.mouseTracker;
	const auto& mouseState = m_resources.m_mouse->GetState();

	auto isMouseOverPoint
		= [&worldToScreen](
				const DirectX::Mouse::State& mouse, const Vector3& point) -> bool {

		auto screenPos					= Vector3::Transform(point, worldToScreen);
		const float mX					= static_cast<float>(mouse.x);
		const float mY					= static_cast<float>(mouse.y);
		static const float SIZE = 10.f;

		return (mX >= screenPos.x - SIZE) && (mX <= screenPos.x + SIZE)
					 && (mY >= screenPos.y - SIZE) && (mY <= screenPos.y + SIZE);
	};

	for (size_t i = 0; i < path.waypoints.size(); ++i)
	{
		auto& waypoint = path.waypoints[i];
		if (isMouseOverPoint(mouseState, waypoint.wayPoint))
		{
			m_selectedIdx			= i;
			isControlSelected = false;
		}
		if (isMouseOverPoint(mouseState, waypoint.controlPoint))
		{
			m_selectedIdx			= i;
			isControlSelected = true;
		}
	}

	// Move points with mouse
	using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;
	if (mouseBtns.leftButton == ButtonState::HELD)
	{
		Vector3 mouseScreenPos = {
			static_cast<float>(mouseState.x), static_cast<float>(mouseState.y), 1.0f};
		auto mouseInWorldPos = Vector3::Transform(mouseScreenPos, screenToWorld);

		const Vector3& cameraPos = m_context.cameraPos();
		const Vector3 rayDir		 = mouseInWorldPos - cameraPos;

		static const DirectX::SimpleMath::Plane plane(
			Vector3(), Vector3(0.0f, 0.0f, 1.0f));
		DirectX::SimpleMath::Ray ray(cameraPos, rayDir);

		float dist;
		if (ray.Intersects(plane, dist))
		{
			auto& point = (isControlSelected)
											? path.waypoints[m_selectedIdx].controlPoint
											: path.waypoints[m_selectedIdx].wayPoint;
			point = cameraPos + (rayDir * dist);
		}
	}
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
	std::array<ModeButton, NUM_MODE_BUTTONS> m_modeButtons;

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

	void update(const DX::StepTimer& timer);

	void handleInput(const DX::StepTimer& timer);
	void handleModeMenuInput(const DX::StepTimer& timer);

	void render();
	void renderUI();
	void renderModeMenuUI();

	void renderStarField();
};

//------------------------------------------------------------------------------
void
EditorState::Impl::setupModeMenu()
{
	TRACE
	float xPos					= MODE_BUTTON_START_X;
	auto positionButton = [&](auto& button) {
		using DirectX::SimpleMath::Vector2;
		button.position.x = xPos - (MODE_BUTTON_WIDTH * 0.5f);
		button.position.y = MODE_BUTTON_POSITION_Y;
		button.size				= Vector2(MODE_BUTTON_WIDTH, MODE_BUTTON_HEIGHT);

		button.uiText.font	= m_resources.font16pt.get();
		button.uiText.color = DirectX::Colors::Yellow;
		button.centerText();
		xPos += MODE_BUTTON_STEP_X;
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
EditorState::Impl::update(const DX::StepTimer& timer)
{
	TRACE
	m_resources.starField->update(timer);
	m_gameLogic.m_enemies.incrementCurrentTime(timer);

	m_modes.pCurrentMode->update(timer);

	m_gameLogic.m_enemies.performPhysicsUpdate();
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
	renderStarField();
	m_gameLogic.renderEntities();

	DX::DrawContext drawContext(m_context, m_resources);
	if (m_context.debugDraw)
	{
		drawContext.begin();
		m_gameLogic.renderEntitiesDebug();
		drawContext.end();
	}

	drawContext.begin();
	m_modes.pCurrentMode->render();
	drawContext.end();

	drawContext.begin(DX::DrawContext::Projection::Screen);
	renderUI();
	drawContext.end();
}

//------------------------------------------------------------------------------
void
EditorState::Impl::renderUI()
{
	TRACE
	renderModeMenuUI();
	m_modes.pCurrentMode->renderUI();
}

//------------------------------------------------------------------------------
void
EditorState::Impl::renderModeMenuUI()
{
	TRACE
	for (auto& btn : m_modeButtons)
	{
		btn.draw(*m_resources.m_batch, *m_resources.m_spriteBatch);
	}
}

//------------------------------------------------------------------------------
void
EditorState::Impl::renderStarField()
{
	TRACE
	auto& spriteBatch = m_resources.m_spriteBatch;

	spriteBatch->Begin();
	m_resources.starField->render(*spriteBatch);
	spriteBatch->End();
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
	m_pImpl->update(timer);
}

//------------------------------------------------------------------------------
void
EditorState::render()
{
	m_pImpl->render();
}

//------------------------------------------------------------------------------
void
EditorState::load()
{
}

//------------------------------------------------------------------------------
void
EditorState::unload()
{
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
	m_gameLogic.m_enemies.save();
}

//------------------------------------------------------------------------------
