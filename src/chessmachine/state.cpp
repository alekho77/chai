#include "stdafx.h"
#include "state.h"

namespace Chai {
  namespace Chess {

    CHESSBOARD;

    ChessState::ChessState()
      : whitePieces({ PAWN(a2),   PAWN(b2),   PAWN(c2),  PAWN(d2), PAWN(e2),   PAWN(f2),   PAWN(g2), PAWN(h2),
                      ROOK(a1), KNIGHT(b1), BISHOP(c1), QUEEN(d1), KING(e1), BISHOP(f1), KNIGHT(g1), ROOK(h1) })
      , blackPieces({ PAWN(a7),   PAWN(b7),   PAWN(c7),  PAWN(d7), PAWN(e7),   PAWN(f7),   PAWN(g7), PAWN(h7),
                      ROOK(a8), KNIGHT(b8), BISHOP(c8), QUEEN(d8), KING(e8), BISHOP(f8), KNIGHT(g8), ROOK(h8) })
    {
    }

  }
}