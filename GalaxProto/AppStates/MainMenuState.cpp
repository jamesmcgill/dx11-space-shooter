//#define ENABLE_TRACE
//#define ENABLE_LOCAL

#include "pch.h"
#include "AppStates.h"
#include "MainMenuState.h"
#include "AppResources.h"
#include "GameLogic.h"

using namespace DirectX;

//------------------------------------------------------------------------------
extern void ExitGame();

//------------------------------------------------------------------------------
void
MainMenuState::handleInput(const DX::StepTimer& timer)
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
			case Command::PlaySingle:
			case Command::PlayMulti:
				m_states.changeState(m_states.playing.get());
				break;

			//case Command::ViewScores:
			//case Command::GotoMenu:
			case Command::QuitApp:
				ExitGame();
				break;
			//case Command::ResumeGame:
			//case Command::EndGame:
		}
	}
}

//------------------------------------------------------------------------------
void
MainMenuState::update(const DX::StepTimer& timer)
{
	m_resources.starField->update(timer);
	m_resources.menuManager->update(timer);
}

//------------------------------------------------------------------------------
void
MainMenuState::render()
{
	m_resources.m_spriteBatch->Begin();
	m_resources.starField->render(*m_resources.m_spriteBatch);
	m_resources.m_spriteBatch->End();

	m_resources.m_spriteBatch->Begin();
		m_resources.menuManager->render(
		m_resources.m_font.get(), m_resources.m_spriteBatch.get());
	m_resources.m_spriteBatch->End();
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
