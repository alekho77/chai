#pragma once

namespace Chai {
namespace Chess {

struct Move {
  Piece piece;
  Position to;
  Type promotion;
};

typedef boost::container::static_vector<Move, 100> Moves;

class GreedyEngine : public IEngine, private IInfoCall
{
public:
  GreedyEngine();
  //virtual ~GreedyEngine();

  bool Start(const IMachine& position, int depth, int timeout) override;
  void Stop() override;
  void ProcessInfo(IInfoCall* cb) override;
  float EvalPosition(const IMachine& position) const override;

private:
  void SearchDepth(int depth) override;
  void NodesSearched(size_t nodes) override;
  void NodesPerSecond(int nps) override;
  void ReadyOk() override;
  void BestMove(std::string notation) override;
  void BestScore(float score) override;

  void ThreadFun(boost::shared_ptr<IMachine> machine, int maxdepth);
  float Search(IMachine& machine, int depth, std::string *bestmove = nullptr);
  Moves EmunMoves(const IMachine& position) const;
  
  float EvalSide(const IMachine& position, Set set, const Pieces& white, const Pieces& black) const;
  float PieceWeight(Type type) const;
  float PositionWeight(Set set, const Piece& piece, const Pieces& white, const Pieces& black) const;

  boost::asio::io_service service;
  boost::thread thread;
  IInfoCall* callBack;
  volatile bool stopped;
};

}
}

