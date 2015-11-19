#include "stdafx.h"

using namespace Chai::Chess;

CHESSBOARD;

class infotest : private IInfoCall {
public:
  infotest() : readyok(false), deadline(false), bestscore(0) {}

  std::string bestmove;
  int bestscore;

  bool wait(IEngine* engine, int timeout) {
    if (deadline) {
      return false; // wait only once
    }
    boost::asio::io_service service;
    boost::asio::deadline_timer timer(service);
    timer.expires_from_now(boost::posix_time::milliseconds(timeout));
    timer.async_wait(boost::bind(&infotest::on_timeout, this, _1));
    boost::thread thread = boost::thread(boost::bind(&boost::asio::io_service::run, &service));
    while (!readyok && !deadline) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(1));
      engine->ProcessInfo(this);
    }
    service.stop();
    thread.join();
    deadline = true;
    return readyok;
  }

private:
  void SearchDepth(int depth) override {}
  void NodesSearched(size_t nodes) override {}
  void NodesPerSecond(int nps) override {}

  void ReadyOk() override { readyok = true; }
  void BestMove(const char* notation) override { bestmove.assign(notation); }
  void BestScore(int score) override { bestscore = score; }

  void on_timeout(const boost::system::error_code& e) {
    if (e != boost::asio::error::operation_aborted) {
      deadline = true;
    }
  }

  volatile bool readyok;
  volatile bool deadline;
};


BOOST_AUTO_TEST_SUITE( GreedyEngineTest )

BOOST_AUTO_TEST_CASE( ConstructorTest )
{
  boost::shared_ptr<IEngine> engine(CreateGreedyEngine(), DeleteGreedyEngine);
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");

  BOOST_REQUIRE(!engine->Start(*machine, 0));
}

BOOST_AUTO_TEST_CASE( StartTest )
{
  boost::shared_ptr<IEngine> engine(CreateGreedyEngine(), DeleteGreedyEngine);
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  {
    infotest info;
    BOOST_REQUIRE(engine->Start(*machine, 0));
    BOOST_CHECK(info.wait(&*engine, 1000));
    BOOST_CHECK(info.bestmove.empty());
    BOOST_CHECK(info.bestscore == 0);
  }
}

BOOST_AUTO_TEST_CASE( GumpSteinitzTest )
{
  /*
    Gump - Steinitz Vienna, 1859 Vienna Game
  */
  boost::shared_ptr<IEngine> engine(CreateGreedyEngine(), DeleteGreedyEngine);
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("\
1.e4 e5 2.Nc3 Nf6 3.f4 d5 4.exd5 Nxd5 5.fxe5 Nxc3 6.bxc3 Qh4+ 7.Ke2 Bg4+ 8.Nf3 Nc6 \
9.d4 O-O-O 10.Bd2 Bxf3+ 11.gxf3 Nxe5 12.dxe5 Bc5 13.Qe1 Qc4+ 14.Kd1 Qxc3 \
15.Rb1 Qxf3+ 16.Qe2 Rxd2+ 17.Kxd2 Rd8+ 18.Kc1 Ba3+ 19.Rb2 Qc3 20.Bh3+ Kb8 \
21.Qb5 Qd2+ 22.Kb1 Qd1+ 23.Rxd1 Rxd1# *\
");
  BOOST_REQUIRE(moves.size() == 46);

  const std::vector< std::pair<std::string, int> > scores0 = {
    { "e4", 0 },{ "e5", -26 },{ "Nc3", 0 },{ "Nf6", -22 },{ "f4", 4 },{ "d5", -12 },{ "exd5", -12 },{ "Nxd5", -1011 },{ "fxe5", -26 },{ "Nxc3", -1003 },
    { "bxc3", -2023 },{ "Qh4", -1007 },{ "Ke2", 965 },{ "Bg4", -980 },{ "Nf3", 963 },{ "Nc6", -1003 },{ "d4", 970 },{ "O-O-O", -1000 },{ "Bd2", 990 },
    { "Bxf3", -999 },{ "gxf3", -2056 },{ "Nxe5", -991 },{ "dxe5", -47 },{ "Bc5", -2997 },{ "Qe1", 2977 },{ "Qc4", -2983 },{ "Kd1", 2956 },{ "Qxc3", -2984 },
    { "Rb1", 1981 },{ "Qxf3", -1993 },{ "Qe2", 951 },{ "Rxd2", -992 },{ "Kxd2", -2064 },{ "Rd8", -2977 },{ "Kc1", 2938 },{ "Ba3", -2976 },{ "Rb2", 2956 },
    { "Qc3", -2985 },{ "Bh3", 2976 },{ "Kb8", -3035 },{ "Qb5", 2976 },{ "Qd2", -2993 },{ "Kb1", 2958 },{ "Qd1", -2998 },{ "Rxd1", 2964 },{ "Rxd1", -12053 }
  };

  size_t nm = 0;
  for (auto m : moves) {
    if (nm < scores0.size()) {
      auto s0 = scores0[nm];
      infotest info;
      assert(s0.first == m);
      BOOST_REQUIRE(s0.first == m);
      BOOST_REQUIRE_MESSAGE(engine->Start(*machine, 0), "Can't evaluate position at move '" + m + "'");
      BOOST_CHECK_MESSAGE(info.wait(&*engine, 1000), "Evaluation timeout at move '" + m + "'");
      BOOST_CHECK_MESSAGE(info.bestmove.empty(), "The evaluated best move (" + info.bestmove + ") do not match at move '" + m + "'");
      BOOST_CHECK_MESSAGE(info.bestscore == s0.second, "The evaluated best score (" + std::to_string(info.bestscore) + ") do not match at move '" + m + "'");
    }
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    BOOST_CHECK_MESSAGE(machine->LastMoveNotation() == m, "Can't take move " + m);
    ++nm;
  }
  {
    //infotest info;
    //BOOST_REQUIRE(engine->Start(*machine, 0));
    //BOOST_CHECK(info.wait(&*engine, 1000));
    //BOOST_CHECK(info.bestmove.empty());
    //BOOST_CHECK(info.bestscore == 0);
  }

}

BOOST_AUTO_TEST_SUITE_END()