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
          auto moves = laststate.moves.find(p.first);
          if (moves != laststate.moves.end()) {
            Postion* move = piece->moves;
            for (const auto m : moves->second) {
              *move = m;
              ++move;
            }
          }
          ++piece;
        }
        return true;
      }
      return false;
    }

  }
}