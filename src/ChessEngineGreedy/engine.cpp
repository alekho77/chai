#include "stdafx.h"
#include "engine.h"

namespace Chai {
namespace Chess {

GreedyEngine::GreedyEngine() : callBack(nullptr), stopped(true), maxDepth(0) {
}

bool GreedyEngine::Start(const IMachine& position, int depth, int timeout) {
  if (position.CheckStatus() == Status::normal || position.CheckStatus() == Status::check) {
    chessMachine.reset();
    chessMachine.reset(position.Clone(), DeleteChessMachine);
    stopped = false;
    maxDepth = depth;
    thread = boost::thread(boost::bind(&GreedyEngine::ThreadFun, this));
    return true;
  }
  return false;
}

void GreedyEngine::Stop() {

}

void GreedyEngine::ProcessInfo(IInfoCall* cb) {
  callBack = cb;
  service.poll();
  service.reset();
  callBack = nullptr;
}

void GreedyEngine::SearchDepth(int depth) {
}

void GreedyEngine::NodesSearched(size_t nodes) {
}

void GreedyEngine::NodesPerSecond(int nps) {
}

void GreedyEngine::ReadyOk() {
  if (callBack) {
    callBack->ReadyOk();
  }
}

void GreedyEngine::BestMove(const char* notation) {
  if (callBack) {
    callBack->BestMove(notation);
  }
}

void GreedyEngine::BestScore(int score) {
  if (callBack) {
    callBack->BestScore(score);
  }
}

void GreedyEngine::ThreadFun() {
  bestMove.clear();
  int bestscore = Search(chessMachine->CurrentMove(), maxDepth);
  service.post(boost::bind(&GreedyEngine::BestScore, this, bestscore));
  service.post(boost::bind(&GreedyEngine::BestMove, this, bestMove.c_str()));
  service.post(boost::bind(&GreedyEngine::ReadyOk, this));
}

int GreedyEngine::Search(Set set, int depth) {
  if (depth > 0) {
    int maxscore = std::numeric_limits<int>::min();

    return maxscore;
  }
  return EvalPosition(set);
}

int GreedyEngine::EvalPosition(Set set) const {
  if (chessMachine->CheckStatus() == Status::checkmate) {
    return std::numeric_limits<int>::min();
  }
  return EvalSide(set) - EvalSide(set == Set::white ? Set::black : Set::white);
}

int GreedyEngine::EvalSide(Set set) const {
  int score = 0;
  for (const Piece* piece = chessMachine->GetSet(set); piece->type != Type::bad; ++piece) {
    score += PieceWeight(piece->type);
    for (const Position* pos = chessMachine->CheckMoves(piece->position); *pos != BADPOS; ++pos) {
      score++;
    }
  }
  return score;
}

int GreedyEngine::PieceWeight(Type type) const {
  static const std::map<Type, int> weights = { {Type::pawn, 1000}, {Type::knight, 3000}, {Type::bishop, 3000}, {Type::rook, 5000}, {Type::queen, 9000}, {Type::king, 0} };
  return weights.at(type);
}

}
}
