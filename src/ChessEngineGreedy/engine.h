#pragma once

#include <Interfaces/chessmachine.h>

#include <boost/tuple/tuple.hpp>
#include <boost/container/small_vector.hpp>

#include <SDKDDKVer.h>
#pragma warning(push)
#pragma warning(disable:4005) // warning C4005: 'BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT': macro redefinition
#include <boost/asio.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable:4100) // future.hpp(410): warning C4100: 'lk': unreferenced formal parameter
#pragma warning(disable:4459) // bind.hpp(227): warning C4459: declaration of 'a1' hides global declaration
#include <boost/thread.hpp>
#pragma warning(pop)

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
  ~GreedyEngine() override;

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

