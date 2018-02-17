#include "pch.h"
#include "Editor/PathListMode.h"
#include "Editor/Modes.h"
#include "AppContext.h"
#include "AppResources.h"
#include "Enemies.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
std::wstring
PathListMode::controlInfoText() const
{
  return L"Navigate(Up/Down), Select(Enter), "
         "Create(C), Delete(Del), Edit Name(E)";
}

//------------------------------------------------------------------------------
std::wstring
PathListMode::menuTitle() const
{
  return L"Path List";
}

//------------------------------------------------------------------------------
std::wstring
PathListMode::itemName(size_t itemIdx) const
{
  return pathRef(itemIdx).id;
}

//------------------------------------------------------------------------------
std::wstring
PathListMode::itemNameToDisplay(size_t itemIdx) const
{
  return (m_isRenaming && (itemIdx == m_selectedIdx))
           ? m_renameText.getDisplayText()
           : itemName(itemIdx);
}

//------------------------------------------------------------------------------
void
PathListMode::setItemName(size_t itemIdx, const std::wstring& newName)
{
  pathRef(itemIdx).id = newName;
}

//------------------------------------------------------------------------------
void
PathListMode::onCreate()
{
  pathsRef().emplace_back(Path{L"New", {Waypoint()}});
}

//------------------------------------------------------------------------------
void
PathListMode::onDeleteItem(size_t itemIdx)
{
  auto& paths = pathsRef();
  ASSERT(itemIdx < paths.size());

  paths.erase(paths.begin() + itemIdx);

  // Refresh formation indices
  for (auto& f : formationsRef())
  {
    for (auto& s : f.sections)
    {
      if (s.pathIdx >= itemIdx)
      {
        --s.pathIdx;
      }
    }
  }
}

//------------------------------------------------------------------------------
size_t
PathListMode::firstMenuIdx() const
{
  // Prevent displaying the dummy data at index[0]
  return Enemies::DUMMY_PATH_IDX + 1;
}

//------------------------------------------------------------------------------
size_t
PathListMode::lastItemIdx() const
{
  return pathsRef().size() - 1;
}

//------------------------------------------------------------------------------
void
PathListMode::onItemSelected()
{
  const size_t& createItemIdx = m_lastIdx;
  if (m_selectedIdx != createItemIdx)
  {
    spawnPath(m_selectedIdx);
  }
}

//------------------------------------------------------------------------------
void
PathListMode::onItemCommand()
{
  m_context.editorPathIdx = m_selectedIdx;
  m_modes.enterMode(&m_modes.pathEditorMode);
};

//------------------------------------------------------------------------------
