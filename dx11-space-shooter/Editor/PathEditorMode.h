#pragma once

#include "Editor/IMode.h"

//------------------------------------------------------------------------------
struct PathEditorMode : public IMode
{
  PathEditorMode(
    Modes& modes,
    AppContext& context,
    AppResources& resources,
    GameLogic& logic)
      : IMode(modes, context, resources, logic)
  {
  }

  std::wstring controlInfoText() const override;
  std::wstring menuTitle() const override;
  std::wstring itemName(size_t itemIdx) const override;

  void onBack() override;
  void onCreate() override;
  void onDeleteItem(size_t itemIdx) override;
  void onItemSelected() override {}

  size_t lastItemIdx() const override;

  void onEnterMode(bool isNavigatingForward = true) override;
  void onExitMode() override;

  void render() override;
  void handleInput(const DX::StepTimer& timer) override;

  bool isControlSelected = false;
};

//------------------------------------------------------------------------------
