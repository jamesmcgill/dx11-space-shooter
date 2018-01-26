#include "pch.h"

#include "Editor/IRenameableMode.h"
#include "AppResources.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
void
IRenameableMode::onEnterMode(bool isNavigatingForward)
{
	m_renamingIdx = RENAME_DISABLED;
	IMode::onEnterMode(isNavigatingForward);
}

//------------------------------------------------------------------------------
std::wstring
IRenameableMode::renameText() const
{
	std::wstring rename = m_renameBuffer;
	rename.push_back(L'|');		 // Mark that we are editting
	return rename;
}

//------------------------------------------------------------------------------
void
IRenameableMode::handleInput(const DX::StepTimer& timer)
{
	TRACE
	const auto& kb = m_resources.kbTracker;
	using DirectX::Keyboard;

	// Steal all input if in renaming mode
	if (m_renamingIdx != RENAME_DISABLED)
	{
		handleInput_renaming(timer);
		return;		 // Don't process any other input
	}

	if (kb.IsKeyPressed(Keyboard::E))
	{
		const size_t& createItemIdx = m_lastIdx;
		if (m_selectedIdx != createItemIdx)
		{
			m_renamingIdx	= m_selectedIdx;
			m_renameBuffer = itemName(m_selectedIdx);
		}
	}

	IMode::handleInput(timer);
}

//------------------------------------------------------------------------------
void
IRenameableMode::handleInput_renaming(const DX::StepTimer& timer)
{
	TRACE
	UNREFERENCED_PARAMETER(timer);
	const auto& kb		 = m_resources.kbTracker;
	const auto kbState = m_resources.m_keyboard->GetState();
	using DirectX::Keyboard;

	auto isCapitalised = [&kbState]() -> bool {
		return kbState.IsKeyDown(Keyboard::CapsLock)
					 || kbState.IsKeyDown(Keyboard::LeftShift)
					 || kbState.IsKeyDown(Keyboard::RightShift);
	};

	for (int k = (int)Keyboard::A; k <= (int)Keyboard::Z; ++k)
	{
		if (kb.IsKeyPressed((Keyboard::Keys)k))
		{
			bool isCapital	 = isCapitalised();
			wchar_t baseChar = (isCapital) ? L'A' : L'a';
			m_renameBuffer.push_back(baseChar + wchar_t(k - (int)Keyboard::A));
		}
	}

	for (int k = (int)Keyboard::D0; k <= (int)Keyboard::D9; ++k)
	{
		if (kb.IsKeyPressed((Keyboard::Keys)k))
		{
			m_renameBuffer.push_back(L'0' + wchar_t(k - (int)Keyboard::D0));
		}
	}

	for (int k = (int)Keyboard::NumPad0; k <= (int)Keyboard::NumPad9; ++k)
	{
		if (kb.IsKeyPressed((Keyboard::Keys)k))
		{
			m_renameBuffer.push_back(L'0' + wchar_t(k - (int)Keyboard::NumPad0));
		}
	}

	if (kb.IsKeyPressed(Keyboard::OemMinus))
	{
		m_renameBuffer.push_back(isCapitalised() ? L'_' : L'-');
	}

	if (kb.IsKeyPressed(Keyboard::Escape))
	{
		m_renamingIdx = RENAME_DISABLED;
	}

	if (kb.IsKeyPressed(Keyboard::Delete) || kb.IsKeyPressed(Keyboard::Back))
	{
		if (!m_renameBuffer.empty())
		{
			m_renameBuffer.pop_back();
		}
	}

	if (kb.IsKeyPressed(Keyboard::Enter))
	{
		setItemName(m_renamingIdx, m_renameBuffer);
		m_renamingIdx = RENAME_DISABLED;
	}
}

//------------------------------------------------------------------------------
