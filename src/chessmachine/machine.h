#pragma once
#include "chessmachine.h"
#include "state.h"
#include <list>

namespace Chai {
  namespace Chess {
    class ChessMachine : public IChessMachine
    {
    public:
      ChessMachine();
      //~ChessMachine();
      void Start() override;
      bool GetSnapshot(Snapshot& snapshot) const override;
    private:
      std::list<ChessState> states;
    };
  }
}

