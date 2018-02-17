#include "pch.h"
#include "Editor/ModeMenu.h"
#include "Editor/IMode.h"
#include "Editor/Modes.h"

#include "AppContext.h"
#include "AppResources.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
void
ModeMenu::init()
{
  TRACE
  float xPos          = MODE_BUTTON_START_X;
  auto positionButton = [&](auto& button) {
    using DirectX::SimpleMath::Vector2;
    button.position.x = xPos - (MODE_BUTTON_WIDTH * 0.5f);
    button.position.y = MODE_BUTTON_POSITION_Y;
    button.size       = Vector2(MODE_BUTTON_WIDTH, MODE_BUTTON_HEIGHT);

    button.uiText.font  = m_resources.font16pt.get();
    button.uiText.color = DirectX::Colors::Yellow;
    button.centerText();
    xPos += MODE_BUTTON_STEP_X;
  };

  m_modeButtons[0].pGotoMode   = &m_modes.levelListMode;
  m_modeButtons[0].uiText.text = L"Levels";

  m_modeButtons[1].pGotoMode   = &m_modes.formationListMode;
  m_modeButtons[1].uiText.text = L"Formations";

  m_modeButtons[2].pGotoMode   = &m_modes.pathListMode;
  m_modeButtons[2].uiText.text = L"Paths";

  for (auto& btn : m_modeButtons)
  {
    positionButton(btn);
  }

  m_modes.enterMode(m_modes.pCurrentMode);
  for (auto& btn : m_modeButtons)
  {
    if (btn.pGotoMode == m_modes.pCurrentMode)
    {
      btn.appearance = ui::Button::Appearance::Selected;
    }
  }
}

//------------------------------------------------------------------------------
void
ModeMenu::update(const DX::StepTimer& timer)
{
  UNREFERENCED_PARAMETER(timer);
}

//------------------------------------------------------------------------------
void
ModeMenu::handleInput(const DX::StepTimer& timer)
{
  TRACE
  UNREFERENCED_PARAMETER(timer);

  const auto& mouseBtns  = m_resources.mouseTracker;
  const auto& mouseState = m_resources.m_mouse->GetState();
  for (auto& button : m_modeButtons)
  {
    if (button.isPointInside(
          static_cast<float>(mouseState.x), static_cast<float>(mouseState.y)))
    {
      using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;
      if (mouseBtns.leftButton == ButtonState::PRESSED)
      {
        for (auto& btn : m_modeButtons)
        {
          btn.appearance = ui::Button::Appearance::Normal;
        }
        button.appearance = ui::Button::Appearance::Selected;
        m_modes.enterMode(button.pGotoMode);
      }

      if (button.appearance != ui::Button::Appearance::Selected)
      {
        button.appearance = ui::Button::Appearance::HoverOver;
      }
    }
    else
    {
      if (button.appearance != ui::Button::Appearance::Selected)
      {
        button.appearance = ui::Button::Appearance::Normal;
      }
    }
  }

  const auto& kb = m_resources.kbTracker;
  using DirectX::Keyboard;
  if (kb.IsKeyPressed(Keyboard::Tab))
  {
    bool getIteratorOnNextLoop = false;
    auto nextButtonIt          = m_modeButtons.end();
    for (auto it = m_modeButtons.begin(); it != m_modeButtons.end(); ++it)
    {
      if (getIteratorOnNextLoop)
      {
        getIteratorOnNextLoop = false;
        nextButtonIt          = it;
      }

      auto& button = *it;
      if (button.appearance == ui::Button::Appearance::Selected)
      {
        button.appearance     = ui::Button::Appearance::Normal;
        getIteratorOnNextLoop = true;
      }
    }
    if (nextButtonIt == m_modeButtons.end())
    {
      nextButtonIt = m_modeButtons.begin();
    }
    auto& nextButton      = *nextButtonIt;
    nextButton.appearance = ui::Button::Appearance::Selected;
    m_modes.enterMode(nextButton.pGotoMode);
  }    // Tab Key
}

//------------------------------------------------------------------------------
void
ModeMenu::render()
{
  TRACE
  for (auto& btn : m_modeButtons)
  {
    btn.draw(*m_resources.m_batch, *m_resources.m_spriteBatch);
  }
}

//------------------------------------------------------------------------------
