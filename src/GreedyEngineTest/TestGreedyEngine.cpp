#include "stdafx.h"

namespace Chai {
namespace Chess {

class InfoTest : public IInfoCall {
public:
  InfoTest() : readyok(false), deadline(false) {}

  void SearchDepth(int depth) override {}
  void NodesSearched(size_t nodes) override {}
  void NodesPerSecond(int nps) override {}

  void ReadyOk() override { readyok = true; }
  void BestMove(const char* notation) override { bestmove.assign(notation); }

  std::string GetBestMove() const { return bestmove; }
  bool Wait(IEngine* engine, int timeout) {
    if (deadline) {
      return false; // wait only once
    }
    boost::asio::io_service service;
    boost::asio::deadline_timer timer(service);
    timer.expires_from_now(boost::posix_time::milliseconds(timeout));
    timer.async_wait(boost::bind(&InfoTest::onTimeout, this, _1));
    boost::thread thread = boost::thread(boost::bind(&boost::asio::io_service::run, &service));
    while (!readyok && !deadline) {
      engine->ProcessInfo(this);
    }
    service.stop();
    thread.join();
    deadline = true;
    return readyok;
  }

private:
  void onTimeout(const boost::system::error_code& e) {
    if (e != boost::asio::error::operation_aborted) {
      deadline = true;
    }
  }

  volatile bool readyok;
  volatile bool deadline;
  std::string bestmove;
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

  BOOST_REQUIRE(!engine->Start(*machine, 0));
}

BOOST_AUTO_TEST_CASE( StartTest )
{
  using namespace Chai::Chess;
  boost::shared_ptr<IEngine> engine(CreateGreedyEngine(), DeleteGreedyEngine);
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  {
    InfoTest info;
    BOOST_REQUIRE(engine->Start(*machine, 0));
    BOOST_CHECK(info.Wait(&*engine, 5000));
  }


}

BOOST_AUTO_TEST_SUITE_END()