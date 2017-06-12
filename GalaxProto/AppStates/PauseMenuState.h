#pragma once

#include "IAppState.h"

//------------------------------------------------------------------------------
class PauseMenuState : public IAppState
{
public:
	PauseMenuState(
		AppStates& states,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IAppState(states, context, resources, logic)
	{
	}

	void handleInput(const DX::StepTimer& timer) override;
	void update(const DX::StepTimer& timer) override;
	void render() override;

	void load() override;
	void unload() override;
	bool isLoaded() const override;

	void enter() override;
	void exit() override;
};

//------------------------------------------------------------------------------
