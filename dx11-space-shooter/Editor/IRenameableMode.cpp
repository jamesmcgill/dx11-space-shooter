#include "pch.h"

#include "Editor/IRenameableMode.h"
#include "AppResources.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
void
IRenameableMode::handleInput(const DX::StepTimer& timer)
{
	const auto& kb = m_resources.kbTracker;
	using DirectX::Keyboard;

	if (m_isRenaming)
	{
		if (kb.IsKeyPressed(Keyboard::Escape))
		{
			m_isRenaming = false;
		}

		if (kb.IsKeyPressed(Keyboard::Enter))
		{
			setItemName(m_selectedIdx, m_renameText.getRawText());
			m_isRenaming = false;
		}

		m_renameText.handleInput(timer);
		return;		 // Steal all input if in renaming mode
	}

	IMode::handleInput(timer);

	if (kb.IsKeyPressed(Keyboard::E))
	{
		const size_t& createItemIdx = m_lastIdx;
		if (m_selectedIdx != createItemIdx)
		{
			m_isRenaming = true;
			m_renameText.setRawText(itemName(m_selectedIdx));
		}
	}
}

//------------------------------------------------------------------------------
