#include "pch.h"
#include "AppStates.h"

//------------------------------------------------------------------------------
AppStates::AppStates(
	AppContext& context, AppResources& resources, GameLogic& logic)
		: menu(*this, context, resources, logic)
		, playing(*this, context, resources, logic)
		, paused(*this, context, resources, logic)
		, m_currentState(&menu)
{
}

//------------------------------------------------------------------------------
IAppState*
AppStates::currentState()
{
	assert(m_currentState);
	return m_currentState;
}

//------------------------------------------------------------------------------
void
AppStates::changeState(IAppState* newState)
{
	m_currentState->exit();

	m_currentState = newState;
	loadAndEnterState();
}

//------------------------------------------------------------------------------
void
AppStates::loadAndEnterState()
{
	if (!m_currentState->isLoaded()) {
		m_currentState->load();
	}
	m_currentState->enter();
}

//------------------------------------------------------------------------------
