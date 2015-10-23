#pragma once
#include "chessmachine.h"
#include <vector>

#define PAWN(p)   {false, Type::pawn,   p}
#define KNIGHT(p) {false, Type::knight, p}
#define BISHOP(p) {false, Type::bishop, p}
#define ROOK(p)   {false, Type::rook,   p}
#define QUEEN(p)  {false, Type::queen,  p}
#define KING(p)   {false, Type::king,   p}

namespace Chai {
  namespace Chess {

    struct PieceState
    {
      bool moved;
      Type type;
      Postion position;
    };

    struct ChessState
    {
      ChessState();
      std::vector<PieceState> whitePieces;
      std::vector<PieceState> blackPieces;
    };
  }
}

