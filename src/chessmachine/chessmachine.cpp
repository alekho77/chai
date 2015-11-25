// chessmachine.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "machine.h"

namespace Chai {
  namespace Chess {
    IMachine* CreateChessMachine()
    {
      return new ChessMachine();
    }

    void DeleteChessMachine(IMachine* ptr)
    {
      delete ptr;
    }
  }
}
