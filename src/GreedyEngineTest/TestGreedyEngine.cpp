#include "stdafx.h"

using namespace Chai::Chess;

CHESSBOARD;

static const float inff = std::numeric_limits<float>::infinity();

class infotest : private IInfoCall {
public:
  infotest() : readyok(false), deadline(false), bestscore(0), nodes(0) {}

  std::string bestmove;
  float bestscore;
  size_t nodes;

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
  void NodesSearched(size_t n) override { nodes = n; }
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

struct Move {
  Piece piece;
  Position to;
  Type promotion;
};

/* Classic NegaMax searching */
std::pair<float, std::string> TestSearch(IMachine& machine, int depth, size_t* nodes = nullptr, const float alpha = inff, float betta = -inff) {
  if (depth > 0) {
    if (machine.CheckStatus() == Status::checkmate) {
      return std::make_pair(-std::numeric_limits<float>::infinity(), std::string());
    }
    if (machine.CheckStatus() == Status::stalemate) {
      return std::make_pair(0.0f, std::string());
    }
    boost::optional< std::pair<float, std::string> > bestmove;
    for (const auto& p : machine.GetSet(machine.CurrentPlayer())) {
      for (const auto& m : machine.CheckMoves(p.position)) {
        boost::container::small_vector<Move, 4> moves;
        if (p.type == Type::pawn && (m.rank() == '1' || m.rank() == '8')) {
          for (auto pp : { Type::knight, Type::bishop, Type::rook, Type::queen }) {
            moves.push_back({ p, m, pp });
          }
        } else {
          moves.push_back({ p, m, Type::bad });
        }
        for (const auto& mm : moves) {
          if (machine.Move(mm.piece.type, mm.piece.position, mm.to, mm.promotion)) {
            float score = -TestSearch(machine, depth - 1, nodes).first;
            if (!bestmove || score > bestmove->first) {
              bestmove = { score, machine.LastMoveNotation() };
              //if (score >= alpha) {
              //  machine.Undo();
              //  return *bestmove;
              //}
            }
            //if (-score > betta) {
            //  betta = -score;
            //}
            machine.Undo();
          }
          else {
            assert(!"Can't make move!");
          }
        }
      }
    }
    assert(bestmove);
    return *bestmove;
  }
  if (nodes) {
    ++(*nodes);
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
  const int max_depth_testing = 3;
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

  typedef std::vector< boost::tuple<std::string, float, size_t> > scores_t;
  const std::vector<scores_t> scoresn = {
   {
    { "e4", 0.026f, 20 },{ "e5", 0.000f, 20 },{ "d4", 0.024f, 29 },{ "d5", 0.002f, 29 },{ "d4", 0.027f, 31 },{ "Nxe4", 1.020f, 28 },{ "fxe5", 1.017f, 33 },
    { "Qxd5", 0.034f, 38 },{ "Nxd5", 3.024f, 33 },{ "Nxc3", 2.023f, 41 },{ "dxc3", 1.022f, 27 },{ "Qxd2", 0.038f, 38 },{ "g3", 0.997f, 2 },{ "Qxh2", 0.019f, 47 },
    { "Nf3", 1.003f, 3 },{ "Bxf3", 2.050f, 42 },{ "d4", 1.000f, 15 },{ "Bxf3", 2.055f, 48 },{ "Qd3", 1.010f, 22 },{ "Bxf3", 2.056f, 45 },{ "gxf3", 0.991f, 4 },
    { "Nxd4", 0.057f, 43 },{ "dxe5", 2.997f, 22 },{ "Rxd2", 0.046f, 44 },{ "Bh3", 3.037f, 20 },{ "Qxe1", 6.056f, 52 },{ "Kd1", 2.984f, 1 },{ "Rxd2", 0.063f, 52 },
    { "Bh3", 2.034f, 24 },{ "Qxd2", 1.066f, 49 },{ "Kc1", 0.994f, 3 },{ "Qxe2", 8.064f, 55 },{ "Kxd2", 2.976f, 3 },{ "Qxe2", 6.077f, 48 },{ "Qd3", 2.987f, 3 },
    { "Qxe2", 6.060f, 54 },{ "Rb2", 2.984f, 1 },{ "Qxe2", 6.058f, 50 },{ "Qg4", 3.044f, 21 },{ "Qxh3", 0.029f, 4 },{ "Qd3", 3.001f, 28 },{ "Bxb2", 2.044f, 47 },
    { "Kb1", 2.998f, 1 },{ "Bxb2", 2.011f, 48 },{ "Rxd1", 12.054f, 1 },{ "Rxd1", inff, 30 }
  },
  {
    { "e4", 0.000f, 400 },{ "e5", -0.024f, 600 },{ "Nc3", -0.002f, 835 },{ "Nc6", -0.023f, 895 },{ "d4", -1.005f, 830 },{ "exf4", 0.977f, 891 },{ "fxe5", -0.016f, 1181 },
    { "exf4", -0.039f, 1241 },{ "Nxd5", -0.035f, 1359 },{ "Qh4", -0.989f, 1272 },{ "bxc3", -0.038f, 1172 },{ "Qh4", -0.997f, 967 },{ "Ke2", -0.019f, 92 },
    { "Nc6", -0.983f, 758 },{ "Nf3", -2.050f, 130 },{ "Bxf3", -0.992f, 644 },{ "d4", -2.055f, 714 },{ "Bxf3", -0.992f, 1031 },{ "Kd2", -2.041f, 981 },{ "Bxf3", -0.991f, 910 },
    { "gxf3", -0.057f, 186 },{ "Kb8", -1.000f, 908 },{ "dxe5", -0.046f, 964 },{ "Qc4", -3.005f, 846 },{ "Bh3", -0.031f, 974 },{ "Qc4", -2.984f, 1140 },{ "Kd1", -0.063f, 52 },
    { "Qxc3", -2.034f, 1133 },{ "Bh3", 1.973f, 1121 },{ "Qxf3", -0.994f, 1372 },{ "Kc1", -3.991f, 166 },{ "Qxh1", 2.987f, 1309 },{ "Kxd2", -6.077f, 162 },
    { "Qxh1", 1.004f, 1366 },{ "Kc1", -6.060f, 157 },{ "Qxh1", 1.001f, 1441 },{ "Rb2", -6.058f, 50 },{ "Qxh1", 1.950f, 1021 },{ "Qg4", 2.981f, 896 },{ "Qxh3", -0.031f, 104 },
    { "Qb5", -2.044f, 1325 },{ "Bxb2", -0.997f, 1202 },{ "Kb1", -2.011f, 48 },{ "Bxb2", -1.020f, 1510 },{ "Rxd1", -inff, 30 },{ "Rxd1", inff, 1197 }
  },
  {
    { "d4", 0.026f, 8902 },{ "d5", 0.010f, 13160 },{ "Qh5", 1.040f, 24825 },{ "Qh4", 1.020f, 26521 },{ "Qf3", 1.080f, 26037 },{ "Bb4", 1.036f, 26479 },{ "fxe5", 2.056f, 38683 },
    { "Nxd5", 0.035f, 46513 },{ "Bb5", 3.027f, 43800 },{ "Qh4", 2.034f, 51053 },{ "dxc3", 1.014f, 32484 },{ "Qd5", 0.033f, 36059 },{ "g3", 0.999f, 1921 },
    { "Bg4", 2.050f, 33682 },{ "Nf3", 0.992f, 2499 },{ "Nc6", 2.055f, 26994 },{ "d4", 0.992f, 12993 },{ "Be7", 2.042f, 47495 },{ "Qd3", 1.005f, 22713 },{ "Ba3", 2.050f, 40703 },
    { "gxf3", 1.000f, 4152 },{ "Rd5", 0.051f, 37347 },{ "dxe5", 3.005f, 21491 },{ "Qc4", 0.035f, 34511 },{ "Be1", 2.995f, 20973 },{ "Qc4", 0.063f, 52804 },
    { "Kd1", 2.034f, 1133 },{ "Qe6", 0.030f, 52961 },{ "Bh3", 2.003f, 28384 },{ "Rxd2", 5.080f, 62289 },{ "Be2", 1.999f, 4643 },{ "Rxd2", 6.077f, 63008 },{ "Kxd2", -1.004f, 4505 },
    { "Rd8", 6.060f, 58788 },{ "Kc1", -1.001f, 4071 },{ "Ba3", 6.058f, 68511 },{ "Rb2", -1.950f, 1021 },{ "Bxb2", 8.032f, 45163 },{ "Qg4", 3.993f, 21416 },{ "Qxh3", 5.028f, 4400 },
    { "Qb5", 0.997f, 35003 },{ "Bxb2", 5.023f, 51141 },{ "Kb1", 1.020f, 1510 },{ "Qd1", inff, 65356 },{ "Rxd1", -inff, 1197 },{ "Rxd1", inff, 31483 }
  }
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
    //int depth = 3;
    for (int depth = 1; depth <= max_depth_testing; depth++)
    {
      BOOST_TEST_MESSAGE("Search at '" + m + "'(" + std::to_string(nm / 2 + 1) + ") with " + std::to_string(depth) + " moves in depth");
      size_t searched_nodes;
      std::pair<float, std::string> bestmove;
      if (scoresn.size() >= depth) {
        searched_nodes = scoresn[depth - 1][nm].get<2>();
        bestmove = std::make_pair(scoresn[depth - 1][nm].get<1>(), scoresn[depth - 1][nm].get<0>());
      } else {
        searched_nodes = 0;
        bestmove = TestSearch(*machine, depth, &searched_nodes);
      }
      //char str[32];
      //sprintf_s(str, "%.3f", bestmove.first);
      //BOOST_TEST_MESSAGE("{ \"" + bestmove.second + "\", " + std::string(str) + "f, " + std::to_string(searched_nodes) + " }");
      infotest info2;
      BOOST_REQUIRE_MESSAGE(engine->Start(*machine, depth), "Can't start search " + std::to_string(depth) + " moves in depth at '" + m + "' move");
      BOOST_REQUIRE_MESSAGE(info2.wait(&*engine, 30000), "Searching timeout at move '" + m + "' with " + std::to_string(depth) + " moves in depth");
      if (std::abs(bestmove.first) == inff) {
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
    BOOST_CHECK(info.bestscore == -inff);
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