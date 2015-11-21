#include "stdafx.h"

using namespace Chai::Chess;

CHESSBOARD;

class infotest : private IInfoCall {
public:
  infotest() : readyok(false), deadline(false), bestscore(0) {}

  std::string bestmove;
  float bestscore;

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
  void BestScore(float score) override { bestscore = score; }

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
  BOOST_CHECK(engine->EvalPosition(*machine) == 0);
}

BOOST_AUTO_TEST_CASE( StartTest )
{
  boost::shared_ptr<IEngine> engine(CreateGreedyEngine(), DeleteGreedyEngine);
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  BOOST_CHECK(engine->EvalPosition(*machine) == 0);
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

  static const float wpawn = 1000.0f;
  const std::vector< std::pair<std::string, float> > scores0 = {
    { "e4", 0 / wpawn },{ "e5", -26 / wpawn },{ "Nc3", 0 / wpawn },{ "Nf6", -22 / wpawn },{ "f4", 4 / wpawn },{ "d5", -12 / wpawn },{ "exd5", -12 / wpawn },
    { "Nxd5", -1011 / wpawn },{ "fxe5", -26 / wpawn },{ "Nxc3", -1003 / wpawn },{ "bxc3", -2023 / wpawn },{ "Qh4", -1007 / wpawn },{ "Ke2", 965 / wpawn },
    { "Bg4", -980 / wpawn },{ "Nf3", 963 / wpawn },{ "Nc6", -1003 / wpawn },{ "d4", 970 / wpawn },{ "O-O-O", -1000 / wpawn },{ "Bd2", 990 / wpawn },
    { "Bxf3", -999 / wpawn },{ "gxf3", -2056 / wpawn },{ "Nxe5", -991 / wpawn },{ "dxe5", -47 / wpawn },{ "Bc5", -2997 / wpawn },{ "Qe1", 2977 / wpawn },
    { "Qc4", -2983 / wpawn },{ "Kd1", 2956 / wpawn },{ "Qxc3", -2984 / wpawn },{ "Rb1", 1981 / wpawn },{ "Qxf3", -1993 / wpawn },{ "Qe2", 951 / wpawn },
    { "Rxd2", -992 / wpawn },{ "Kxd2", -2064 / wpawn },{ "Rd8", -2977 / wpawn },{ "Kc1", 2938 / wpawn },{ "Ba3", -2976 / wpawn },{ "Rb2", 2956 / wpawn },
    { "Qc3", -2985 / wpawn },{ "Bh3", 2976 / wpawn },{ "Kb8", -3035 / wpawn },{ "Qb5", 2976 / wpawn },{ "Qd2", -2993 / wpawn },{ "Kb1", 2958 / wpawn },
    { "Qd1", -2998 / wpawn },{ "Rxd1", 2964 / wpawn },{ "Rxd1", -12053 / wpawn }
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
      BOOST_TEST_MESSAGE("The evaluated best score is {" + std::to_string(info.bestscore) + "} at move '" + m + "'");
      BOOST_CHECK_SMALL(info.bestscore - s0.second, 0.001f);
      BOOST_CHECK_SMALL(engine->EvalPosition(*machine) - (machine->CurrentPlayer() == Set::white ? s0.second : - s0.second), 0.001f);
    }
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    BOOST_CHECK_MESSAGE(machine->LastMoveNotation() == m, "Can't take move " + m);
    ++nm;
  }
  {
    infotest info;
    BOOST_REQUIRE(machine->CheckStatus() == Status::checkmate);
    BOOST_REQUIRE(engine->Start(*machine, 0));
    BOOST_CHECK(info.wait(&*engine, 1000));
    BOOST_CHECK(info.bestmove.empty());
    BOOST_CHECK(info.bestscore == - std::numeric_limits<float>::infinity());
  }

}

BOOST_AUTO_TEST_SUITE_END()