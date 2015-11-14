// ChessEngineGreedy.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "greedyengine.h"

namespace Chai {
  namespace Chess {
    IMachine* CreateGreedyEngine(const IInfoCall* cb)
    {
      return nullptr;
    }

    void DeleteGreedyEngine(IMachine* ptr)
    {
      delete ptr;
    }
  }
}
