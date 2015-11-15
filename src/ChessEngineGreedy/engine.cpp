#include "stdafx.h"
#include "engine.h"

namespace Chai {
namespace Chess {

GreedyEngine::GreedyEngine() {
}

bool GreedyEngine::Start(const IMachine& position, int depth, int timeout) {
  if (position.CheckStatus() != Status::invalid) {
    chessMachine.reset();
    chessMachine.reset(position.Clone(), DeleteChessMachine);
    return true;
  }
  return false;
}

void GreedyEngine::Stop() {

}

void GreedyEngine::ProcessInfo(const IInfoCall* cb) {

}

}
}
