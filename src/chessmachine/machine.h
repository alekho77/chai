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

      const Piece* GetSet(Set set) const override;
      const Postion* CheckMoves(Postion from) const override;
      Status CheckStatus() const override;

    private:
      std::list<ChessState> states;
      mutable std::vector<Piece> piecesSet; // temporary
      mutable std::vector<Postion> pieceMoves; // temporary
    };
  }
}

