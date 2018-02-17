#pragma once

#include "Editor/IMode.h"
#include "utils/KeyboardInputString.h"

//------------------------------------------------------------------------------
namespace DX
{
class StepTimer;
};
struct Modes;
struct AppContext;
struct AppResources;
class GameLogic;

//------------------------------------------------------------------------------
class IRenameableMode : public IMode
{
public:
  IRenameableMode(
    Modes& modes,
    AppContext& context,
    AppResources& resources,
    GameLogic& logic)
      : IMode(modes, context, resources, logic)
      , m_renameText(resources)
  {
  }

  virtual void handleInput(const DX::StepTimer& timer) override;
  virtual void setItemName(size_t itemIdx, const std::wstring& newName) = 0;

protected:
  KeyboardInputString m_renameText;
  bool m_isRenaming = false;
};

//------------------------------------------------------------------------------
