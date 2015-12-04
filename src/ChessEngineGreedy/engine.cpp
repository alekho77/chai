#include "stdafx.h"
#include "engine.h"

boost::shared_ptr<Chai::Chess::IEngine> CreateGreedyEngine()
{
  return boost::shared_ptr<Chai::Chess::IEngine>(new Chai::Chess::GreedyEngine());
}

namespace Chai {
namespace Chess {

GreedyEngine::GreedyEngine() : callBack(nullptr), aborted(false) {
}

bool GreedyEngine::Start(const IMachine& position, int depth) {
  if (position.CheckStatus() == Status::normal || position.CheckStatus() == Status::check || (depth == 0 && position.CheckStatus() != Status::invalid)) {
    aborted = false;
    thread = boost::thread(boost::bind(&GreedyEngine::ThreadFun, this, position.Clone(), depth));
    return true;
  }
  return false;
}

void GreedyEngine::Stop() {
  aborted = true;
  thread.join();
}

void GreedyEngine::ProcessInfo(IInfoCall* cb) {
  callBack = cb;
  service.poll();
  service.reset();
  callBack = nullptr;
}

float GreedyEngine::EvalPosition(const IMachine & position) const
{
  if (position.CheckStatus() == Status::checkmate) {
    return -std::numeric_limits<float>::infinity();
  }
  if (position.CheckStatus() == Status::stalemate || position.CheckStatus() == Status::invalid) {
    return 0;
  }
  Set set = position.CurrentPlayer();
  Set xset = (set == Set::white) ? Set::black : Set::white;
  Pieces pieces = position.GetSet(set);
  Pieces xpieces = position.GetSet(xset);
  return EvalSide(position, set, pieces, xpieces) - EvalSide(position, xset, xpieces, pieces);
}

void GreedyEngine::NodesSearched(size_t nodes) {
  if (callBack) {
    callBack->NodesSearched(nodes);
  }
}

void GreedyEngine::NodesPerSecond(int nps) {
  if (callBack) {
    callBack->NodesPerSecond(nps);
  }
}

void GreedyEngine::ReadyOk() {
  if (callBack) {
    callBack->ReadyOk();
  }
}

void GreedyEngine::BestMove(std::string notation) {
  if (callBack) {
    callBack->BestMove(notation);
  }
}

void GreedyEngine::BestScore(float score) {
  if (callBack) {
    callBack->BestScore(score);
  }
}

void GreedyEngine::ThreadFun(boost::shared_ptr<IMachine> machine, int maxdepth) {
  std::string bestmove;
  size_t searched_nodes = 0;
  float bestscore = Search(*machine, maxdepth, searched_nodes, -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), &bestmove);
  service.post(boost::bind(&GreedyEngine::NodesSearched, this, searched_nodes));
  service.post(boost::bind(&GreedyEngine::BestScore, this, bestscore));
  service.post(boost::bind(&GreedyEngine::BestMove, this, bestmove));
  service.post(boost::bind(&GreedyEngine::ReadyOk, this));
}

float GreedyEngine::Search(IMachine& machine, int depth, size_t& nodes, float alpha, const float betta, std::string *bestmove) {
  Status status = machine.CheckStatus();
  if (depth > 0 && status != Status::checkmate && status != Status::stalemate) {
    Moves moves = EmunMoves(machine);
    assert(!moves.empty());
    bool first_move = !!bestmove;
    boost::container::flat_set<Position> xpos;
    for (const auto& xp : machine.GetSet(machine.CurrentPlayer() == Set::white ? Set::black : Set::white)) {
      xpos.insert(xp.position);
    }
    for (auto move : moves) {
      if (aborted || alpha >= betta) {
        break;
      }
      if (machine.Move(move.piece.type, move.piece.position, move.to, move.promotion)) {
        bool forcing = depth == 1 && (machine.CheckStatus() == Status::check || xpos.find(move.to) != xpos.end()); // todo: en passat
        float score = - Search(machine, forcing ? depth : depth - 1, nodes, -betta, -alpha);
        if (first_move || score > alpha) {
          first_move = false;
          if (score > alpha) {
            alpha = score;
          }
          service.post(boost::bind(&GreedyEngine::NodesSearched, this, nodes));
          service.post(boost::bind(&GreedyEngine::BestScore, this, score));
          if (bestmove) {
            *bestmove = machine.LastMoveNotation();
            service.post(boost::bind(&GreedyEngine::BestMove, this, *bestmove));
          }
        }
        machine.Undo();
      } else {
        assert(!"Can't make move!");
      }
    }
    return alpha;
  }
  ++nodes;
  return EvalPosition(machine);
}

Moves GreedyEngine::EmunMoves(const IMachine& position) const
{
  Moves moves;
  for (const auto& piece : position.GetSet(position.CurrentPlayer())) {
    for (const auto& move : position.EnumMoves(piece.position)) {
      if (piece.type == Type::pawn && (move.rank() == '1' || move.rank() == '8')) {
        for (auto type : { Type::knight, Type::bishop, Type::rook, Type::queen }) {
          moves.push_back({ piece, move, type });
        }
      } else {
        moves.push_back({ piece, move, Type::bad });
      }
    }
  }
  return moves;
}

float GreedyEngine::EvalSide(const IMachine & position, Set set, const Pieces& pieces, const Pieces& xpieces) const {
  float score = 0;
  for (const auto& piece : pieces) {
    score += PieceWeight(piece.type) + PositionWeight(set, piece, pieces, xpieces) + 0.001f * position.EnumMoves(piece.position).size();
  }
  return score;
}

float GreedyEngine::PieceWeight(Type type) const {
  static const std::map<Type, float> weights = { {Type::pawn, 1.0f}, {Type::knight, 3.0f}, {Type::bishop, 3.0f}, {Type::rook, 5.0f}, {Type::queen, 9.0f}, {Type::king, 0.0f} };
  return weights.at(type);
}

float GreedyEngine::PositionWeight(Set set, const Piece & piece, const Pieces& pieces, const Pieces& xpieces) const {
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

  int x = piece.position.x();
  assert(x >= 0 && x < 8);
  int y = piece.position.y();
  assert(y >= 0 && y <8);
  switch (piece.type) {
  case Type::pawn:
    if (set == Set::black) {
      y = 7 - y;
    }
    return pawn[y][x];
  case Type::knight:  return knight[y][x];
  case Type::bishop:  return bishop[y][x];
  case Type::rook:    return 0;
  case Type::queen:
  {
    auto xking = std::find_if(xpieces.begin(), xpieces.end(), [](auto p) { return p.type == Type::king; });
    assert(xking != xpieces.end());
    return (2 * 8 * 8 - ((x - xking->position.x()) * (x - xking->position.x()) + (y - xking->position.y()) * (y - xking->position.y()))) / 4000.0f;
  }
  case Type::king:
  {
    const float(&king)[8][8] = std::any_of(pieces.begin(), pieces.end(), [](auto p) { return p.type == Type::pawn; }) ||
                             std::any_of(xpieces.begin(), xpieces.end(), [](auto p) { return p.type == Type::pawn; }) ? king1 : king2;
    return king[y][x];
  }
  default:
    assert(!"Bad piece type");
  }
  return 0;
}

}
}
