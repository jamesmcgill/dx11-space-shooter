#pragma once

#include "pch.h"

//------------------------------------------------------------------------------
namespace DX
{
class StepTimer;
};
struct AppResources;

//------------------------------------------------------------------------------
class KeyboardInputString
{
public:
	KeyboardInputString(AppResources& resources)
			: m_resources(resources)
	{
	}

	std::wstring getDisplayText() const;
	std::wstring getRawText() const;
	void setRawText(const std::wstring& text);

	void handleInput(const DX::StepTimer& timer);

private:
	AppResources& m_resources;
	std::wstring m_buffer;
};

//------------------------------------------------------------------------------
