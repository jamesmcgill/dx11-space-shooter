#pragma once
#include "pch.h"
#include "AppContext.h"
#include "AppResources.h"
#include "GameLogic.h"
#include "AppStates/AppStates.h"

namespace logger
{
struct TimedRecord;
}

//------------------------------------------------------------------------------
// A basic game implementation that creates a D3D11 device and
// provides a game loop.
//------------------------------------------------------------------------------
class Game : public DX::IDeviceNotify
{
public:
  Game();
  ~Game();

  // Initialization and management
  void initialize(HWND window, int width, int height);

  // Basic game loop
  void tick();

  // IDeviceNotify
  virtual void OnDeviceLost() override;
  virtual void OnDeviceRestored() override;

  // Messages
  void onActivated();
  void onDeactivated();
  void onSuspending();
  void onResuming();
  void onWindowSizeChanged(int width, int height);

  // Properties
  void getDefaultSize(int& width, int& height) const;

private:
  void update();
  void render();
  void drawBasicProfileInfo();
  void drawProfilerList();
  void drawFlameGraph();
  void updateControlsInfo(ui::Text& uiText);
  void clear();

  HRESULT createDeviceDependentResources();
  HRESULT createWindowSizeDependentResources();

  AppContext m_context;
  AppResources m_resources;
  GameLogic m_gameLogic;
  AppStates m_appStates;

  const logger::TimedRecord* overriddenFlameHead = nullptr;
};

//------------------------------------------------------------------------------
