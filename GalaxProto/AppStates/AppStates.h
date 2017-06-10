#pragma once

#include "AppStates/IAppState.h"

#include "AppStates/MainMenuState.h"
#include "AppStates/GamePlayState.h"

//------------------------------------------------------------------------------
class AppStates
{
public:
	std::unique_ptr<MainMenuState> menu;
	std::unique_ptr<GamePlayState> playing;

public:
	AppStates(AppContext& context, AppResources& resources, GameLogic& logic);
	IAppState* currentState();
	void changeState(IAppState* newState);

private:
	void loadAndEnterState();
	IAppState* m_currentState;
};

//------------------------------------------------------------------------------
