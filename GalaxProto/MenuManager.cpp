#include "pch.h"
#include "MenuManager.h"
#include "StepTimer.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
struct MenuButton
{
	std::wstring text;
	Command command;
	size_t gotoMenuIdx = 0;
};

struct Menu
{
	const std::vector<MenuButton> buttons;
	const size_t previousMenuIdx = static_cast<size_t>(-1);
};

//------------------------------------------------------------------------------
static const std::vector<MenuButton> s_mainMenuButtons = {
	{L"Play Single Player", Command::PlaySingle},
	{L"Play Multi Player", Command::PlayMulti},
	{L"View Hi-Scores", Command::ViewScores},
	{L"Quit", Command::GotoMenu, 1},
};

static const std::vector<MenuButton> s_confirmQuitButtons = {
	{L"Return", Command::GotoMenu}, {L"Quit", Command::QuitApp},
};

static const std::vector<MenuButton> s_pauseMenuButtons = {
	{L"Resume", Command::ResumeGame}, {L"End Current Game", Command::EndGame},
};

static const std::vector<Menu> s_menus = {
	{s_mainMenuButtons}, {s_confirmQuitButtons, 0}, {s_pauseMenuButtons},
};

//------------------------------------------------------------------------------
const XMVECTOR HIGHLIGHT_COLOR = { 1.0f, 1.0f, 0.0f };
const XMVECTOR NORMAL_COLOR = { 1.0f, 1.0f, 1.0f };

//------------------------------------------------------------------------------
void
MenuManager::update(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);
}

//------------------------------------------------------------------------------
void
MenuManager::render(
	DirectX::SpriteFont* font, DirectX::SpriteBatch* spriteBatch)
{
	assert(m_currentMenuIdx < s_menus.size());
	auto& currentMenu = s_menus[m_currentMenuIdx];

	// Button Layout
	//
	//       ####
	// ####  ----
	// ----  ####
	// ####  ---- X
	// ----  ####
	// ####  ----
	//       ####
	float fontHeight = XMVectorGetY(font->MeasureString(L"XXX"));
	float padding		 = fontHeight;
	Vector2 pos			 = {m_screenHeight / 2.0f, 0.0f};

	if (currentMenu.buttons.size() % 2 != 0) {
		float halfRows = (currentMenu.buttons.size() - 1) / 2.0f;
		pos.y					 = (m_screenHeight / 2) - ((halfRows - 0.5f) * fontHeight)
						- (halfRows * padding);
	}
	else
	{
		float halfRows = currentMenu.buttons.size() / 2.0f;
		pos.y					 = (m_screenHeight / 2) - (halfRows * fontHeight)
						- ((halfRows - 0.5f) * padding);
	}

	// Render Menu
	for (size_t i = 0; i < currentMenu.buttons.size(); ++i)
	{
		auto& button		= currentMenu.buttons[i];
		bool isSelected = (m_selectedButtonIdx == i);

		XMVECTOR dimensions = font->MeasureString(button.text.c_str());
		Vector2 fontOrigin	= {(XMVectorGetX(dimensions) / 2.f), 0.0f};

		font->DrawString(
			spriteBatch,
			button.text.c_str(),
			pos,
			(isSelected) ? HIGHLIGHT_COLOR : NORMAL_COLOR,
			0.f,
			fontOrigin);

		pos.y += fontHeight + padding;
	}
}

//------------------------------------------------------------------------------
void
MenuManager::setWindowSize(int screenWidth, int screenHeight)
{
	m_screenWidth	= screenWidth;
	m_screenHeight = screenHeight;
}

//------------------------------------------------------------------------------
bool
MenuManager::isRootMenu() const
{
	return (m_currentMenuIdx == 0);
}

//------------------------------------------------------------------------------
void
MenuManager::focusNextButton()
{
	assert(m_currentMenuIdx < s_menus.size());
	auto& currentMenu = s_menus[m_currentMenuIdx];
	++m_selectedButtonIdx;

	if (m_selectedButtonIdx >= currentMenu.buttons.size()) {
		m_selectedButtonIdx = 0;
	}
}

//------------------------------------------------------------------------------
void
MenuManager::focusPrevButton()
{
	assert(m_currentMenuIdx < s_menus.size());
	auto& currentMenu = s_menus[m_currentMenuIdx];

	if (m_selectedButtonIdx > 0) {
		--m_selectedButtonIdx;
	}
	else
	{
		m_selectedButtonIdx = currentMenu.buttons.size() - 1;
	}
}

//------------------------------------------------------------------------------
Command
MenuManager::selectCurrentButton()
{
	assert(m_currentMenuIdx < s_menus.size());
	auto& currentMenu = s_menus[m_currentMenuIdx];

	assert(m_selectedButtonIdx < currentMenu.buttons.size());
	auto& currentButton = currentMenu.buttons[m_selectedButtonIdx];

	const Command& cmd = currentButton.command;
	switch (cmd)
	{
		case Command::GotoMenu:
			m_selectedButtonIdx = 0;
			m_currentMenuIdx		= currentButton.gotoMenuIdx;
			break;
	}

	return cmd;
}

//------------------------------------------------------------------------------
void
MenuManager::prevMenu()
{
	assert(m_currentMenuIdx < s_menus.size());
	auto& currentMenu = s_menus[m_currentMenuIdx];

	if (currentMenu.previousMenuIdx != -1) {
		m_selectedButtonIdx = 0;
		m_currentMenuIdx		= currentMenu.previousMenuIdx;
	}
}

//------------------------------------------------------------------------------
