#pragma once

#include "pch.h"
#include "UIDebugDraw.h"

//------------------------------------------------------------------------------
namespace DX
{
class StepTimer;
};
struct Modes;
struct AppContext;
struct AppResources;
class GameLogic;
struct IMode;

//------------------------------------------------------------------------------
class ModeMenu
{
public:
	struct ModeButton : public ui::Button
	{
		IMode* pGotoMode = nullptr;		 // TODO(James): <NOT_NULL>
	};

	static constexpr size_t NUM_MODE_BUTTONS = 3;
	static constexpr float CAMERA_SPEED_X		 = 1.0f;
	static constexpr float CAMERA_SPEED_Y		 = 1.0f;

	static constexpr float MODE_BUTTON_WIDTH			= 150.0f;
	static constexpr float MODE_BUTTON_HEIGHT			= 40.0f;
	static constexpr float MODE_BUTTON_STEP_X			= MODE_BUTTON_WIDTH + 20.f;
	static constexpr float MODE_BUTTON_POSITION_Y = 20.0f;
	static constexpr float MODE_BUTTON_START_X
		= 20.0f + (MODE_BUTTON_WIDTH * 0.5f);

private:
	Modes& m_modes;
	AppContext& m_context;
	AppResources& m_resources;
	GameLogic& m_gameLogic;

	std::array<ModeButton, NUM_MODE_BUTTONS> m_modeButtons;

public:
	ModeMenu(
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

	void init();
	void update(const DX::StepTimer& timer);
	void handleInput(const DX::StepTimer& timer);
	void render();
};

//------------------------------------------------------------------------------
