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
  void BestMove(std::string notation) override { bestmove = notation; }
  void BestScore(float score) override { bestscore = score; }

  void on_timeout(const boost::system::error_code& e) {
    if (e != boost::asio::error::operation_aborted) {
      deadline = true;
    }
  }

  volatile bool readyok;
  volatile bool deadline;
};

/* Classic NegaMax searching */
std::pair<float, std::string> TestSearch(IMachine& machine, int depth) {
  if (depth > 0) {
    if (machine.CheckStatus() == Status::checkmate) {
      return std::make_pair(-std::numeric_limits<float>::infinity(), std::string());
    }
    if (machine.CheckStatus() == Status::stalemate) {
      return std::make_pair(0.0f, std::string());
    }
    boost::container::static_vector< std::pair<float, std::string>, 100 > moves;
    for (const auto& p : machine.GetSet(machine.CurrentPlayer())) {
      for (const auto& m : machine.CheckMoves(p.position)) {
        if (p.type == Type::pawn && (m.rank() == '1' || m.rank() == '8')) {
          for (auto pp : { Type::knight, Type::bishop, Type::rook, Type::queen }) {
            if (machine.Move(p.type, p.position, m, pp)) {
              moves.push_back(std::make_pair(-TestSearch(machine, depth - 1).first, std::string(machine.LastMoveNotation())));
              machine.Undo();
            } else {
              assert(!"Can't make move!");
            }
          }
        } else if (machine.Move(p.type, p.position, m)) {
          moves.push_back(std::make_pair(-TestSearch(machine, depth - 1).first, std::string(machine.LastMoveNotation())));
          machine.Undo();
        } else {
          assert(!"Can't make move!");
        }
      }
    }
    assert(!moves.empty());
    return *std::max_element(moves.begin(), moves.end(), [](auto a, auto b) { return a.first < b.first; });
  }
  boost::shared_ptr<IEngine> engine = CreateGreedyEngine();
  return std::make_pair(engine->EvalPosition(machine), std::string());
}


BOOST_AUTO_TEST_SUITE( GreedyEngineTest )

BOOST_AUTO_TEST_CASE( ConstructorTest )
{
  boost::shared_ptr<IEngine> engine = CreateGreedyEngine();
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine = CreateChessMachine();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");

  BOOST_REQUIRE(!engine->Start(*machine, 0));
  BOOST_CHECK(engine->EvalPosition(*machine) == 0);
}

BOOST_AUTO_TEST_CASE( StartTest )
{
  boost::shared_ptr<IEngine> engine = CreateGreedyEngine();
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine = CreateChessMachine();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  BOOST_CHECK_SMALL(engine->EvalPosition(*machine), 0.001f);
  {
    infotest info;
    BOOST_REQUIRE(engine->Start(*machine, 0));
    BOOST_CHECK(info.wait(&*engine, 1000));
    BOOST_CHECK(info.bestmove.empty());
    BOOST_CHECK_SMALL(info.bestscore, 0.001f);
  }
}

BOOST_AUTO_TEST_CASE( GumpSteinitzTest )
{
  /*
    Gump - Steinitz Vienna, 1859 Vienna Game
  */
  boost::shared_ptr<IEngine> engine = CreateGreedyEngine();
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine = CreateChessMachine();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

#ifdef _DEBUG
  const int max_depth_testing = 1;
#else
  const int max_depth_testing = 2;
#endif
  const std::vector<std::string> moves = split("\
1.e4 e5 2.Nc3 Nf6 3.f4 d5 4.exd5 Nxd5 5.fxe5 Nxc3 6.bxc3 Qh4+ 7.Ke2 Bg4+ 8.Nf3 Nc6 \
9.d4 O-O-O 10.Bd2 Bxf3+ 11.gxf3 Nxe5 12.dxe5 Bc5 13.Qe1 Qc4+ 14.Kd1 Qxc3 \
15.Rb1 Qxf3+ 16.Qe2 Rxd2+ 17.Kxd2 Rd8+ 18.Kc1 Ba3+ 19.Rb2 Qc3 20.Bh3+ Kb8 \
21.Qb5 Qd2+ 22.Kb1 Qd1+ 23.Rxd1 Rxd1# *\
");
  BOOST_REQUIRE(moves.size() == 46);

  const std::vector< std::pair<std::string, float> > scores0 = {
    { "e4", 0.000f },{ "e5", -0.026f },{ "Nc3", 0.000f },{ "Nf6", -0.022f },{ "f4", 0.004f },{ "d5", -0.012f },{ "exd5", -0.012f },{ "Nxd5", -1.011f },
    { "fxe5", -0.026f },{ "Nxc3", -1.003f },{ "bxc3", -2.023f },{ "Qh4", -1.007f },{ "Ke2", 0.965f },{ "Bg4", -0.980f },{ "Nf3", 0.963f },{ "Nc6", -1.003f },
    { "d4", 0.970f },{ "O-O-O", -1.000f },{ "Bd2", 0.990f },{ "Bxf3", -0.999f },{ "gxf3", -2.056f },{ "Nxe5", -0.991f },{ "dxe5", -0.047f },
    { "Bc5", -2.997f },{ "Qe1", 2.977f },{ "Qc4", -2.983f },{ "Kd1", 2.956f },{ "Qxc3", -2.984f },{ "Rb1", 1.981f },{ "Qxf3", -1.993f },{ "Qe2", 0.951f },
    { "Rxd2", -0.992f },{ "Kxd2", -2.064f },{ "Rd8", -2.977f },{ "Kc1", 2.938f },{ "Ba3", -2.976f },{ "Rb2", 2.956f },{ "Qc3", -2.985f },
    { "Bh3", 2.976f },{ "Kb8", -3.035f },{ "Qb5", 2.976f },{ "Qd2", -2.993f },{ "Kb1", 2.958f },{ "Qd1", -2.998f },{ "Rxd1", 2.964f },{ "Rxd1", -12.053f }
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
      BOOST_CHECK_SMALL(engine->EvalPosition(*machine) - s0.second, 0.001f);
    }
    for (int depth = 1; depth <= max_depth_testing; depth++)
    {
      BOOST_TEST_MESSAGE("Search at '" + m + "'(" + std::to_string(nm / 2 + 1) + ") with " + std::to_string(depth) + " moves in depth");
      auto bestmove = TestSearch(*machine, depth);
      infotest info2;
      BOOST_REQUIRE_MESSAGE(engine->Start(*machine, depth), "Can't start search " + std::to_string(depth) + " moves in depth at '" + m + "' move");
      BOOST_REQUIRE_MESSAGE(info2.wait(&*engine, 90000), "Searching timeout at move '" + m + "' with " + std::to_string(depth) + " moves in depth");
      if (std::abs(bestmove.first) == std::numeric_limits<float>::infinity()) {
        BOOST_CHECK(info2.bestscore == bestmove.first);
      } else {
        BOOST_CHECK_SMALL(info2.bestscore - bestmove.first, 0.001f);
      }
      BOOST_CHECK(info2.bestmove == bestmove.second);
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
  //for (int depth = 1; depth <= max_depth_testing; depth++)
  //{
  //  auto bestmove = TestSearch(*machine, depth);
  //  infotest info;
  //  BOOST_REQUIRE(engine->Start(*machine, depth));
  //  BOOST_CHECK(info.wait(&*engine, 1000));
  //  BOOST_CHECK_SMALL(info.bestscore - bestmove.first, 0.001f);
  //  BOOST_CHECK(info.bestmove == bestmove.second);
  //}
}

BOOST_AUTO_TEST_SUITE_END()