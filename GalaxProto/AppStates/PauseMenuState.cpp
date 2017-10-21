#include "pch.h"
#include "AppStates.h"
#include "PauseMenuState.h"
#include "AppResources.h"
#include "GameLogic.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
extern void ExitGame();

//------------------------------------------------------------------------------
static const std::vector<MenuManager::MenuButton> s_pauseMenuButtons = {
	{L"Resume", Command::ResumeGame},
	{L"End Current Game", Command::EndGame},
};

static const std::vector<MenuManager::Menu> s_menus = {
	{s_pauseMenuButtons},
};

//------------------------------------------------------------------------------
void
PauseMenuState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	TRACE
	auto& kb		= m_resources.kbTracker;
	auto& menus = m_resources.menuManager;
	using DirectX::Keyboard;

	if (kb.IsKeyPressed(Keyboard::Escape))
	{
		if (!menus->isRootMenu())
		{
			menus->prevMenu();
		}
	}

	if (kb.IsKeyPressed(Keyboard::Up))
	{
		menus->focusPrevButton();
	}
	else if (kb.IsKeyPressed(Keyboard::Down))
	{
		menus->focusNextButton();
	}

	if (
		kb.IsKeyPressed(Keyboard::LeftControl) || kb.IsKeyPressed(Keyboard::Space)
		|| kb.IsKeyPressed(Keyboard::Enter))
	{
		Command cmd = menus->selectCurrentButton();
		switch (cmd)
		{
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
	TRACE
	m_resources.menuManager->update(timer);
}

//------------------------------------------------------------------------------
void
PauseMenuState::render()
{
	TRACE
	auto& spriteBatch = m_resources.m_spriteBatch;

	spriteBatch->Begin();
	m_resources.starField->render(*spriteBatch);
	spriteBatch->End();

	m_gameLogic.render();

	m_resources.m_spriteBatch->Begin();
	m_resources.menuManager->render(
		m_resources.font32pt.get(), m_resources.m_spriteBatch.get());
	spriteBatch->End();
}

//------------------------------------------------------------------------------
void
PauseMenuState::load()
{
	TRACE
}

//------------------------------------------------------------------------------
void
PauseMenuState::unload()
{
	TRACE
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
	TRACE
	m_resources.m_timer.PauseTotalTimer(true);
	m_resources.menuManager->loadMenus(&s_menus);
}

//------------------------------------------------------------------------------
void
PauseMenuState::exit()
{
	TRACE
	m_resources.m_timer.PauseTotalTimer(false);
}

//------------------------------------------------------------------------------
