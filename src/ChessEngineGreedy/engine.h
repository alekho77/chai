#pragma once
#include "../chessmachine/chessmachine.h"

namespace Chai {
namespace Chess {

class GreedyEngine : public IEngine, private IInfoCall
{
public:
  GreedyEngine();
  //virtual ~GreedyEngine();

  bool Start(const IMachine& position, int depth, int timeout) override;
  void Stop() override;
  void ProcessInfo(IInfoCall* cb) override;

private:
  void SearchDepth(int depth) override;
  void NodesSearched(size_t nodes) override;
  void NodesPerSecond(int nps) override;
  void ReadyOk() override;
  void BestMove(const char* notation) override;
  void BestScore(int score) override;

  void ThreadFun();
  int Search(Set set, int depth);
  int EvalPosition(Set set) const;
  int EvalSide(Set set) const;
  int PieceWeight(Type type) const;

  boost::shared_ptr<IMachine> chessMachine;
  boost::asio::io_service service;
  boost::thread thread;
  IInfoCall* callBack;
  volatile bool stopped;
  int maxDepth;
  std::string bestMove;
};

}
}

