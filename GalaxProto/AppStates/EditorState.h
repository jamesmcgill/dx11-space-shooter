#pragma once

#include "IAppState.h"

//------------------------------------------------------------------------------
class EditorState : public IAppState
{
public:
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
	void renderStarField();

private:
	struct Impl;
	std::unique_ptr<Impl> m_pImpl;
};

//------------------------------------------------------------------------------
