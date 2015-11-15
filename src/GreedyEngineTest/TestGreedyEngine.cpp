#include "stdafx.h"

namespace Chai {
namespace Chess {

class InfoTest : public IInfoCall {
public:
  InfoTest() : readyok(false) {}

  void SearchDepth(int depth) override {}
  void NodesSearched(size_t nodes) override {}
  void NodesPerSecond(int nps) override {}

  void ReadyOk() override {}
  void BestMove(const char* notation) override {}

private:
  bool readyok;
};

}
}

BOOST_AUTO_TEST_SUITE( GreedyEngineTest )

BOOST_AUTO_TEST_CASE( ConstructorTest )
{
  using namespace Chai::Chess;
  boost::shared_ptr<IEngine> engine(CreateGreedyEngine(), DeleteGreedyEngine);
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");

  BOOST_REQUIRE(!engine->Start(nullptr, *machine, 0));
}

BOOST_AUTO_TEST_CASE( StartTest )
{
  using namespace Chai::Chess;
  boost::shared_ptr<IEngine> engine(CreateGreedyEngine(), DeleteGreedyEngine);
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  InfoTest info;
  BOOST_REQUIRE(engine->Start(&info, *machine, 0));

  while (true)
  {

  }
}

BOOST_AUTO_TEST_SUITE_END()