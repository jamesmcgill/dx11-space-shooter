#include "pch.h"
#include "AppStates.h"
#include "EditorState.h"
#include "AppContext.h"
#include "AppResources.h"
#include "GameLogic.h"

#include "Editor/IMode.h"
#include "Editor/IRenameableMode.h"
#include "Editor/LevelListMode.h"
#include "Editor/LevelEditorMode.h"
#include "Editor/FormationListMode.h"
#include "Editor/FormationSectionEditorMode.h"
#include "Editor/PathListMode.h"
#include "Editor/PathEditorMode.h"
#include "Editor/Modes.h"
#include "Editor/ModeMenu.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
namespace
{
constexpr float CAMERA_SPEED_X = 1.0f;
constexpr float CAMERA_SPEED_Y = 1.0f;
};    // anon namespace

//------------------------------------------------------------------------------
struct EditorState::Impl
{
  AppContext& m_context;
  AppResources& m_resources;
  GameLogic& m_gameLogic;
  Modes m_modes;
  ModeMenu m_modeMenu;

  Impl(AppContext& context, AppResources& resources, GameLogic& logic)
      : m_context(context)
      , m_resources(resources)
      , m_gameLogic(logic)
      , m_modes(context, resources, logic)
      , m_modeMenu(m_modes, context, resources, logic)
  {
  }

  void init();
  void update(const DX::StepTimer& timer);
  void handleInput(const DX::StepTimer& timer);
  void render();
  void renderUI();
  void renderStarField();
};

//------------------------------------------------------------------------------
void
EditorState::Impl::init()
{
  m_modes.initModes();
  m_modeMenu.init();
}

//------------------------------------------------------------------------------
void
EditorState::Impl::update(const DX::StepTimer& timer)
{
  TRACE
  m_resources.starField->update(timer);
  m_gameLogic.m_enemies.incrementCurrentTime(timer);

  m_modes.pCurrentMode->update(timer);

  m_gameLogic.m_enemies.performPhysicsUpdate();
}

//------------------------------------------------------------------------------
void
EditorState::Impl::handleInput(const DX::StepTimer& timer)
{
  TRACE
  m_modeMenu.handleInput(timer);
  m_modes.pCurrentMode->handleInput(timer);
}

//------------------------------------------------------------------------------
void
EditorState::Impl::render()
{
  TRACE
  renderStarField();
  m_gameLogic.renderEntityModels();

  DX::DrawContext drawContext(m_context, m_resources);
  if (m_context.debugDraw)
  {
    drawContext.begin();
    m_gameLogic.renderEntitiesDebug();
    drawContext.end();
  }

  drawContext.begin();
  m_modes.pCurrentMode->render();
  drawContext.end();

  drawContext.begin(DX::DrawContext::Projection::Screen);
  renderUI();
  drawContext.end();
}

//------------------------------------------------------------------------------
void
EditorState::Impl::renderUI()
{
  TRACE
  m_modeMenu.render();
  m_modes.pCurrentMode->renderUI();
}

//------------------------------------------------------------------------------
void
EditorState::Impl::renderStarField()
{
  TRACE
  auto& spriteBatch = m_resources.m_spriteBatch;

  spriteBatch->Begin();
  m_resources.starField->render(*spriteBatch);
  spriteBatch->End();
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
EditorState::EditorState(
  AppStates& states,
  AppContext& context,
  AppResources& resources,
  GameLogic& logic)
    : IAppState(states, context, resources, logic)
    , m_pImpl(std::make_unique<Impl>(context, resources, logic))
{
}

//------------------------------------------------------------------------------
EditorState::~EditorState() = default;

//------------------------------------------------------------------------------
void
EditorState::handleInput(const DX::StepTimer& timer)
{
  TRACE
  float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

  const auto& kb      = m_resources.kbTracker;
  const auto& kbState = m_resources.kbTracker.lastState;
  using DirectX::Keyboard;

  // Debug Controls
  if (kb.IsKeyPressed(Keyboard::F2))
  {
    m_context.debugDraw = !m_context.debugDraw;
  }

  if (kb.IsKeyPressed(Keyboard::F3))
  {
    m_states.changeState(m_states.previousState());
  }

  if (kbState.W)
  {
    m_context.cameraRotationX -= elapsedTimeS * CAMERA_SPEED_X;
    m_context.updateViewMatrix();
  }
  else if (kbState.S)
  {
    m_context.cameraRotationX += elapsedTimeS * CAMERA_SPEED_X;
    m_context.updateViewMatrix();
  }

  if (kbState.A)
  {
    m_context.cameraRotationY -= elapsedTimeS * CAMERA_SPEED_Y;
    m_context.updateViewMatrix();
  }
  else if (kbState.D)
  {
    m_context.cameraRotationY += elapsedTimeS * CAMERA_SPEED_Y;
    m_context.updateViewMatrix();
  }
  if (kbState.R)
  {
    m_context.cameraRotationX = 0.0f;
    m_context.cameraRotationY = 0.0f;
    m_context.updateViewMatrix();
  }

  m_pImpl->handleInput(timer);
}

//------------------------------------------------------------------------------
void
EditorState::update(const DX::StepTimer& timer)
{
  m_pImpl->update(timer);
}

//------------------------------------------------------------------------------
void
EditorState::render()
{
  m_pImpl->render();
}

//------------------------------------------------------------------------------
void
EditorState::load()
{
}

//------------------------------------------------------------------------------
void
EditorState::unload()
{
}

//------------------------------------------------------------------------------
bool
EditorState::isLoaded() const
{
  return false;
}

//------------------------------------------------------------------------------
void
EditorState::enter()
{
  TRACE
  m_gameLogic.m_enemies.reset();
  m_pImpl->init();
}

//------------------------------------------------------------------------------
void
EditorState::exit()
{
  TRACE
  m_gameLogic.m_enemies.save();
}

//------------------------------------------------------------------------------
