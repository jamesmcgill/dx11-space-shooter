#pragma once

#include "IAppState.h"

//------------------------------------------------------------------------------
class EditorState : public IAppState
{
public:
	struct Impl;

	EditorState(
		AppStates& states,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic);
	~EditorState() override;

	void handleInput(const DX::StepTimer& timer) override;
	void update(const DX::StepTimer& timer) override;
	void render() override;

	void load() override;
	void unload() override;
	bool isLoaded() const override;

	void enter() override;
	void exit() override;

private:
	std::unique_ptr<Impl> m_pImpl;
};

//------------------------------------------------------------------------------
