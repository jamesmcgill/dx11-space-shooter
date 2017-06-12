#include "pch.h"
#include "MenuManager.h"
#include "StepTimer.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
static const std::vector<MenuManager::Menu> s_nullMenu = {
	{
		{
			{L"Null", Command::QuitApp},
		},
	},
};

//------------------------------------------------------------------------------
const XMVECTOR HIGHLIGHT_COLOR = {1.0f, 1.0f, 0.0f};
const XMVECTOR NORMAL_COLOR		 = {1.0f, 1.0f, 1.0f};

//------------------------------------------------------------------------------
MenuManager::MenuManager()
		: m_activeMenus(&s_nullMenu)
{
}

//------------------------------------------------------------------------------
void
MenuManager::loadMenus(std::vector<MenuManager::Menu> const* menus)
{
	assert(menus);
	assert((*menus).size());

	m_activeMenus = menus;
}

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
	auto& menus = *m_activeMenus;

	assert(m_currentMenuIdx < menus.size());
	auto& currentMenu = menus[m_currentMenuIdx];

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
	float padding		 = fontHeight * 0.5f;
	Vector2 pos			 = {m_screenWidth / 2.0f, 0.0f};

	float numRowsAboveCenter
		= (currentMenu.buttons.size() % 2 == 0)
				? currentMenu.buttons.size() / 2.0f
				: ((currentMenu.buttons.size() + 1) / 2.0f) - 0.5f;
	float numPaddingRowsAbove = numRowsAboveCenter - 0.5f;

	// Screen center - text rows - padding rows
	pos.y = (m_screenHeight / 2) - (numRowsAboveCenter * fontHeight)
					- (numPaddingRowsAbove * padding);

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
	auto& menus = *m_activeMenus;

	assert(m_currentMenuIdx < menus.size());
	auto& currentMenu = menus[m_currentMenuIdx];
	++m_selectedButtonIdx;

	if (m_selectedButtonIdx >= currentMenu.buttons.size()) {
		m_selectedButtonIdx = 0;
	}
}

//------------------------------------------------------------------------------
void
MenuManager::focusPrevButton()
{
	auto& menus = *m_activeMenus;

	assert(m_currentMenuIdx < menus.size());
	auto& currentMenu = menus[m_currentMenuIdx];

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
	auto& menus = *m_activeMenus;

	assert(m_currentMenuIdx < menus.size());
	auto& currentMenu = menus[m_currentMenuIdx];

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
	auto& menus = *m_activeMenus;
	assert(m_currentMenuIdx < menus.size());
	auto& currentMenu = menus[m_currentMenuIdx];

	if (currentMenu.previousMenuIdx != -1) {
		m_selectedButtonIdx = 0;
		m_currentMenuIdx		= currentMenu.previousMenuIdx;
	}
}

//------------------------------------------------------------------------------
