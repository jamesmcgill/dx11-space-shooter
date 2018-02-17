#pragma once
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4505)
#include "WinRTMidi.h"
#include "WindowsVersionHelper.h"
#pragma warning(pop)

#include <iostream>
#include <functional>
#include <array>
#include <bitset>

#define USING_APP_MANIFEST
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace midi
{
//------------------------------------------------------------------------------
constexpr size_t MAX_CONTROLLERS = 128;
using State                      = std::array<int, MAX_CONTROLLERS>;

//------------------------------------------------------------------------------
class MidiControllerTracker
{
public:
  State lastState;
  State currentState;
  std::bitset<MAX_CONTROLLERS> dirtyMask = 0;

public:
  MidiControllerTracker() { reset(); }

  void onEvent(int controllerId, int value)
  {
    currentState[controllerId] = value;
    dirtyMask.set(controllerId, true);
  }

  void flush()
  {
    dirtyMask.reset();
    lastState = currentState;
  }

  void reset() { memset(this, 0, sizeof(MidiControllerTracker)); }
};

//------------------------------------------------------------------------------
static CRITICAL_SECTION g_criticalSection;
static std::function<void(int controllerId, int value)> g_onControllerEvent;

//------------------------------------------------------------------------------
static void
midiPortChangedCallback(
  const WinRT::WinRTMidiPortWatcherPtr portWatcher,
  WinRT::WinRTMidiPortUpdateType update)
{
  UNREFERENCED_PARAMETER(portWatcher);
  UNREFERENCED_PARAMETER(update);
}

//------------------------------------------------------------------------------
static void
midiInCallback(
  const WinRT::WinRTMidiInPortPtr port,
  double timeStamp,
  const unsigned char* message,
  unsigned int nBytes)
{
  UNREFERENCED_PARAMETER(port);
  UNREFERENCED_PARAMETER(timeStamp);

  EnterCriticalSection(&g_criticalSection);

  if (nBytes > 2)
  {
    int status = (static_cast<int>(message[0]) & 0xF0);
    if (status == 0xB0)
    {
      int controllerId = (static_cast<int>(message[1]) & 0x7F);
      int value        = (static_cast<int>(message[2]) & 0x7F);
      if (g_onControllerEvent)
      {
        g_onControllerEvent(controllerId, value);
      }
    }
  }
  LeaveCriticalSection(&g_criticalSection);
}

//------------------------------------------------------------------------------
class MidiController
{
private:
  HINSTANCE dllHandle = NULL;

  // WinRTMidi DLL function pointers
  WinRT::WinRTWatcherPortCountFunc m_watcherPortCountFunc = nullptr;
  WinRT::WinRTWatcherPortNameFunc m_watcherPortNameFunc   = nullptr;
  WinRT::WinRTWatcherPortTypeFunc m_watcherPortTypeFunc   = nullptr;
  WinRT::WinRTMidiInitializeFunc m_midiInitFunc           = nullptr;
  WinRT::WinRTMidiFreeFunc m_midiFreeFunc                 = nullptr;
  WinRT::WinRTMidiGetPortWatcherFunc m_midiGetPortWatcher = nullptr;
  WinRT::WinRTMidiInPortOpenFunc m_midiInPortOpenFunc     = nullptr;
  WinRT::WinRTMidiInPortFreeFunc m_midiInPortFreeFunc     = nullptr;

  WinRT::WinRTMidiPtr midiPtr          = nullptr;
  WinRT::WinRTMidiInPortPtr midiInPort = nullptr;

public:
  //----------------------------------------------------------------------------
  MidiController() { InitializeCriticalSection(&g_criticalSection); }

  //----------------------------------------------------------------------------
  ~MidiController()
  {
    if (m_midiInPortFreeFunc)
    {
      m_midiInPortFreeFunc(midiInPort);
    }

    if (m_midiFreeFunc)
    {
      m_midiFreeFunc(midiPtr);
    }

    if (dllHandle)
    {
      FreeLibrary(dllHandle);
    }
    DeleteCriticalSection(&g_criticalSection);
  }

  //----------------------------------------------------------------------------
  bool loadAndInitDll()
  {
#ifdef USING_APP_MANIFEST
    if (WinRT::windows10orGreaterWithManifest())
    {
      dllHandle = LoadLibrary(L"WinRTMidi.dll");
    }
#else
    if (windows10orGreater())
    {
      dllHandle = LoadLibrary(L"WinRTMidi.dll");
    }
#endif

    if (NULL == dllHandle)
    {
      std::cout << "Unable to load WinRTMidi.dll" << std::endl;
      return false;
    }

    m_midiInitFunc = reinterpret_cast<WinRT::WinRTMidiInitializeFunc>(
      ::GetProcAddress(dllHandle, "winrt_initialize_midi"));
    if (!m_midiInitFunc)
    {
      std::cout << "Unable to acquire function winrt_initialize_midi"
                << std::endl;
      return false;
    }

    m_midiFreeFunc = reinterpret_cast<WinRT::WinRTMidiFreeFunc>(
      ::GetProcAddress(dllHandle, "winrt_free_midi"));
    if (!m_midiFreeFunc)
    {
      std::cout << "Unable to acquire function winrt_free_midi" << std::endl;
      return false;
    }

    m_midiGetPortWatcher = reinterpret_cast<WinRT::WinRTMidiGetPortWatcherFunc>(
      ::GetProcAddress(dllHandle, "winrt_get_portwatcher"));
    if (!m_midiGetPortWatcher)
    {
      std::cout << "Unable to acquire function winrt_get_portwatcher"
                << std::endl;
      return false;
    }

    m_midiInPortOpenFunc = reinterpret_cast<WinRT::WinRTMidiInPortOpenFunc>(
      ::GetProcAddress(dllHandle, "winrt_open_midi_in_port"));
    if (!m_midiInPortOpenFunc)
    {
      std::cout << "Unable to acquire function winrt_open_midi_in_port"
                << std::endl;
      return false;
    }

    m_midiInPortFreeFunc = reinterpret_cast<WinRT::WinRTMidiInPortFreeFunc>(
      ::GetProcAddress(dllHandle, "winrt_free_midi_in_port"));
    if (!m_midiInPortFreeFunc)
    {
      std::cout << "Unable to acquire function winrt_free_midi_in_port"
                << std::endl;
      return false;
    }

    m_watcherPortCountFunc = reinterpret_cast<WinRT::WinRTWatcherPortCountFunc>(
      ::GetProcAddress(dllHandle, "winrt_watcher_get_port_count"));
    if (!m_watcherPortCountFunc)
    {
      std::cout << "Unable to acquire function winrt_watcher_get_port_count"
                << std::endl;
      return false;
    }

    m_watcherPortNameFunc = reinterpret_cast<WinRT::WinRTWatcherPortNameFunc>(
      ::GetProcAddress(dllHandle, "winrt_watcher_get_port_name"));
    if (!m_watcherPortNameFunc)
    {
      std::cout << "Unable to acquire function winrt_watcher_get_port_name"
                << std::endl;
      return false;
    }

    m_watcherPortTypeFunc = reinterpret_cast<WinRT::WinRTWatcherPortTypeFunc>(
      ::GetProcAddress(dllHandle, "winrt_watcher_get_port_type"));
    if (!m_watcherPortTypeFunc)
    {
      std::cout << "Unable to acquire function winrt_watcher_get_port_type"
                << std::endl;
      return false;
    }

    WinRT::WinRTMidiErrorType result
      = m_midiInitFunc(midiPortChangedCallback, &midiPtr);
    if (result != WinRT::WINRT_NO_ERROR)
    {
      std::cout << "Unable to initialize WinRTMidi" << std::endl;
      return false;
    }

    result = m_midiInPortOpenFunc(midiPtr, 0, midiInCallback, &midiInPort);
    if (result != WinRT::WINRT_NO_ERROR)
    {
      std::cout << "Unable to create Midi In port" << std::endl;
      return false;
    }

    return true;
  }

  //----------------------------------------------------------------------------
};

};    // namespace midi
