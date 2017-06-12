#pragma once

#include "chessmachine.h"

#include <SDKDDKVer.h>

#include <boost/tuple/tuple.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/container/small_vector.hpp>

namespace Chai {
namespace Chess {

struct Move {
  Piece piece;
  Position to;
  Type promotion;
};

typedef boost::container::small_vector<Move, 50> Moves;

class GreedyEngine : public IEngine, private IInfoCall
{
  typedef boost::tuple<bool, boost::shared_ptr<IMachine>, Move> TaskData;
public:
  GreedyEngine();
  //virtual ~GreedyEngine();

  bool Start(const IMachine& position, int depth) override;
  void Stop() override;
  void ProcessInfo(IInfoCall* cb) override;
  float EvalPosition(const IMachine& position) const override;

private:
  void NodesSearched(size_t nodes) override;
  void NodesPerSecond(int nps) override;
  void ReadyOk() override;
  void BestMove(std::string notation) override;
  void BestScore(float score) override;

  void ThreadFun(boost::shared_ptr<IMachine> machine, int maxdepth);
  float Search(const IMachine& machine, int depth, size_t& nodes, float alpha, const float betta, std::string *bestmove = nullptr);
  Moves EmunMoves(const IMachine& position) const;
  void TaskFun(TaskData& data);

  float EvalSide(const IMachine& position, Set set, const Pieces& white, const Pieces& black) const;
  float PieceWeight(Type type) const;
  float PositionWeight(Set set, const Piece& piece, const Pieces& white, const Pieces& black) const;

  boost::asio::io_service cbservice;
  boost::thread mainthread;
  IInfoCall* callBack;
  volatile bool aborted;

  boost::asio::io_service taskservice;
  boost::condition_variable condtasks;
  boost::mutex muttasks;
  int workingtasks;
  const int maxthreads = std::max(1u, boost::thread::hardware_concurrency());
};

}
}

