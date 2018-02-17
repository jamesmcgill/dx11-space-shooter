#pragma once

#include "Editor/IMode.h"

//------------------------------------------------------------------------------
struct FormationSectionEditorMode : public IMode
{
  FormationSectionEditorMode(
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
  void onItemSelected() override;
  void onPlus() override;
  void onSubtract() override;
  void onPgUp() override;
  void onPgDn() override;
  void onHome() override;
  void onEnd() override;

  size_t lastItemIdx() const override;
};

//------------------------------------------------------------------------------
