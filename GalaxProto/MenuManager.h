//------------------------------------------------------------------------------
// Menu Class
//
// Handles the Main Menu and Pause Menu Interfaces
//
//
// James McGill		3/6/2017
//------------------------------------------------------------------------------
#pragma once
#include "pch.h"

namespace DX
{
class StepTimer;
}

//------------------------------------------------------------------------------
enum class Command
{
	// Menu Navigation
	PlaySingle,
	PlayTwoPlayers,
	ViewScores,
	GotoMenu,

	// Application control
	QuitApp,
	ResumeGame,
	EndGame,
};

//------------------------------------------------------------------------------
class MenuManager
{
public:
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

public:
	MenuManager();
	void loadMenus(std::vector<MenuManager::Menu> const* menus);

	void update(const DX::StepTimer& timer);
	void render(DirectX::SpriteFont* font, DirectX::SpriteBatch* spriteBatch);
	void setWindowSize(int screenWidth, int screenHeight);

	// Menu navigating
	bool isRootMenu() const;
	void focusNextButton();
	void focusPrevButton();
	Command selectCurrentButton();
	void prevMenu();

private:
	int m_screenWidth					 = 0;
	int m_screenHeight				 = 0;
	size_t m_currentMenuIdx		 = 0;
	size_t m_selectedButtonIdx = 0;
	std::vector<MenuManager::Menu> const* m_activeMenus;
};

//------------------------------------------------------------------------------
