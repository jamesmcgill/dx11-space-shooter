#include "pch.h"
#include "Editor/LevelListMode.h"
#include "Editor/Modes.h"
#include "GameLogic.h"
#include "AppContext.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
std::wstring
LevelListMode::controlInfoText() const
{
	return L"Navigate(Up/Down), Select(Enter), Create(C), Delete(Del)";
}

//------------------------------------------------------------------------------
std::wstring
LevelListMode::menuTitle() const
{
	return L"Level List";
}

//------------------------------------------------------------------------------
std::wstring
LevelListMode::itemName(size_t itemIdx) const
{
	return fmt::format(L"Level_{}", itemIdx);
}

//------------------------------------------------------------------------------
void
LevelListMode::onCreate()
{
	levelsRef().emplace_back();
}

//------------------------------------------------------------------------------
void
LevelListMode::onDeleteItem(size_t itemIdx)
{
	auto& levels = levelsRef();
	ASSERT(itemIdx < levels.size());
	levels.erase(levels.begin() + itemIdx);
}

//------------------------------------------------------------------------------
size_t
LevelListMode::firstMenuIdx() const
{
	// Prevent displaying the dummy data at index[0]
	return Enemies::DUMMY_LEVEL_IDX + 1;
}

//------------------------------------------------------------------------------
size_t
LevelListMode::lastItemIdx() const
{
	return levelsRef().size() - 1;
}

//------------------------------------------------------------------------------
void
LevelListMode::onItemSelected()
{
	const size_t& createItemIdx = m_lastIdx;
	if (m_selectedIdx != createItemIdx)
	{
		jumpToLevelWave(m_selectedIdx, 0);
	}
}

//------------------------------------------------------------------------------
void
LevelListMode::onItemCommand()
{
	m_context.editorLevelIdx = m_selectedIdx;
	m_modes.enterMode(&m_modes.levelEditorMode);
};

//------------------------------------------------------------------------------
void
LevelListMode::update(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);
	m_gameLogic.m_enemies.updateLevel();
}

//------------------------------------------------------------------------------
