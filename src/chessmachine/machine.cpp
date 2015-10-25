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

    SetPieces ChessMachine::GetSet(Set set) const
    {
      SetPieces pieces = { 0, { Type::bad, BADPOS } };
      if (!states.empty()) {
        const ChessState& laststate = states.back();
        for (auto p : laststate.pieces) {
          if (p.second.set == set) {
            pieces.pieces[pieces.count].type = p.second.type;
            pieces.pieces[pieces.count].position = p.first;
            pieces.count++;
          }
        }
      }
      return pieces;
    }

    PieceMoves ChessMachine::CheckMoves(Postion from) const
    {
      PieceMoves moves = { 0, {false, BADPOS} };
      if (!states.empty()) {
        const ChessState& laststate = states.back();
        auto ms = laststate.moves.find(from);
        if (ms != laststate.moves.end()) {
          for (const Postion& m : ms->second) {
            moves.moves[moves.count].threat = false; // TODO
            moves.moves[moves.count].move = m;
            moves.count++;
          }
        }
      }
      return moves;
    }

  }
}