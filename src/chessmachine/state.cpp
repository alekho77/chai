#include "stdafx.h"
#include "state.h"

namespace Chai {
  namespace Chess {

    CHESSBOARD;

    ChessState::ChessState()
      : pieces({  WPAWN(a2),   WPAWN(b2),   WPAWN(c2),  WPAWN(d2), WPAWN(e2),   WPAWN(f2),   WPAWN(g2), WPAWN(h2),
                  WROOK(a1), WKNIGHT(b1), WBISHOP(c1), WQUEEN(d1), WKING(e1), WBISHOP(f1), WKNIGHT(g1), WROOK(h1),
                  BPAWN(a7),   BPAWN(b7),   BPAWN(c7),  BPAWN(d7), BPAWN(e7),   BPAWN(f7),   BPAWN(g7), BPAWN(h7),
                  BROOK(a8), BKNIGHT(b8), BBISHOP(c8), BQUEEN(d8), BKING(e8), BBISHOP(f8), BKNIGHT(g8), BROOK(h8) })
    {
    }

  }
}