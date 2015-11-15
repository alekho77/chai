// ChessEngineGreedy.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "greedyengine.h"
#include "engine.h"

namespace Chai {
  namespace Chess {
    IEngine* CreateGreedyEngine()
    {
      return new GreedyEngine();
    }

    void DeleteGreedyEngine(IEngine* ptr)
    {
      delete ptr;
    }
  }
}
