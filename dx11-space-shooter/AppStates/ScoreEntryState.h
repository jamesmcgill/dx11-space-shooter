#pragma once

#include "IAppState.h"
#include "utils/KeyboardInputString.h"

//------------------------------------------------------------------------------
class ScoreEntryState : public IAppState
{
	static constexpr size_t MAX_NAME_LENGTH = 20;

public:
	ScoreEntryState(
		AppStates& states,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IAppState(states, context, resources, logic)
			, m_playerName(resources, MAX_NAME_LENGTH)
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

private:
	KeyboardInputString m_playerName;
};

//------------------------------------------------------------------------------
