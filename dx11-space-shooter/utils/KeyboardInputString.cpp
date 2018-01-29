#include "pch.h"

#include "utils/KeyboardInputString.h"
#include "AppResources.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
std::wstring
KeyboardInputString::getDisplayText() const
{
	std::wstring rename = m_buffer;
	rename.push_back(L'|');		 // Mark that we are editting
	return rename;
}

//------------------------------------------------------------------------------
std::wstring
KeyboardInputString::getRawText() const
{
	return m_buffer;
}

//------------------------------------------------------------------------------
void
KeyboardInputString::setRawText(const std::wstring& text)
{
	m_buffer = text;
}

//------------------------------------------------------------------------------
void
KeyboardInputString::handleInput(const DX::StepTimer& timer)
{
	TRACE
	UNREFERENCED_PARAMETER(timer);
	const auto& kb		 = m_resources.kbTracker;
	const auto kbState = m_resources.m_keyboard->GetState();
	using DirectX::Keyboard;

	if (kb.IsKeyPressed(Keyboard::Delete) || kb.IsKeyPressed(Keyboard::Back))
	{
		if (!m_buffer.empty())
		{
			m_buffer.pop_back();
		}
	}

	if (m_buffer.size() >= m_maxLength) {
		return;
	}

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
			m_buffer.push_back(baseChar + wchar_t(k - (int)Keyboard::A));
		}
	}

	for (int k = (int)Keyboard::D0; k <= (int)Keyboard::D9; ++k)
	{
		if (kb.IsKeyPressed((Keyboard::Keys)k))
		{
			m_buffer.push_back(L'0' + wchar_t(k - (int)Keyboard::D0));
		}
	}

	for (int k = (int)Keyboard::NumPad0; k <= (int)Keyboard::NumPad9; ++k)
	{
		if (kb.IsKeyPressed((Keyboard::Keys)k))
		{
			m_buffer.push_back(L'0' + wchar_t(k - (int)Keyboard::NumPad0));
		}
	}

	if (kb.IsKeyPressed(Keyboard::OemMinus))
	{
		m_buffer.push_back(isCapitalised() ? L'_' : L'-');
	}

	if (kb.IsKeyPressed(Keyboard::Space))
	{
		m_buffer.push_back(L' ');
	}
}

//------------------------------------------------------------------------------
