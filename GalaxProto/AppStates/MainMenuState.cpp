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
static const std::vector<MenuManager::MenuButton> s_mainMenuButtons = {
	{L"Play Single Player", Command::PlaySingle},
	{L"Play Multi Player", Command::PlayMulti},
	{L"View Hi-Scores", Command::ViewScores},
	{L"Quit", Command::GotoMenu, 1},
};

static const std::vector<MenuManager::MenuButton> s_confirmQuitButtons = {
	{L"Return", Command::GotoMenu}, {L"Quit", Command::QuitApp},
};

static const std::vector<MenuManager::Menu> s_menus = {
	{s_mainMenuButtons}, {s_confirmQuitButtons, 0},
};

//------------------------------------------------------------------------------
void
MainMenuState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	auto& kb		= m_resources.kbTracker;
	auto& menus = m_resources.menuManager;

	if (kb.IsKeyPressed(Keyboard::Escape)) {
		if (!menus->isRootMenu()) {
			menus->prevMenu();
		}
	}

	if (kb.IsKeyPressed(Keyboard::Up)) {
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
			case Command::PlaySingle:
			case Command::PlayMulti:
				m_states.changeState(&m_states.playing);
				break;

			// case Command::ViewScores:
			case Command::QuitApp:
				ExitGame();
				break;
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
	auto& spriteBatch = m_resources.m_spriteBatch;

	spriteBatch->Begin();
	m_resources.starField->render(*spriteBatch);
	spriteBatch->End();

	spriteBatch->Begin();
	m_resources.menuManager->render(m_resources.m_font.get(), spriteBatch.get());
	spriteBatch->End();
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
	m_resources.menuManager->loadMenus(&s_menus);
}

//------------------------------------------------------------------------------
void
MainMenuState::exit()
{
	// TRACE("MainMenuState::exit()");
}

//------------------------------------------------------------------------------
