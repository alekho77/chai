#pragma once

namespace Chai {
namespace Chess {

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
  float EvalPosition(const IMachine& position) const override;

private:
  void SearchDepth(int depth) override;
  void NodesSearched(size_t nodes) override;
  void NodesPerSecond(int nps) override;
  void ReadyOk() override;
  void BestMove(std::string notation) override;
  void BestScore(float score) override;

  void ThreadFun(boost::shared_ptr<IMachine> machine);
  float Search(IMachine& machine, Set set, int depth);
  std::vector<Move> EmunMoves(const IMachine& position) const;
  
  Set xSet(Set set) const { return set == Set::white ? Set::black : (set == Set::black ? Set::white : Set::unknown); }
  float EvalPosition(const IMachine& position, Set set) const;
  float EvalSide(const IMachine& position, Set set, const Pieces& white, const Pieces& black) const;
  float PieceWeight(Type type) const;
  float PositionWeight(Set set, const Piece& piece, const Pieces& white, const Pieces& black) const;

  boost::asio::io_service service;
  boost::thread thread;
  IInfoCall* callBack;
  volatile bool stopped;
  int maxDepth;
  std::string bestMove;
};

}
}

