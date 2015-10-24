#include "stdafx.h"
#include "machine.h"

namespace Chai {
  namespace Chess {
    ChessMachine::ChessMachine()
    {

    }

    void ChessMachine::Start()
    {
      states.clear();
      states.push_back(ChessState());
    }

    bool ChessMachine::GetSnapshot(Snapshot& snapshot) const
    {
      snapshot = Snapshot({ { Set::unknown, Type::bad, BADPOS, { false, BADPOS } } });
      if (!states.empty())
      {
        const ChessState& laststate = states.back();
        Snapshot::Piece* s = snapshot.pieces;
        for (const auto& p : laststate.pieces)
        {
          s->set = p.set;
          s->type = p.type;
          s->position = p.position;
          ++s;
        }
        return true;
      }
      return false;
    }
  }
}