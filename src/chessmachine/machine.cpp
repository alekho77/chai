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

    bool ChessMachine::Move(Type type, Postion from, Postion to)
    {
      return false;
    }

    bool ChessMachine::Move(const char* notation)
    {
      return false;
    }

    const Piece* ChessMachine::GetSet(Set set) const
    {
      piecesSet.clear();
      if (!states.empty()) {
        const ChessState& laststate = states.back();
        piecesSet.reserve(laststate.pieces.size() + 1);
        for (auto p : laststate.pieces) {
          if (p.second.set == set) {
            piecesSet.push_back({p.second.type, p.first});
          }
        }
        piecesSet.push_back({ Type::bad, BADPOS });
        return &piecesSet[0];
      }
      return nullptr;
    }

    const Postion* ChessMachine::CheckMoves(Postion from) const
    {
      pieceMoves.clear();
      if (!states.empty()) {
        const ChessState& laststate = states.back();
        auto ms = laststate.moves.find(from);
        if (ms != laststate.moves.end()) {
          for (const Postion& m : ms->second) {
            pieceMoves.push_back(m);
          }
          pieceMoves.push_back(BADPOS);
          return &pieceMoves[0];
        }
      }
      return nullptr;
    }

  }
}