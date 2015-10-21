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
      snapshot = Snapshot({ { Type::bad, {0, 0}, { false, {0, 0} } }, { Type::bad, {0, 0}, { false, {0, 0} } } });
      if (!states.empty())
      {
        const ChessState& laststate = states.back();
      }
      return false;
    }
  }
}