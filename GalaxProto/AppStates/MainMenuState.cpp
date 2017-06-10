//#define ENABLE_TRACE
//#define ENABLE_LOCAL

#include "pch.h"
#include "MainMenuState.h"

//------------------------------------------------------------------------------
void
MainMenuState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);
}

//------------------------------------------------------------------------------
void
MainMenuState::tick(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	static int tickCount = 0;
	++tickCount;

	if (tickCount > 360) {
		tickCount = 0;
		// Exit game
	}
}

//------------------------------------------------------------------------------
void
MainMenuState::load()
{
	// TRACE("MainMenuState::load()");
}

//------------------------------------------------------------------------------
void
MainMenuState::unload()
{
	// TRACE("MainMenuState::unload()");
}

//------------------------------------------------------------------------------
bool
MainMenuState::isLoaded() const
{
	return false;
}

//------------------------------------------------------------------------------
void
MainMenuState::enter()
{
	// TRACE("MainMenuState::enter()");
}

//------------------------------------------------------------------------------
void
MainMenuState::exit()
{
	// TRACE("MainMenuState::exit()");
}

//------------------------------------------------------------------------------
