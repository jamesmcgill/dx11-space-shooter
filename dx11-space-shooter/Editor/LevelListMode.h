#pragma once

#include "Editor/IMode.h"

//------------------------------------------------------------------------------
struct LevelListMode : public IMode
{
  LevelListMode(
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

  void onCreate() override;
  void onItemSelected() override;
  void onDeleteItem(size_t itemIdx) override;
  void onItemCommand() override;
  size_t firstMenuIdx() const override;
  size_t lastItemIdx() const override;

  void update(const DX::StepTimer& timer) override;
};

//------------------------------------------------------------------------------
