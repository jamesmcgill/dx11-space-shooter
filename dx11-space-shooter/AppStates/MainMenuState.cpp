#include "pch.h"
#include "AppStates.h"
#include "MainMenuState.h"
#include "AppResources.h"
#include "GameLogic.h"

#include "utils/Log.h"

using namespace DirectX;

//------------------------------------------------------------------------------
extern void ExitGame();
constexpr float STATE_TIMEOUT_SECONDS = 5.0f;

//------------------------------------------------------------------------------
static const std::vector<MenuManager::MenuButton> s_mainMenuButtons = {
	{L"Play", Command::PlaySingle},
	{L"Hi-Scores", Command::ViewScores},
	{L"Quit", Command::GotoMenu, 1},
};

static const std::vector<MenuManager::MenuButton> s_confirmQuitButtons = {
	{L"Return", Command::GotoMenu},
	{L"Quit", Command::QuitApp},
};

static const std::vector<MenuManager::Menu> s_menus = {
	{s_mainMenuButtons},
	{s_confirmQuitButtons, 0},
};

//------------------------------------------------------------------------------
void
MainMenuState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	TRACE
	auto& kb		= m_resources.kbTracker;
	auto& menus = m_resources.menuManager;

	if (kb.IsKeyPressed(Keyboard::Escape))
	{
		resetTimer();
		if (!menus->isRootMenu())
		{
			menus->prevMenu();
		}
	}

	if (kb.IsKeyPressed(Keyboard::Up))
	{
		resetTimer();
		menus->focusPrevButton();
	}
	else if (kb.IsKeyPressed(Keyboard::Down))
	{
		resetTimer();
		menus->focusNextButton();
	}

	if (
		kb.IsKeyPressed(Keyboard::LeftControl) || kb.IsKeyPressed(Keyboard::Space)
		|| kb.IsKeyPressed(Keyboard::Enter))
	{
		resetTimer();
		Command cmd = menus->selectCurrentButton();
		switch (cmd)
		{
			case Command::PlaySingle:
			case Command::PlayTwoPlayers:
				m_states.changeState(&m_states.getReady);
				break;

			case Command::ViewScores:
				m_states.changeState(&m_states.showingScores);
				break;

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
	TRACE
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	m_resources.starField->update(timer);
	m_resources.menuManager->update(timer);

	m_timeoutS -= elapsedTimeS;
	if (m_timeoutS <= 0.0f)
	{
		m_states.changeState(&m_states.showingScores);
	}
}

//------------------------------------------------------------------------------
void
MainMenuState::render()
{
	TRACE
	auto& spriteBatch = m_resources.m_spriteBatch;

	spriteBatch->Begin();
	m_resources.starField->render(*spriteBatch);
	spriteBatch->End();

	spriteBatch->Begin();
	m_resources.menuManager->render(
		m_resources.font32pt.get(), spriteBatch.get());
	spriteBatch->End();
}

//------------------------------------------------------------------------------
void
MainMenuState::load()
{
	TRACE
}

//------------------------------------------------------------------------------
void
MainMenuState::unload()
{
	TRACE
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
	TRACE
	m_resources.menuManager->loadMenus(&s_menus);
	resetTimer();
}

//------------------------------------------------------------------------------
void
MainMenuState::exit()
{
	TRACE
}

//------------------------------------------------------------------------------
void
MainMenuState::resetTimer()
{
	m_timeoutS = STATE_TIMEOUT_SECONDS;
}
