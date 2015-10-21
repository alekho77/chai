// chessmachine.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "chessmachine.h"
#include "machine.h"

namespace Chai {
  namespace Chess {
    IChessMachine* CreateChessMachine()
    {
      return /*new ChessMachine()*/ nullptr;
    }

    void DeleteChessMachine(IChessMachine* ptr)
    {
      delete ptr;
    }
  }
}
