#include "stdafx.h"
#include "machine.h"

namespace Chai {
  namespace Chess {
    ChessMachine::ChessMachine() {

    }

    void ChessMachine::Start() {
      states.clear();
      states.push_back(ChessState());
    }

    bool ChessMachine::GetSnapshot(Snapshot& snapshot) const {
      snapshot = Snapshot({ { Set::unknown, Type::bad, BADPOS, { BADPOS } } });
      if (!states.empty()) {
        const ChessState& laststate = states.back();
        Snapshot::Piece* piece = snapshot.pieces;
        for (const auto& p : laststate.pieces) {
          piece->set = p.second.set;
          piece->type = p.second.type;
          piece->position = p.first;
          const std::vector<Postion> moves = pieceMoves(p.first, laststate);
          Postion* move = piece->moves;
          for (const auto m : moves) {
            *move = m;
            ++move;
          }
          ++piece;
        }
        return true;
      }
      return false;
    }

    std::vector<Postion> ChessMachine::pieceMoves(const Postion& pos, const ChessState& state) const {
      std::vector<Postion> moves;
      auto piece = state.pieces.find(pos);
      assert(piece != state.pieces.end());
      switch (piece->second.type) {
       case Type::pawn:
        if (piece->second.set == Set::white) {
          if (addMoveIf(moves, { pos.file, pos.rank + 1 }, state) && !piece->second.moved && pos.rank == '2') {
            addMoveIf(moves, { pos.file, pos.rank + 2 }, state);
          }
          // TODO: added 'capture' moves
          // TODO: added move 'En passant'
        } else {
          if (addMoveIf(moves, { pos.file, pos.rank - 1 }, state) && !piece->second.moved && pos.rank == '7') {
            addMoveIf(moves, { pos.file, pos.rank - 2 }, state);
          }
          // TODO: added 'capture' moves
          // TODO: added move 'En passant'
        }
        break;
       case Type::knight:
        for (MoveVector v : std::vector<MoveVector>({ {-1,+2}, {+1,+2}, {-1,-2}, {+1,-2}, {+2,+1}, {+2,-1}, {-2,+1}, {-2,-1} })) {
          addMoveIf(moves, pos + v, state);
        }
        // TODO: added 'capture' moves
        break;
       case Type::bishop:
        for (MoveVector v : std::vector<MoveVector>({ {+1,+1}, {+1,-1}, {-1,+1}, {-1,-1} })) {
          for (Postion p = pos + v; addMoveIf(moves, p, state); p += v);
        }
        // TODO: added 'capture' moves
        break;
       case Type::rook:
        for (MoveVector v : std::vector<MoveVector>({ {0,+1}, {0,-1}, {+1,0}, {-1,0} })) {
          for (Postion p = pos + v; addMoveIf(moves, p, state); p += v);
        }
        // TODO: added 'capture' moves
        break;
       case Type::queen:
        for (MoveVector v : std::vector<MoveVector>({ {+1,+1}, {+1,-1}, {-1,+1}, {-1,-1}, {0,+1}, {0,-1}, {+1,0}, {-1,0} })) {
          for (Postion p = pos + v; addMoveIf(moves, p, state); p += v);
        }
        // TODO: added 'capture' moves
        break;
      }
      return moves;
    }

    bool ChessMachine::addMoveIf(std::vector<Postion>& moves, const Postion& pos, const ChessState& state) const {
      if (pos.isValid() && state.pieces.find(pos) == state.pieces.end()) {
        moves.push_back(pos);
        return true;
      }
      return false;
    }

  }
}