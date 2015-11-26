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
      evalMoves({});
    }

    ChessState::ChessState(const ChessState& state, const Move& move)
      : pieces(state.pieces)
      , lastMove(move)
      , activeSet(state.activeSet == Set::white ? Set::black : Set ::white)
    {
      assert(pieces.at(move.from).set == state.activeSet);
      assert(pieces.at(move.from).type == move.type);
      assert(std::binary_search(pieces.at(move.from).moves.begin(), pieces.at(move.from).moves.end(), move.to));
      
      pieces.erase(move.from);
      pieces[move.to] = PieceState({ state.activeSet, move.promotion == Type::bad ? move.type : move.promotion, true, PieceMoves() });
      
      if (move.type == Type::king) {
        const char kingrank = state.activeSet == Set::white ? '1' : '8';
        const Position kingpos1 = { 'e', kingrank };
        if (move.from == kingpos1) {
          const Position kingpos2 = { 'g', kingrank };
          const Position kingpos3 = { 'c', kingrank };
          if (move.to == kingpos2) {
            Position rookpos1 = { 'h', kingrank };
            Position rookpos2 = { 'f', kingrank };
            assert(pieces.at(rookpos1).set == state.activeSet);
            assert(pieces.at(rookpos1).type == Type::rook);
            pieces.erase(rookpos1);
            pieces[rookpos2] = PieceState({ state.activeSet, Type::rook, true, PieceMoves() });
          }
          else if (move.to == kingpos3) {
            Position rookpos1 = { 'a', kingrank };
            Position rookpos2 = { 'd', kingrank };
            assert(pieces.at(rookpos1).set == state.activeSet);
            assert(pieces.at(rookpos1).type == Type::rook);
            pieces.erase(rookpos1);
            pieces[rookpos2] = PieceState({ state.activeSet, Type::rook, true, PieceMoves() });
          }
        }
      } else if (move.type == Type::pawn) {
        if (abs(move.from.file - move.to.file) == 1 && state.pieces.find(move.to) == state.pieces.end()) {
          pieces.erase({ move.to.file, move.from.rank }); // En passant
        }
      }
      
      evalMoves(lastMove);
    }

    void ChessState::evalMoves(boost::optional<Move> xmove)
    {
      SetMoves xmoves;
      for (auto& piece : pieces) {
        if (piece.second.set != activeSet) {
          piece.second.moves = pieceMoves(pieces, piece.first, {});
          xmoves.insert(piece.second.moves.begin(), piece.second.moves.end());
        }
      }
      for (auto& piece : pieces) {
        if (piece.second.set == activeSet) {
          PieceMoves probmoves = pieceMoves(pieces, piece.first, xmove, xmoves);
          piece.second.moves = probmoves;
          for (auto m : probmoves) {
            PieceStates testpieces = pieces;
            testpieces[m] = PieceState({ piece.second.set, piece.second.type, true, PieceMoves() });
            testpieces.erase(piece.first);
            if (piece.second.type == Type::pawn) {
              if (abs(piece.first.file - m.file) == 1 && pieces.find(m) == pieces.end()) {
                testpieces.erase({ m.file, piece.first.rank }); // En passant
              }
            }
            Position king = std::find_if(testpieces.begin(), testpieces.end(), [&](const auto& p) { return p.second.set == activeSet && p.second.type == Type::king; })->first;
            for (auto p : testpieces) {
              if (p.second.set != activeSet) {
                PieceMoves moves = pieceMoves(testpieces, p.first, {});
                if (std::binary_search(moves.begin(), moves.end(), king)) {
                  auto im = std::lower_bound(piece.second.moves.begin(), piece.second.moves.end(), m);
                  assert(*im == m);
                  piece.second.moves.erase(im);
                  break;
                }
              }
            }
          }
        }
      }
    }

    PieceMoves ChessState::pieceMoves(const PieceStates& pieces, const Position& pos, boost::optional<Move> xmove, const SetMoves& xmoves)
    {
      static const std::vector<MoveVector> Lshape_moves = { {-1,+2}, {+1,+2}, {-1,-2}, {+1,-2}, {+2,+1}, {+2,-1}, {-2,+1}, {-2,-1} };
      static const std::vector<MoveVector> diagonal_moves = { { +1,+1 },{ +1,-1 },{ -1,+1 },{ -1,-1 } };
      static const std::vector<MoveVector> straight_moves = { { 0,+1 },{ 0,-1 },{ +1,0 },{ -1,0 } };
      auto merge = [](const std::vector<MoveVector>& a, const std::vector<MoveVector>& b) { std::vector<MoveVector> t(a); t.insert(t.end(), b.begin(), b.end()); return t; };
      static const std::vector<MoveVector> any_moves = merge(straight_moves, diagonal_moves);

      PieceMoves moves;
      const PieceState& piece = pieces.at(pos);
      switch (piece.type) {
       case Type::pawn:
        if (piece.set == Set::white) {
          if (addMoveIf(pieces, moves, { pos.file, pos.rank + 1 }) && !piece.moved && pos.rank == '2') {
            addMoveIf(pieces, moves, { pos.file, pos.rank + 2 });
          }
          addMoveIf(pieces, moves, { pos.file - 1, pos.rank + 1 }, piece.set, true);
          addMoveIf(pieces, moves, { pos.file + 1, pos.rank + 1 }, piece.set, true);
          if (xmove && xmove->type == Type::pawn && pos.rank == '5' && (xmove->from.rank - xmove->to.rank) == 2 && abs(xmove->to.file - pos.file) == 1) {
            addMoveIf(pieces, moves, { xmove->to.file, pos.rank + 1 }); // 'En passant'
          }
        } else { // black
          if (addMoveIf(pieces, moves, { pos.file, pos.rank - 1 }) && !piece.moved && pos.rank == '7') {
            addMoveIf(pieces, moves, { pos.file, pos.rank - 2 });
          }
          addMoveIf(pieces, moves, { pos.file - 1, pos.rank - 1 }, piece.set, true);
          addMoveIf(pieces, moves, { pos.file + 1, pos.rank - 1 }, piece.set, true);
          if (xmove && xmove->type == Type::pawn && pos.rank == '4' && (xmove->to.rank - xmove->from.rank) == 2 && abs(xmove->to.file - pos.file) == 1) {
            addMoveIf(pieces, moves, { xmove->to.file, pos.rank - 1 }); // 'En passant'
          }
        }
        break;
       case Type::knight:
        for (MoveVector v : Lshape_moves) {
          addMoveIf(pieces, moves, pos + v, piece.set);
        }
        break;
       case Type::bishop:
        for (MoveVector v : diagonal_moves) {
          for (Position p = pos + v; addMoveIf(pieces, moves, p, piece.set); p += v);
        }
        break;
       case Type::rook:
        for (MoveVector v : straight_moves) {
          for (Position p = pos + v; addMoveIf(pieces, moves, p, piece.set); p += v);
        }
        break;
       case Type::queen:
        for (MoveVector v : any_moves) {
          for (Position p = pos + v; addMoveIf(pieces, moves, p, piece.set); p += v);
        }
        break;
       case Type::king:
        for (MoveVector v : any_moves) {
          addMoveIf(pieces, moves, pos + v, piece.set);
        }
        const char kingrank = piece.set == Set::white ? '1' : '8';
        if (!piece.moved && pos == Position({ 'e', kingrank })) {
          // O-O
          if ( testPath(pieces, xmoves, { { 'f', kingrank },{'g', kingrank} }) && testPiece(pieces, ROOK(Position({ 'h',kingrank }), piece.set)) ) {
            moves.push_back(Position({'g', kingrank}));
          }
          // O-O-O
          if ( testPath(pieces, xmoves, { { 'd', kingrank },{ 'c', kingrank },{ 'b', kingrank } }) && testPiece(pieces, ROOK(Position({ 'a', kingrank }), piece.set)) ) {
            moves.push_back(Position({ 'c', kingrank }));
          }
        }
        break;
      }
      std::sort(moves.begin(), moves.end());
      return moves;
    }

    bool ChessState::addMoveIf(const PieceStates& pieces, PieceMoves& moves, const Position& pos, Set set, bool capture)
    {
      if (pos.isValid()) {
        if (pieces.find(pos) == pieces.end()) { // it takes more than 25%
          if (!capture) {
            moves.push_back(pos);
            return true;
          }
        } else if (set != Set::unknown && set != pieces.at(pos).set) {
          moves.push_back(pos); // capture
        }
      }
      return false;
    }

    bool ChessState::testPath(const PieceStates& pieces, const SetMoves& xmoves, const PiecePath& path)
    {
      for (auto p : path) {
        if (pieces.find(p) != pieces.end() || xmoves.find(p) != xmoves.end()) {
          return false;
        }
      }
      return true;
    }

    bool ChessState::testPiece(const PieceStates& pieces, const std::pair<Position, PieceState>& piece)
    {
      auto p = pieces.find(piece.first);
      return p != pieces.end() && p->second == piece.second;
    }

  }
}