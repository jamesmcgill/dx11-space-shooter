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

struct AppContext;
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
  MenuManager(AppContext& context);
  void loadMenus(std::vector<MenuManager::Menu> const* menus);

  void update(const DX::StepTimer& timer);
  void render(DirectX::SpriteFont* font, DirectX::SpriteBatch* spriteBatch);

  // Menu navigating
  bool isRootMenu() const;
  void focusNextButton();
  void focusPrevButton();
  Command selectCurrentButton();
  void prevMenu();

private:
  AppContext& m_context;
  size_t m_currentMenuIdx    = 0;
  size_t m_selectedButtonIdx = 0;
  std::vector<MenuManager::Menu> const* m_activeMenus;
};

//------------------------------------------------------------------------------
