#pragma once
#include "chessmachine.h"
#include <vector>

namespace Chai {
  namespace Chess {
    inline Postion h2p(int hex) { return Postion({ ((hex >> 4) & 7) + 'a', (hex & 7) + '1' }); }
    
    struct PieceState
    {
      PieceState(Type t, Postion p) : moved(false), type(t), position(p) {}
      PieceState(Type t, int p) : moved(false), type(t), position(h2p(p)) {}
      bool moved;
      Type type;
      Postion position;
    };

    class ChessState
    {
    public:
      ChessState();
      //~ChessState();
    private:
      std::vector<PieceState> whitePieces;
      std::vector<PieceState> blackPieces;
    };
  }
}

