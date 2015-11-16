#include "stdafx.h"
#include "engine.h"

namespace Chai {
namespace Chess {

GreedyEngine::GreedyEngine() : callBack(nullptr), stopped(true), maxDepth(0) {
}

bool GreedyEngine::Start(const IMachine& position, int depth, int timeout) {
  if (position.CheckStatus() == Status::normal || position.CheckStatus() == Status::check || (depth == 0 && position.CheckStatus() != Status::invalid)) {
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
  stopped = true;
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
  stopped = true;
}

int GreedyEngine::Search(Set set, int depth) {
  if (depth > 0) {
    int maxscore = std::numeric_limits<int>::min();
    std::vector<Move> moves = EmunMoves();
    for (auto move : moves) {
      if (stopped) {
        break;
      }
      if (chessMachine->Move(move.piece.type, move.piece.position, move.to, move.promotion)) {
        int score = - Search(set == Set::white ? Set::black : Set::white, depth - 1);
        if (score > maxscore) {
          maxscore = score;
          if (depth == maxDepth) {
            bestMove.assign(chessMachine->LastMoveNotation());
          }
        }
        chessMachine->Undo();
      }
    }
    return maxscore;
  }
  return EvalPosition(set);
}

std::vector<Move> GreedyEngine::EmunMoves() const
{
  std::vector<Move> moves;
  return moves;
}

int GreedyEngine::EvalPosition(Set set) const {
  if (chessMachine->CheckStatus() == Status::checkmate) {
    return std::numeric_limits<int>::min();
  }
  Set opset = set == Set::white ? Set::black : Set::white;
  std::vector<Piece> white = arr2vec(chessMachine->GetSet(set));
  std::vector<Piece> black = arr2vec(chessMachine->GetSet(opset));
  return EvalSide(set, white, black) - EvalSide(opset, white, black);
}

int GreedyEngine::EvalSide(Set set, const std::vector<Piece>& white, const std::vector<Piece>& black) const {
  int score = 0;
  const std::vector<Piece>& pieces = set == Set::white ? white : black;
  for (auto piece : pieces) {
    score += PieceWeight(piece.type) + PositionWeight(set, piece, white, black);
    for (const Position* pos = chessMachine->CheckMoves(piece.position); *pos != BADPOS; ++pos) {
      score++;
    }
  }
  return score;
}

int GreedyEngine::PieceWeight(Type type) const {
  static const std::map<Type, int> weights = { {Type::pawn, 1000}, {Type::knight, 3000}, {Type::bishop, 3000}, {Type::rook, 5000}, {Type::queen, 9000}, {Type::king, 0} };
  return weights.at(type);
}

int GreedyEngine::PositionWeight(Set set, const Piece & piece, const std::vector<Piece>& white, const std::vector<Piece>& black) const {
  static const int pawn[8][8] =   { { 0,  0,  0,  0,  0,  0,  0,  0},
                                    { 4,  4,  4,  0,  0,  4,  4,  4},
                                    { 6,  8,  2, 10, 10,  2,  8,  6},
                                    { 6,  8, 12, 16, 16, 12,  8,  6},
                                    { 8, 12, 16, 24, 24, 16, 12,  8},
                                    {12, 16, 24, 32, 32, 24, 16, 12},
                                    {12, 16, 24, 32, 32, 24, 16, 12},
                                    { 0,  0,  0,  0,  0,  0,  0,  0} };

  static const int knight[8][8] = { { 0,  4,  8, 10, 10,  8,  4,  0},
                                    { 4,  8, 16, 20, 20, 16,  8,  4},
                                    { 8, 16, 24, 28, 28, 24, 16,  8},
                                    {10, 20, 28, 32, 32, 28, 20, 10},
                                    {10, 20, 28, 32, 32, 28, 20, 10},
                                    { 8, 16, 24, 28, 28, 24, 16,  8},
                                    { 4,  8, 16, 20, 20, 16,  8,  4},
                                    { 0,  4,  8, 10, 10,  8,  4,  0} };

  static const int bishop[8][8] = { {14, 14, 14, 14, 14, 14, 14, 14},
                                    {14, 22, 18, 18, 18, 18, 22, 14},
                                    {14, 18, 22, 22, 22, 22, 18, 14},
                                    {14, 18, 22, 22, 22, 22, 18, 14},
                                    {14, 18, 22, 22, 22, 22, 18, 14},
                                    {14, 18, 22, 22, 22, 22, 18, 14},
                                    {14, 22, 18, 18, 18, 18, 22, 14},
                                    {14, 14, 14, 14, 14, 14, 14, 14} };

  static const int king1[8][8] =  { {  0,   0,  -4, -10, -10,  -4,   0,   0},
                                    { -4,  -4,  -8, -12, -12,  -8,  -4,  -4},
                                    {-12, -16, -20, -20, -20, -20, -16, -12},
                                    {-16, -20, -24, -24, -24, -24, -20, -16},
                                    {-16, -20, -24, -24, -24, -24, -20, -16},
                                    {-12, -16, -20, -20, -20, -20, -16, -12},
                                    { -4,  -4,  -8, -12, -12,  -8,  -4,  -4},
                                    {  0,   0,  -4, -10, -10,  -4,   0,   0} };

  static const int king2[8][8] =  { {  0,   6,  12,  18,  18,  12,   6,   0},
                                    {  6,  12,  18,  24,  24,  18,  12,   6},
                                    { 12,  18,  24,  30,  30,  24,  18,  12},
                                    { 18,  24,  30,  36,  36,  30,  24,  18},
                                    { 18,  24,  30,  36,  36,  30,  24,  18},
                                    { 12,  18,  24,  30,  30,  24,  18,  12},
                                    {  6,  12,  18,  24,  24,  18,  12,   },
                                    {  0,   6,  12,  18,  18,  12,   6,   0} };

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
  case Type::rook:    return 22;
  case Type::queen:
  {
    const auto& opieces = set == Set::white ? black : white;
    auto oking = std::find_if(opieces.begin(), opieces.end(), [](auto p) { return p.type == Type::king; });
    assert(oking != opieces.end());
    int kx = oking->position.file - 'a';
    int ky = oking->position.rank - '1';
    if (set == Set::black) {
      ky = 7 - ky;
    }
    return (2 * 8 * 8 - ((x - kx) * (x - kx) + (y - ky) * (y - ky))) / 4;
  }
  case Type::king:
  {
    const int(&king)[8][8] = std::any_of(white.begin(), white.end(), [](auto p) { return p.type == Type::pawn; }) ||
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
