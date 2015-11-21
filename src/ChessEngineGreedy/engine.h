#pragma once
#include "../chessmachine/chessmachine.h"

namespace Chai {
namespace Chess {

inline std::vector<Piece> arr2vec(const Piece* p) {
  std::vector<Piece> vec;
  if (p) {
    while (p->type != Type::bad) {
      vec.push_back(*(p++));
    }
  }
  return vec;
}

struct Move {
  Piece piece;
  Position to;
  Type promotion;
};

class GreedyEngine : public IEngine, private IInfoCall
{
public:
  GreedyEngine();
  //virtual ~GreedyEngine();

  bool Start(const IMachine& position, int depth, int timeout) override;
  void Stop() override;
  void ProcessInfo(IInfoCall* cb) override;
  int EvalPosition(const IMachine& position) const override;

private:
  void SearchDepth(int depth) override;
  void NodesSearched(size_t nodes) override;
  void NodesPerSecond(int nps) override;
  void ReadyOk() override;
  void BestMove(const char* notation) override;
  void BestScore(int score) override;

  void ThreadFun(boost::shared_ptr<IMachine> machine);
  int Search(boost::shared_ptr<IMachine> machine, Set set, int depth);
  std::vector<Move> EmunMoves() const;
  
  Set xSet(Set set) const { return set == Set::white ? Set::black : (set == Set::black ? Set::white : Set::unknown); }
  int EvalPosition(const IMachine & position, Set set) const;
  int EvalSide(const IMachine & position, Set set, const std::vector<Piece>& white, const std::vector<Piece>& black) const;
  int PieceWeight(Type type) const;
  int PositionWeight(Set set, const Piece& piece, const std::vector<Piece>& white, const std::vector<Piece>& black) const;

  boost::asio::io_service service;
  boost::thread thread;
  IInfoCall* callBack;
  volatile bool stopped;
  int maxDepth;
  std::string bestMove;
};

}
}

