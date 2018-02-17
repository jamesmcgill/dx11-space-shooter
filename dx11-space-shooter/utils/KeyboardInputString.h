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
  KeyboardInputString(AppResources& resources, size_t maxLength = 30)
      : m_resources(resources)
      , m_maxLength(maxLength)
  {
  }

  std::wstring getDisplayText() const;
  std::wstring getRawText() const;
  void setRawText(const std::wstring& text);

  void handleInput(const DX::StepTimer& timer);

private:
  AppResources& m_resources;
  size_t m_maxLength;
  std::wstring m_buffer;
};

//------------------------------------------------------------------------------
