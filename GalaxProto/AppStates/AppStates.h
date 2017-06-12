#pragma once

#include "AppStates/IAppState.h"

#include "AppStates/MainMenuState.h"
#include "AppStates/GamePlayState.h"
#include "AppStates/PauseMenuState.h"

//------------------------------------------------------------------------------
class AppStates
{
public:
	// Available States
	MainMenuState menu;
	GamePlayState playing;
	PauseMenuState paused;

public:
	AppStates(AppContext& context, AppResources& resources, GameLogic& logic);
	IAppState* currentState();
	void changeState(IAppState* newState);

private:
	void loadAndEnterState();
	IAppState* m_currentState;
};

//------------------------------------------------------------------------------
