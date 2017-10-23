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
//	DirectX::SimpleMath::Vector3 wayPoint			= {};
//	DirectX::SimpleMath::Vector3 controlPoint = {};
//};
//
// struct EnemyWaveSection
//{
//	const std::vector<Waypoint> waypoints;
//	const int numShips;
//	const ModelResource model;
//};
//
// struct EnemyWave
//{
//	const std::vector<EnemyWaveSection> sections;
//	const float instanceTimeS;
//};
//
// struct Level
//{
//	const std::vector<EnemyWave> waves;
//};

//	3 distinct editors (3 buttons at top + load/save)
// Load/Save All  (Define human editable file format)

// 1) Level Editor:
// LEVEL LIST			- Create, Delete + Select (to WaveList)
// WAVE LIST			- Create, Delete, Nav Back(to LevelList)   + Inc/Dec Time +
// Inc/Dec FormationID

// 2) Formation Editor
// FORMATION LIST - Create, Delete,  Inc/Dec ShipCount + Inc/Dec Model +
// Inc/Dec Path

// 3) Path Editor
// PATH LIST			- Select, Create, Delete Path
// PATH EDITOR		- Select/Move/Delete Points  + Add Point +  Nav Back(to
// List)

// List Controls
// Up/Down			-> Move Selection  / Inc/Decrement editable control
// Enter				-> Edit selected / 'Create' if selected
// Delete				-> Delete selected
// Left/Right		-> Cycle editables controls

//------------------------------------------------------------------------------
namespace
{
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
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct IMode
{
	AppContext& m_context;
	AppResources& m_resources;
	GameLogic& m_gameLogic;

	size_t m_menuSize		 = 0;
	size_t m_selectedIdx = 0;

	IMode(AppContext& context, AppResources& resources, GameLogic& logic)
			: m_context(context)
			, m_resources(resources)
			, m_gameLogic(logic)
	{
	}

	virtual ~IMode() = default;

	virtual void onCreate() {}
	virtual size_t numMenuItems() const = 0;
	virtual wchar_t* typeString() const = 0;

	void onUp()
	{
		const size_t lastItemIdx = (m_menuSize > 0) ? m_menuSize - 1 : 0;
		m_selectedIdx = (m_selectedIdx > 0) ? m_selectedIdx - 1 : lastItemIdx;
	}

	void onDown()
	{
		const size_t lastItemIdx = (m_menuSize > 0) ? m_menuSize - 1 : 0;
		m_selectedIdx = (m_selectedIdx < lastItemIdx) ? m_selectedIdx + 1 : 0;
	}

	void onSelect()
	{
		const size_t createItemIdx = (m_menuSize > 0) ? m_menuSize - 1 : 0;
		if (m_selectedIdx == createItemIdx)
		{
			onCreate();
			updateMenuSize();
			m_selectedIdx = (m_menuSize > 0) ? m_menuSize - 1 : 0;
		}
	}

	void update(const DX::StepTimer& timer){};
	void render();
	void updateMenuSize()
	{
		m_menuSize = numMenuItems() + 1;		// +1 for CREATE button
	}
};

//------------------------------------------------------------------------------
void
IMode::render()
{
	TRACE
	auto monoFont = m_resources.fontMono8pt.get();

	float yPos = MAIN_AREA_START_Y;
	const float yAscent
		= ceil(DirectX::XMVectorGetY(monoFont->MeasureString(L"X")));

	ui::Text uiText;
	uiText.font				= monoFont;
	uiText.position.x = MAIN_AREA_START_X;

	auto drawMenuItem =
		[&yAscent, &yPos, &uiText, &spriteBatch = m_resources.m_spriteBatch](
			bool isSelected, const wchar_t* fmt, auto&&... vars)
	{
		uiText.color			= (isSelected) ? SELECTED_ITEM_COLOR : NORMAL_ITEM_COLOR;
		uiText.text				= fmt::format(fmt, vars...);
		uiText.position.y = yPos;
		yPos += yAscent;
		uiText.draw(*spriteBatch);
	};

	std::wstring menuTitle = fmt::format(L"{} List:", typeString());
	drawMenuItem(false, menuTitle.c_str());

	std::wstring titleUnderline(menuTitle.size(), '-');
	drawMenuItem(false, titleUnderline.c_str());

	size_t idx = 0;
	while (idx < numMenuItems())
	{
		drawMenuItem((idx == m_selectedIdx), L"{}_{}", typeString(), idx);
		idx++;
	}
	drawMenuItem((idx == m_selectedIdx), L"CREATE");
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct LevelListMode : public IMode
{
	LevelListMode(AppContext& context, AppResources& resources, GameLogic& logic)
			: IMode(context, resources, logic)
	{
		updateMenuSize();
	}

	void onCreate() override
	{
		m_gameLogic.m_enemies.debug_getCurrentLevels().emplace_back();
	}

	wchar_t* typeString() const override { return L"Level"; }
	size_t numMenuItems() const override
	{
		return m_gameLogic.m_enemies.debug_getCurrentLevels().size();
	}
};

//------------------------------------------------------------------------------
struct LevelEditorMode : public IMode
{
	LevelEditorMode(
		AppContext& context, AppResources& resources, GameLogic& logic)
			: IMode(context, resources, logic)
	{
		updateMenuSize();
	}

	wchar_t* typeString() const override { return L"Waves"; }
	size_t numMenuItems() const override
	{
		auto& levels = m_gameLogic.m_enemies.debug_getCurrentLevels();
		ASSERT(g_levelIdx < levels.size());
		return levels[g_levelIdx].waves.size();
	}
};

//------------------------------------------------------------------------------
struct FormationListMode : public IMode
{
	FormationListMode(
		AppContext& context, AppResources& resources, GameLogic& logic)
			: IMode(context, resources, logic)
	{
		updateMenuSize();
	}

	wchar_t* typeString() const override { return L"Formation"; }
	size_t numMenuItems() const override { return 0; }
};

//------------------------------------------------------------------------------
struct PathListMode : public IMode
{
	PathListMode(AppContext& context, AppResources& resources, GameLogic& logic)
			: IMode(context, resources, logic)
	{
		updateMenuSize();
	}

	wchar_t* typeString() const override { return L"Path"; }
	size_t numMenuItems() const override { return 0; }
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

	LevelListMode m_levelListMode;
	LevelEditorMode m_levelEditorMode;

	FormationListMode m_formationListMode;
	PathListMode m_pathListMode;

	IMode* m_pCurrentMode = &m_levelListMode;

	static constexpr size_t NUM_BUTTONS = 3;
	std::array<ModeButton, NUM_BUTTONS> m_modeButtons;

	Impl(AppContext& context, AppResources& resources, GameLogic& logic)
			: m_context(context)
			, m_resources(resources)
			, m_gameLogic(logic)
			, m_levelListMode(m_context, m_resources, m_gameLogic)
			, m_levelEditorMode(m_context, m_resources, m_gameLogic)
			, m_formationListMode(m_context, m_resources, m_gameLogic)
			, m_pathListMode(m_context, m_resources, m_gameLogic)
	{
	}

	void setup();

	void handleInput(const DX::StepTimer& timer);
	void handleModeMenuInput(const DX::StepTimer& timer);

	void render();
	void renderModeMenu();

	void enterMode(IMode* pNewMode);
};

//------------------------------------------------------------------------------
void
EditorState::Impl::setup()
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

	m_modeButtons[0].pGotoMode	 = &m_levelListMode;
	m_modeButtons[0].uiText.text = L"Levels";

	m_modeButtons[1].pGotoMode	 = &m_formationListMode;
	m_modeButtons[1].uiText.text = L"Formations";

	m_modeButtons[2].pGotoMode	 = &m_pathListMode;
	m_modeButtons[2].uiText.text = L"Paths";
	for (auto& btn : m_modeButtons)
	{
		positionButton(btn);
	}

	enterMode(m_pCurrentMode);
	for (auto& btn : m_modeButtons)
	{
		if (btn.pGotoMode == m_pCurrentMode)
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

	auto& kb = m_resources.kbTracker;
	using DirectX::Keyboard;

	if (kb.IsKeyPressed(Keyboard::Up))
	{
		m_pCurrentMode->onUp();
	}
	else if (kb.IsKeyPressed(Keyboard::Down))
	{
		m_pCurrentMode->onDown();
	}
	if (
		kb.IsKeyPressed(Keyboard::LeftControl) || kb.IsKeyPressed(Keyboard::Space)
		|| kb.IsKeyPressed(Keyboard::Enter))
	{
		m_pCurrentMode->onSelect();
	}
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
				enterMode(button.pGotoMode);
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
	m_pCurrentMode->render();
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
void
EditorState::Impl::enterMode(IMode* pNewMode)
{
	TRACE
	ASSERT(pNewMode);
	m_pCurrentMode = pNewMode;
	g_levelIdx		 = 0;
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

	if (kb.IsKeyPressed(Keyboard::Escape))
	{
		m_states.changeState(m_states.previousState());
	}

	// Debug Controls
	if (kb.IsKeyPressed(Keyboard::F2))
	{
		m_context.debugDraw = !m_context.debugDraw;
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
	m_pImpl->setup();
}

//------------------------------------------------------------------------------
void
EditorState::exit()
{
	TRACE
}

//------------------------------------------------------------------------------
