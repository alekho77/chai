#pragma once
#include "chessmachine.h"
#include "state.h"

namespace Chai {
  namespace Chess {
    class ChessMachine : public IMachine
    {
    public:
      ChessMachine();
      //~ChessMachine();
      
      void Start() override;
      bool Move(Type type, Position from, Position to, Type promotion) override;
      bool Move(const char* notation) override;
      void Undo() override;

      Set CurrentPlayer() const override { return states.empty() ? Set::unknown : states.back().activeSet; }
      const Piece* GetSet(Set set) const override;
      const Position* CheckMoves(Position from) const override;
      Status CheckStatus() const override;
      const char* LastMoveNotation() const override;

      IMachine* Clone() const override;

    private:
      ChessMachine(const ChessMachine& other);
      std::list<ChessState> states;
      mutable std::vector<Piece> piecesSet; // temporary
      mutable std::vector<Position> pieceMoves; // temporary
      mutable std::string lastMove; // temporary
    };
  }
}

