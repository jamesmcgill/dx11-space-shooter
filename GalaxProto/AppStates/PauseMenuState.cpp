//#define ENABLE_TRACE
//#define ENABLE_LOCAL

#include "pch.h"
#include "AppStates.h"
#include "PauseMenuState.h"
#include "AppResources.h"
#include "GameLogic.h"

using namespace DirectX;

//------------------------------------------------------------------------------
extern void ExitGame();

//------------------------------------------------------------------------------
void
PauseMenuState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	auto& kb = m_resources.kbTracker;
	auto& menus = m_resources.menuManager;

	if (kb.IsKeyPressed(Keyboard::Escape)) {
		if (!menus->isRootMenu()) {
			menus->prevMenu();
		}
	}

	if (kb.IsKeyPressed(Keyboard::Up)) {
		menus->focusPrevButton();
	} else if (kb.IsKeyPressed(Keyboard::Down)) {
		menus->focusNextButton();
	}

	if (
		kb.IsKeyPressed(Keyboard::LeftControl)
		|| kb.IsKeyPressed(Keyboard::Space)
		|| kb.IsKeyPressed(Keyboard::Enter))
	{
		Command cmd = menus->selectCurrentButton();
		switch (cmd) {
			case Command::ResumeGame:
				m_states.changeState(&m_states.playing);
				break;
			case Command::EndGame:
				m_states.changeState(&m_states.menu);
				break;
		}
	}
}

//------------------------------------------------------------------------------
void
PauseMenuState::update(const DX::StepTimer& timer)
{
	m_resources.menuManager->update(timer);
}

//------------------------------------------------------------------------------
void
PauseMenuState::render()
{
	m_resources.m_spriteBatch->Begin();
	m_resources.starField->render(*m_resources.m_spriteBatch);
	m_resources.m_spriteBatch->End();

	m_gameLogic.render();

	m_resources.m_spriteBatch->Begin();
	m_resources.menuManager->render(
		m_resources.m_font.get(), m_resources.m_spriteBatch.get());
	m_resources.m_spriteBatch->End();
}

//------------------------------------------------------------------------------
void
PauseMenuState::load()
{
	// TRACE("PauseMenuState::load()");
}

//------------------------------------------------------------------------------
void
PauseMenuState::unload()
{
	// TRACE("PauseMenuState::unload()");
}

//------------------------------------------------------------------------------
bool
PauseMenuState::isLoaded() const
{
	return false;
}

//------------------------------------------------------------------------------
void
PauseMenuState::enter()
{
	// TRACE("PauseMenuState::enter()");
}

//------------------------------------------------------------------------------
void
PauseMenuState::exit()
{
	// TRACE("PauseMenuState::exit()");
}

//------------------------------------------------------------------------------
