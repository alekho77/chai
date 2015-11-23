#include "stdafx.h"
#include "engine.h"

namespace Chai {
namespace Chess {

GreedyEngine::GreedyEngine() : callBack(nullptr), stopped(true), maxDepth(0) {
}

bool GreedyEngine::Start(const IMachine& position, int depth, int timeout) {
  if (position.CheckStatus() == Status::normal || position.CheckStatus() == Status::check || (depth == 0 && position.CheckStatus() != Status::invalid)) {
    stopped = false;
    maxDepth = depth;
    thread = boost::thread(boost::bind(&GreedyEngine::ThreadFun, this, boost::shared_ptr<IMachine>(position.Clone(), DeleteChessMachine)));
    return true;
  }
  return false;
}

void GreedyEngine::Stop() {
  stopped = true;
}

void GreedyEngine::ProcessInfo(IInfoCall* cb) {
  callBack = cb;
  service.poll();
  service.reset();
  callBack = nullptr;
}

float GreedyEngine::EvalPosition(const IMachine & position) const
{
  return EvalPosition(position, Set::white);
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

void GreedyEngine::BestScore(float score) {
  if (callBack) {
    callBack->BestScore(score);
  }
}

void GreedyEngine::ThreadFun(boost::shared_ptr<IMachine> machine) {
  bestMove.clear();
  float bestscore = Search(*machine, machine->CurrentPlayer(), maxDepth);
  service.post(boost::bind(&GreedyEngine::BestScore, this, bestscore));
  service.post(boost::bind(&GreedyEngine::BestMove, this, bestMove.c_str()));
  service.post(boost::bind(&GreedyEngine::ReadyOk, this));
  stopped = true;
}

float GreedyEngine::Search(IMachine& machine, Set set, int depth) {
  if (depth > 0) {
    float maxscore = - std::numeric_limits<float>::infinity();
    std::vector<Move> moves = EmunMoves();
    for (auto move : moves) {
      if (stopped) {
        break;
      }
      if (machine.Move(move.piece.type, move.piece.position, move.to, move.promotion)) {
        float score = - Search(machine, xSet(set), depth - 1);
        if (score > maxscore) {
          maxscore = score;
          if (depth == maxDepth) {
            bestMove.assign(machine.LastMoveNotation());
          }
        }
        machine.Undo();
      } else {
        assert(!"Can't make move!");
      }
    }
    return maxscore;
  }
  return EvalPosition(machine, set);
}

std::vector<Move> GreedyEngine::EmunMoves() const
{
  std::vector<Move> moves;
  // TODO ...
  return moves;
}

float GreedyEngine::EvalPosition(const IMachine & position, Set set) const {
  if (position.CheckStatus() == Status::checkmate) {
    return - std::numeric_limits<float>::infinity();
  }
  std::vector<Piece> white = arr2vec(position.GetSet(Set::white));
  std::vector<Piece> black = arr2vec(position.GetSet(Set::black));
  return EvalSide(position, set, white, black) - EvalSide(position, xSet(set), white, black);
}

float GreedyEngine::EvalSide(const IMachine & position, Set set, const std::vector<Piece>& white, const std::vector<Piece>& black) const {
  float score = 0;
  const std::vector<Piece>& pieces = set == Set::white ? white : black;
  for (auto piece : pieces) {
    score += PieceWeight(piece.type) + PositionWeight(set, piece, white, black);
    for (const Position* pos = position.CheckMoves(piece.position); *pos != BADPOS; ++pos) {
      score += 0.001f;
    }
  }
  return score;
}

float GreedyEngine::PieceWeight(Type type) const {
  static const std::map<Type, float> weights = { {Type::pawn, 1.0f}, {Type::knight, 3.0f}, {Type::bishop, 3.0f}, {Type::rook, 5.0f}, {Type::queen, 9.0f}, {Type::king, 0.0f} };
  return weights.at(type);
}

float GreedyEngine::PositionWeight(Set set, const Piece & piece, const std::vector<Piece>& white, const std::vector<Piece>& black) const {
  static const float pawn[8][8] =   { {  0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f },
                                      {  0.004f, 0.004f, 0.004f, 0.000f, 0.000f, 0.004f, 0.004f, 0.004f },
                                      {  0.006f, 0.008f, 0.002f, 0.010f, 0.010f, 0.002f, 0.008f, 0.006f },
                                      {  0.006f, 0.008f, 0.012f, 0.016f, 0.016f, 0.012f, 0.008f, 0.006f },
                                      {  0.008f, 0.012f, 0.016f, 0.024f, 0.024f, 0.016f, 0.012f, 0.008f },
                                      {  0.012f, 0.016f, 0.024f, 0.032f, 0.032f, 0.024f, 0.016f, 0.012f },
                                      {  0.012f, 0.016f, 0.024f, 0.032f, 0.032f, 0.024f, 0.016f, 0.012f },
                                      {  0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f } };

  static const float knight[8][8] = { {  0.000f, 0.004f, 0.008f, 0.010f, 0.010f, 0.008f, 0.004f, 0.000f },
                                      {  0.004f, 0.008f, 0.016f, 0.020f, 0.020f, 0.016f, 0.008f, 0.004f },
                                      {  0.008f, 0.016f, 0.024f, 0.028f, 0.028f, 0.024f, 0.016f, 0.008f },
                                      {  0.010f, 0.020f, 0.028f, 0.032f, 0.032f, 0.028f, 0.020f, 0.010f },
                                      {  0.010f, 0.020f, 0.028f, 0.032f, 0.032f, 0.028f, 0.020f, 0.010f },
                                      {  0.008f, 0.016f, 0.024f, 0.028f, 0.028f, 0.024f, 0.016f, 0.008f },
                                      {  0.004f, 0.008f, 0.016f, 0.020f, 0.020f, 0.016f, 0.008f, 0.004f },
                                      {  0.000f, 0.004f, 0.008f, 0.010f, 0.010f, 0.008f, 0.004f, 0.000f } };

  static const float bishop[8][8] = { {  0.014f, 0.014f, 0.014f, 0.014f, 0.014f, 0.014f, 0.014f, 0.014f },
                                      {  0.014f, 0.022f, 0.018f, 0.018f, 0.018f, 0.018f, 0.022f, 0.014f },
                                      {  0.014f, 0.018f, 0.022f, 0.022f, 0.022f, 0.022f, 0.018f, 0.014f },
                                      {  0.014f, 0.018f, 0.022f, 0.022f, 0.022f, 0.022f, 0.018f, 0.014f },
                                      {  0.014f, 0.018f, 0.022f, 0.022f, 0.022f, 0.022f, 0.018f, 0.014f },
                                      {  0.014f, 0.018f, 0.022f, 0.022f, 0.022f, 0.022f, 0.018f, 0.014f },
                                      {  0.014f, 0.022f, 0.018f, 0.018f, 0.018f, 0.018f, 0.022f, 0.014f },
                                      {  0.014f, 0.014f, 0.014f, 0.014f, 0.014f, 0.014f, 0.014f, 0.014f } };

  static const float king1[8][8] =  { {  0.000f, 0.000f,-0.004f,-0.010f,-0.010f,-0.004f, 0.000f, 0.000f },
                                      { -0.004f,-0.004f,-0.008f,-0.012f,-0.012f,-0.008f,-0.004f,-0.004f },
                                      { -0.012f,-0.016f,-0.020f,-0.020f,-0.020f,-0.020f,-0.016f,-0.012f },
                                      { -0.016f,-0.020f,-0.024f,-0.024f,-0.024f,-0.024f,-0.020f,-0.016f },
                                      { -0.016f,-0.020f,-0.024f,-0.024f,-0.024f,-0.024f,-0.020f,-0.016f },
                                      { -0.012f,-0.016f,-0.020f,-0.020f,-0.020f,-0.020f,-0.016f,-0.012f },
                                      { -0.004f,-0.004f,-0.008f,-0.012f,-0.012f,-0.008f,-0.004f,-0.004f },
                                      {  0.000f, 0.000f,-0.004f,-0.010f,-0.010f,-0.004f, 0.000f, 0.000f } };

  static const float king2[8][8] =  { {  0.000f, 0.006f, 0.012f, 0.018f, 0.018f, 0.012f, 0.006f, 0.000f },
                                      {  0.006f, 0.012f, 0.018f, 0.024f, 0.024f, 0.018f, 0.012f, 0.006f },
                                      {  0.012f, 0.018f, 0.024f, 0.030f, 0.030f, 0.024f, 0.018f, 0.012f },
                                      {  0.018f, 0.024f, 0.030f, 0.036f, 0.036f, 0.030f, 0.024f, 0.018f },
                                      {  0.018f, 0.024f, 0.030f, 0.036f, 0.036f, 0.030f, 0.024f, 0.018f },
                                      {  0.012f, 0.018f, 0.024f, 0.030f, 0.030f, 0.024f, 0.018f, 0.012f },
                                      {  0.006f, 0.012f, 0.018f, 0.024f, 0.024f, 0.018f, 0.012f, 0.006f },
                                      {  0.000f, 0.006f, 0.012f, 0.018f, 0.018f, 0.012f, 0.006f, 0.000f } };

  int x = piece.position.file - 'a';
  assert(x >= 0 && x < 8);
  int y = piece.position.rank - '1';
  if (set == Set::black) {
    y = 7 - y;
  }
  assert(y >= 0 && y <8);
  switch (piece.type) {
  case Type::pawn:    return pawn[y][x];
  case Type::knight:  return knight[y][x];
  case Type::bishop:  return bishop[y][x];
  case Type::rook:    return 0;
  case Type::queen:
  {
    const auto& xpieces = set == Set::white ? black : white;
    auto xking = std::find_if(xpieces.begin(), xpieces.end(), [](auto p) { return p.type == Type::king; });
    assert(xking != xpieces.end());
    int kx = xking->position.file - 'a';
    int ky = xking->position.rank - '1';
    if (set == Set::black) {
      ky = 7 - ky;
    }
    return (2 * 8 * 8 - ((x - kx) * (x - kx) + (y - ky) * (y - ky))) / 4000.0f;
  }
  case Type::king:
  {
    const float(&king)[8][8] = std::any_of(white.begin(), white.end(), [](auto p) { return p.type == Type::pawn; }) ||
                             std::any_of(black.begin(), black.end(), [](auto p) { return p.type == Type::pawn; }) ? king1 : king2;
    return king[y][x];
  }
  default:
    assert(!"Bad piece type");
  }
  return 0;
}

}
}
