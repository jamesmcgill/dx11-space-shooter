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
constexpr float CAMERA_SPEED_X = 1.0f;
constexpr float CAMERA_SPEED_Y = 1.0f;

const float MODE_BUTTON_WIDTH			 = 300.0f;
const float MODE_BUTTON_HEIGHT		 = 80.0f;
const float MODE_BUTTON_POSITION_Y = 40.0f;

const float MAIN_AREA_START_X = 20.0f;
const float MAIN_AREA_START_Y
	= MODE_BUTTON_POSITION_Y + (1.5f * MODE_BUTTON_HEIGHT);

//------------------------------------------------------------------------------
class IEditorMode
{
public:
	IEditorMode(
		AppStates& states,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: m_states(states)
			, m_context(context)
			, m_resources(resources)
			, m_gameLogic(logic)
	{
	}

	virtual void handleInput(const DX::StepTimer& timer) = 0;
	virtual void update(const DX::StepTimer& timer)			 = 0;
	virtual void render()																 = 0;
	virtual void enter()																 = 0;
	virtual void exit()																	 = 0;

protected:
	AppStates& m_states;
	AppContext& m_context;
	AppResources& m_resources;
	GameLogic& m_gameLogic;
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct EditorState::Impl
{
	enum class EditorMode
	{
		LevelList,
		LevelEditor,
		FormationList,
		PathList,
		PathEditor
	};
	struct ModeButton : public ui::Button
	{
		EditorMode gotoMode;
	};

	//----------------------------------------------------------------------------
	AppContext& m_context;
	AppResources& m_resources;
	GameLogic& m_gameLogic;

	EditorMode m_currentMode								 = EditorMode::LevelList;
	size_t m_selectedIdx										 = 0;
	size_t m_numMenuItems										 = 0;
	static constexpr size_t NUM_MODE_BUTTONS = 3;
	std::array<ModeButton, NUM_MODE_BUTTONS> modeButtons;

	//----------------------------------------------------------------------------
	Impl(AppContext& context, AppResources& resources, GameLogic& logic)
			: m_context(context)
			, m_resources(resources)
			, m_gameLogic(logic)
	{
	}

	//----------------------------------------------------------------------------
	void setup();
	void handleInput(const DX::StepTimer& timer);
	void enterMode(EditorMode newMode);
	size_t numLevels() const;
	size_t numMenuItems() const;

	void render();
	void renderModeMenu();
	void renderLevelList();
};

//------------------------------------------------------------------------------
void
EditorState::Impl::setup()
{
	TRACE
	const float BUTTON_STEP_X			= m_context.screenWidth / NUM_MODE_BUTTONS;
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

	modeButtons[0].gotoMode		 = EditorMode::LevelList;
	modeButtons[0].uiText.text = L"Levels";

	modeButtons[1].gotoMode		 = EditorMode::FormationList;
	modeButtons[1].uiText.text = L"Formations";

	modeButtons[2].gotoMode		 = EditorMode::PathList;
	modeButtons[2].uiText.text = L"Paths";
	for (auto& btn : modeButtons)
	{
		positionButton(btn);
	}

	enterMode(m_currentMode);
	for (auto& btn : modeButtons)
	{
		if (btn.gotoMode == m_currentMode)
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
	UNREFERENCED_PARAMETER(timer);

	auto& kb			= m_resources.kbTracker;
	using DirectX::Keyboard;

	if (kb.IsKeyPressed(Keyboard::Up))
	{
		const size_t lastItemIdx = (m_numMenuItems > 0) ? m_numMenuItems - 1 : 0;
		m_selectedIdx = (m_selectedIdx > 0) ? m_selectedIdx - 1 : lastItemIdx;
	}
	else if (kb.IsKeyPressed(Keyboard::Down))
	{
		const size_t lastItemIdx = (m_numMenuItems > 0) ? m_numMenuItems - 1 : 0;
		m_selectedIdx = (m_selectedIdx < lastItemIdx) ? m_selectedIdx + 1 : 0;
	}
	if (
		kb.IsKeyPressed(Keyboard::LeftControl) || kb.IsKeyPressed(Keyboard::Space)
		|| kb.IsKeyPressed(Keyboard::Enter))
	{
		const size_t createItemIdx = (m_numMenuItems > 0) ? m_numMenuItems - 1 : 0;
		if (m_selectedIdx == createItemIdx)
		{
			m_gameLogic.m_enemies.debug_getCurrentLevels().emplace_back();
			m_numMenuItems = numMenuItems();
			m_selectedIdx	= (m_numMenuItems > 0) ? m_numMenuItems - 1 : 0;
		}
	}

	const auto& mouseBtns	= m_resources.mouseTracker;
	const auto& mouseState = m_resources.m_mouse->GetState();
	for (auto& button : modeButtons)
	{
		if (button.isPointInside(
					static_cast<float>(mouseState.x), static_cast<float>(mouseState.y)))
		{
			using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;
			if (mouseBtns.leftButton == ButtonState::PRESSED)
			{
				for (auto& btn : modeButtons)
				{
					btn.appearance = ui::Button::Appearance::Normal;
				}
				button.appearance = ui::Button::Appearance::Selected;
				enterMode(button.gotoMode);
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
EditorState::Impl::enterMode(EditorMode newMode)
{
	TRACE
	m_currentMode = newMode;
	m_selectedIdx = 0;

	switch (m_currentMode)
	{
		case EditorMode::FormationList:
			break;
		case EditorMode::LevelEditor:
			break;
		case EditorMode::LevelList:
			m_numMenuItems = numMenuItems();
			break;
		case EditorMode::PathEditor:
			break;
		case EditorMode::PathList:
			break;
		default:
			break;
	}
}

//------------------------------------------------------------------------------
size_t
EditorState::Impl::numLevels() const
{
	return m_gameLogic.m_enemies.debug_getCurrentLevels().size();
}

//------------------------------------------------------------------------------
size_t
EditorState::Impl::numMenuItems() const
{
	switch (m_currentMode)
	{
		case EditorMode::FormationList:
			return 0;
			break;
		case EditorMode::LevelEditor:
			return 0;
			break;
		case EditorMode::LevelList:
			return numLevels() + 1;		 // +1 for CREATE button
			break;
		case EditorMode::PathEditor:
			return 0;
			break;
		case EditorMode::PathList:
			return 0;
			break;
		default:
			return 0;
			break;
	}
}

//------------------------------------------------------------------------------
void
EditorState::Impl::render()
{
	TRACE
	renderModeMenu();
	switch (m_currentMode)
	{
		case EditorMode::FormationList:
			break;
		case EditorMode::LevelEditor:
			break;
		case EditorMode::LevelList:
			renderLevelList();
			break;
		case EditorMode::PathEditor:
			break;
		case EditorMode::PathList:
			break;
		default:
			break;
	}
}

//------------------------------------------------------------------------------
void
EditorState::Impl::renderModeMenu()
{
	TRACE
	for (auto& btn : modeButtons)
	{
		btn.draw(*m_resources.m_batch, *m_resources.m_spriteBatch);
	}
}

//------------------------------------------------------------------------------
void
EditorState::Impl::renderLevelList()
{
	TRACE
	auto monoFont = m_resources.fontMono8pt.get();

	float yPos = MAIN_AREA_START_Y;
	const float yAscent
		= ceil(DirectX::XMVectorGetY(monoFont->MeasureString(L"X")));

	ui::Text uiText;
	uiText.font				= monoFont;
	uiText.position.x = MAIN_AREA_START_X;
	uiText.color			= DirectX::Colors::MediumVioletRed;

	auto drawMenuItem =
		[&yAscent, &yPos, &uiText, &spriteBatch = m_resources.m_spriteBatch](
			bool isSelected, const wchar_t* fmt, auto&&... vars)
	{
		uiText.color
			= (isSelected) ? DirectX::Colors::Red : DirectX::Colors::MediumVioletRed;
		uiText.text				= fmt::format(fmt, vars...);
		uiText.position.y = yPos;
		yPos += yAscent;
		uiText.draw(*spriteBatch);
	};

	drawMenuItem(false, L"Level List:");
	drawMenuItem(false, L"-----------");
	size_t idx = 0;
	while (idx < numLevels())
	{
		drawMenuItem((idx == m_selectedIdx), L"Level_{}", idx);
		idx++;
	}
	drawMenuItem((idx == m_selectedIdx), L"CREATE");
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
void
EditorState::update(const DX::StepTimer& timer)
{
	TRACE
	m_resources.starField->update(timer);
	// m_gameLogic.update(timer);
}

//------------------------------------------------------------------------------
void
EditorState::render()
{
	TRACE
	renderStarField();
	// m_gameLogic.render();

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
