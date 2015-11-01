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
      , activeSet(Set::white)
    {
      evalMoves();
    }

    ChessState::ChessState(const ChessState& state, const Move& move)
      : pieces(state.pieces)
      , lastMove(move)
      , activeSet(state.activeSet == Set::white ? Set::black : Set ::white)
    {
      assert(pieces.at(move.from).set == state.activeSet);
      assert(pieces.at(move.from).type == move.type);
      assert(pieces.at(move.from).moves.find(move.to) != pieces.at(move.from).moves.end());
      
      pieces[move.to] = { state.activeSet, move.type, true, {} };
      pieces.erase(move.from);
      
      evalMoves();
    }

    void ChessState::evalMoves()
    {
      std::set<Postion> opponent;
      for (auto& p : pieces) {
        if (p.second.set != activeSet) {
          pieceMoves(p.second, p.first, std::set<Postion>());
          opponent.insert(p.second.moves.begin(), p.second.moves.end());
        }
      }
      for (auto& p : pieces) {
        if (p.second.set == activeSet) {
          pieceMoves(p.second, p.first, opponent);
        }
      }
    }

    void ChessState::pieceMoves(PieceState& piece, const Postion& pos, const std::set<Postion>& opponent)
    {
      static const std::vector<MoveVector> Lshape_moves = { {-1,+2}, {+1,+2}, {-1,-2}, {+1,-2}, {+2,+1}, {+2,-1}, {-2,+1}, {-2,-1} };
      static const std::vector<MoveVector> diagonal_moves = { { +1,+1 },{ +1,-1 },{ -1,+1 },{ -1,-1 } };
      static const std::vector<MoveVector> straight_moves = { { 0,+1 },{ 0,-1 },{ +1,0 },{ -1,0 } };
      auto merge = [](const std::vector<MoveVector>& a, const std::vector<MoveVector>& b) { std::vector<MoveVector> t(a); t.insert(t.end(), b.begin(), b.end()); return t; };
      static const std::vector<MoveVector> any_moves = merge(straight_moves, diagonal_moves);

      piece.moves.clear();
      switch (piece.type) {
       case Type::pawn:
        if (piece.set == Set::white) {
          if (addMoveIf(piece.moves, { pos.file, pos.rank + 1 }) && !piece.moved && pos.rank == '2') {
            addMoveIf(piece.moves, { pos.file, pos.rank + 2 });
          }
          addMoveIf(piece.moves, { pos.file - 1, pos.rank + 1 }, piece.set, true);
          addMoveIf(piece.moves, { pos.file + 1, pos.rank + 1 }, piece.set, true);
          // TODO: added move 'En passant'
        } else {
          if (addMoveIf(piece.moves, { pos.file, pos.rank - 1 }) && !piece.moved && pos.rank == '7') {
            addMoveIf(piece.moves, { pos.file, pos.rank - 2 });
          }
          addMoveIf(piece.moves, { pos.file - 1, pos.rank - 1 }, piece.set, true);
          addMoveIf(piece.moves, { pos.file + 1, pos.rank - 1 }, piece.set, true);
          // TODO: added move 'En passant'
        }
        break;
       case Type::knight:
        for (MoveVector v : Lshape_moves) {
          addMoveIf(piece.moves, pos + v, piece.set);
        }
        break;
       case Type::bishop:
        for (MoveVector v : diagonal_moves) {
          for (Postion p = pos + v; addMoveIf(piece.moves, p, piece.set); p += v);
        }
        break;
       case Type::rook:
        for (MoveVector v : straight_moves) {
          for (Postion p = pos + v; addMoveIf(piece.moves, p, piece.set); p += v);
        }
        break;
       case Type::queen:
        for (MoveVector v : any_moves) {
          for (Postion p = pos + v; addMoveIf(piece.moves, p, piece.set); p += v);
        }
        break;
       case Type::king:
        for (MoveVector v : any_moves) {
          addMoveIf(piece.moves, pos + v, piece.set);
        }
        // TODO: added move 'Castling'
        break;
      }
    }

    bool ChessState::addMoveIf(std::set<Postion>& moves, const Postion& pos, Set set, bool capture) const
    {
      if (pos.isValid()) {
        if (pieces.find(pos) == pieces.end()) {
          if (!capture) {
            moves.insert(pos);
            return true;
          }
        } else if (set != Set::unknown && set != pieces.at(pos).set) {
          moves.insert(pos); // capture
        }
      }
      return false;
    }

  }
}