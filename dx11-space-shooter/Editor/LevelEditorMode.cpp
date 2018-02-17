#include "pch.h"
#include "Editor/LevelEditorMode.h"
#include "Editor/Modes.h"
#include "AppContext.h"
#include "GameLogic.h"
#include "Enemies.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
namespace
{
// Prevent displaying the dummy data at index[0]
constexpr size_t FORMATION_FIRST_IDX = Enemies::DUMMY_FORMATION_IDX + 1;

constexpr float MIN_SPAWN_TIME = 0.01f;    // zero is disabled

};    // anon namespace

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
std::wstring
LevelEditorMode::controlInfoText() const
{
  return L"Navigate(Up/Down), Select(Enter), Create(C), Delete(Del), "
         "Time(-/+), Formation(PgUp/PgDn), Back(Esc)";
}

//------------------------------------------------------------------------------
std::wstring
LevelEditorMode::menuTitle() const
{
  return fmt::format(
    L"Level_{}  (Spawn Time (sec) - Wave)", m_context.editorLevelIdx);
}

//------------------------------------------------------------------------------
std::wstring
LevelEditorMode::itemName(size_t itemIdx) const
{
  const auto& wave = levelWaveRef(m_context.editorLevelIdx, itemIdx);
  auto& id         = formationRef(wave.formationIdx).id;

  return fmt::format(L"{:4}   {}", wave.spawnTimeS, id);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onBack()
{
  m_modes.enterMode(&m_modes.levelListMode, false);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onCreate()
{
  if (FORMATION_FIRST_IDX >= formationsRef().size())
  {
    return;    // Do nothing. No formations available
  }

  auto& waves = levelRef(m_context.editorLevelIdx).waves;
  float t     = (waves.empty()) ? MIN_SPAWN_TIME : waves.back().spawnTimeS;

  Wave newWave{t, FORMATION_FIRST_IDX};
  waves.emplace_back(newWave);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onItemSelected()
{
  const size_t& createItemIdx = m_lastIdx;
  if (m_selectedIdx != createItemIdx)
  {
    jumpToLevelWave(m_context.editorLevelIdx, m_selectedIdx);
  }
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onDeleteItem(size_t itemIdx)
{
  auto& waves = levelRef(m_context.editorLevelIdx).waves;

  ASSERT(itemIdx < waves.size());
  waves.erase(waves.begin() + itemIdx);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPlus()
{
  auto& t = levelWaveRef(m_context.editorLevelIdx, m_selectedIdx).spawnTimeS;
  t += 1.0f;
  t = round(t);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onSubtract()
{
  auto& t = levelWaveRef(m_context.editorLevelIdx, m_selectedIdx).spawnTimeS;
  t -= 1.0f;
  if (t < MIN_SPAWN_TIME)
  {
    t = MIN_SPAWN_TIME;
  };
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPgUp()
{
  auto& formations = formationsRef();
  auto& curIdx
    = levelWaveRef(m_context.editorLevelIdx, m_selectedIdx).formationIdx;
  const size_t lastIdx = (formations.size() > 0) ? formations.size() - 1 : 0;

  curIdx = (curIdx > FORMATION_FIRST_IDX) ? curIdx - 1 : lastIdx;
  jumpToLevelWave(m_context.editorLevelIdx, m_selectedIdx);
}

//------------------------------------------------------------------------------
void
LevelEditorMode::onPgDn()
{
  auto& formations = formationsRef();
  auto& curIdx
    = levelWaveRef(m_context.editorLevelIdx, m_selectedIdx).formationIdx;
  const size_t lastIdx = (formations.size() > 0) ? formations.size() - 1 : 0;

  curIdx = (curIdx < lastIdx) ? curIdx + 1 : FORMATION_FIRST_IDX;
  jumpToLevelWave(m_context.editorLevelIdx, m_selectedIdx);
}

//------------------------------------------------------------------------------
size_t
LevelEditorMode::lastItemIdx() const
{
  auto& waves = levelRef(m_context.editorLevelIdx).waves;
  return waves.size() - 1;
}

//------------------------------------------------------------------------------
void
LevelEditorMode::update(const DX::StepTimer& timer)
{
  UNREFERENCED_PARAMETER(timer);
  m_gameLogic.m_enemies.updateLevel();
}

//------------------------------------------------------------------------------
