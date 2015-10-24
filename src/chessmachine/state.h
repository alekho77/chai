#pragma once
#include "chessmachine.h"
#include <vector>

#define PAWN(s,p)   {s, Type::pawn,   false, p}
#define KNIGHT(s,p) {s, Type::knight, false, p}
#define BISHOP(s,p) {s, Type::bishop, false, p}
#define ROOK(s,p)   {s, Type::rook,   false, p}
#define QUEEN(s,p)  {s, Type::queen,  false, p}
#define KING(s,p)   {s, Type::king,   false, p}

#define WPAWN(p)   {Set::white, Type::pawn,   false, p}
#define WKNIGHT(p) {Set::white, Type::knight, false, p}
#define WBISHOP(p) {Set::white, Type::bishop, false, p}
#define WROOK(p)   {Set::white, Type::rook,   false, p}
#define WQUEEN(p)  {Set::white, Type::queen,  false, p}
#define WKING(p)   {Set::white, Type::king,   false, p}

#define BPAWN(s,p)   {Set::black, Type::pawn,   false, p}
#define BKNIGHT(s,p) {Set::black, Type::knight, false, p}
#define BBISHOP(s,p) {Set::black, Type::bishop, false, p}
#define BROOK(s,p)   {Set::black, Type::rook,   false, p}
#define BQUEEN(s,p)  {Set::black, Type::queen,  false, p}
#define BKING(s,p)   {Set::black, Type::king,   false, p}

namespace Chai {
  namespace Chess {

    struct PieceState
    {
      Set set;
      Type type;
      bool moved;
      Postion position;
    };

    struct ChessState
    {
      ChessState();
      std::vector<PieceState> pieces;
    };
  }
}

