#include "pch.h"
#include "Editor/FormationListMode.h"
#include "Editor/Modes.h"
#include "AppContext.h"
#include "AppResources.h"
#include "Enemies.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
std::wstring
FormationListMode::controlInfoText() const
{
	return L"Navigate(Up/Down), Select(Enter), Create(C), Delete(Del),"
				 "Edit Name(E)";
}

//------------------------------------------------------------------------------
std::wstring
FormationListMode::menuTitle() const
{
	return L"Formation List";
}

//------------------------------------------------------------------------------
std::wstring
FormationListMode::itemName(size_t itemIdx) const
{
	return formationRef(itemIdx).id;
}

//------------------------------------------------------------------------------
std::wstring
FormationListMode::itemNameToDisplay(size_t itemIdx) const
{
	return (m_isRenaming && (itemIdx == m_selectedIdx))
					 ? m_renameText.getDisplayText()
					 : itemName(itemIdx);
}

//------------------------------------------------------------------------------
void
FormationListMode::setItemName(size_t itemIdx, const std::wstring& newName)
{
	formationRef(itemIdx).id = newName;
}

//------------------------------------------------------------------------------
void
FormationListMode::onCreate()
{
	formationsRef().emplace_back(Formation{L"New"});
}

//------------------------------------------------------------------------------
void
FormationListMode::onDeleteItem(size_t itemIdx)
{
	auto& formations = formationsRef();
	ASSERT(itemIdx < formations.size());

	formations.erase(formations.begin() + itemIdx);

	// Refresh level indices
	auto& levels = levelsRef();
	for (auto& l : levels)
	{
		for (auto& w : l.waves)
		{
			if (w.formationIdx >= itemIdx)
			{
				--w.formationIdx;
			}
		}
	}
}

//------------------------------------------------------------------------------
size_t
FormationListMode::firstMenuIdx() const
{
	// Prevent displaying the dummy data at index[0]
	return Enemies::DUMMY_FORMATION_IDX + 1;
}

//------------------------------------------------------------------------------
size_t
FormationListMode::lastItemIdx() const
{
	return formationsRef().size() - 1;
}

//------------------------------------------------------------------------------
void
FormationListMode::onItemSelected()
{
	const size_t& createItemIdx = m_lastIdx;
	if (m_selectedIdx != createItemIdx)
	{
		spawnFormation(m_selectedIdx);
	}
}

//------------------------------------------------------------------------------
void
FormationListMode::onItemCommand()
{
	m_context.editorFormationIdx = m_selectedIdx;
	m_modes.enterMode(&m_modes.formationSectionEditorMode);
};

//------------------------------------------------------------------------------
