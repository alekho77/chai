#pragma once
#include "chessmachine.h"
#include "state.h"

namespace Chai {
  namespace Chess {
    class ChessMachine : public IChessMachine
    {
    public:
      ChessMachine();
      //~ChessMachine();
      
      void Start() override;
      bool Move(Type type, Postion from, Postion to) override;
      bool Move(const char* notation) override;

      SetPieces GetSet(Set set) const override;
      PieceMoves CheckMoves(Postion from) const override;

    private:
      std::list<ChessState> states;
    };

  }
}

