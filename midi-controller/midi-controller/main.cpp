#include "MidiController.h"
#include <conio.h>
#include <iostream>

//------------------------------------------------------------------------------
int
main()
{
  midi::g_onControllerEvent = [](int controllerId, int value) {
    std::cout << "Controller " << controllerId << " = " << value << "\n";
  };

  midi::MidiController midi;
  midi.loadAndInitDll();

  std::cout << "Press any key to exit..." << std::endl;
  while (true)
  {
    if (_kbhit())
    {
      break;
    }
  }
  return 0;
}

//------------------------------------------------------------------------------
