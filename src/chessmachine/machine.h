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
      bool GetSnapshot(Snapshot& snapshot) const override;
    private:
      std::vector<Postion> pieceMoves(const Postion& pos, const ChessState& state) const;
      bool addMoveIf(std::vector<Postion>& moves, const Postion& pos, const ChessState& state) const;
      std::list<ChessState> states;
    };

    struct MoveVector
    {
      char df, dr;
    };

    inline Postion operator + (const Postion& pos, const MoveVector& vec) {
      return Postion({pos.file + vec.df, pos.rank + vec.dr });
    }
    inline Postion& operator += (Postion& pos, const MoveVector& vec) {
      pos.file += vec.df;
      pos.rank += vec.dr;
      return pos;
    }
  }
}

