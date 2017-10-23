#pragma once

namespace DX
{
class StepTimer;
}

struct AppContext;
struct AppResources;
class AppStates;
class GameLogic;

//------------------------------------------------------------------------------
class IAppState
{
public:
	IAppState(
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
	virtual ~IAppState() = default;

	virtual void handleInput(const DX::StepTimer& timer) = 0;
	virtual void update(const DX::StepTimer& timer)			 = 0;
	virtual void render()																 = 0;

	virtual void load()						= 0;
	virtual void unload()					= 0;
	virtual bool isLoaded() const = 0;

	// Notifications to this state, of state changes
	virtual void enter() = 0;
	virtual void exit()	= 0;

protected:
	AppStates& m_states;
	AppContext& m_context;
	AppResources& m_resources;
	GameLogic& m_gameLogic;
};

//------------------------------------------------------------------------------
