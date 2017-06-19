#pragma once

#include "AppStates/IAppState.h"

#include "AppStates/MainMenuState.h"
#include "AppStates/GetReadyState.h"
#include "AppStates/GamePlayState.h"
#include "AppStates/GameOverState.h"
#include "AppStates/PauseMenuState.h"

//------------------------------------------------------------------------------
class AppStates
{
public:
	// Available States
	MainMenuState menu;
	GetReadyState getReady;
	GamePlayState playing;
	GameOverState gameOver;
	PauseMenuState paused;

public:
	AppStates(AppContext& context, AppResources& resources, GameLogic& logic);
	IAppState* currentState() const;
	IAppState* previousState() const;
	void changeState(IAppState* newState);

private:
	void loadAndEnterState();
	IAppState* m_currentState;
	IAppState* m_previousState = nullptr;
};

//------------------------------------------------------------------------------
