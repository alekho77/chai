#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>

#include <ChessEngineGreedy/engine.h>
#include <chessmachine/machine.h>
#include <Common/machinetestutils.h>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/make_shared.hpp>

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
    if (!readyok) {
      engine->Stop();
      return false;
    }
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

struct scoped_counter {
  explicit scoped_counter(int& c) : count(c) { ++count; }
  ~scoped_counter() { --count; }
  int& count;
};

int global_count = 0;

/* Classic NegaMax searching */
std::pair<float, std::string> TestSearch(IMachine& machine, int depth, size_t& nodes, float alpha = -inff, const float betta = inff) {
  scoped_counter counter(global_count);
  if (counter.count > 100) DebugBreak();
  
  Status status = machine.CheckStatus();
  if (depth > 0 && status != Status::checkmate && status != Status::stalemate) {
    boost::optional< std::pair<float, std::string> > bestmove;
    boost::container::flat_set<Position> xpos;
    for (const auto& xp : machine.GetSet(machine.CurrentPlayer() == Set::white ? Set::black : Set::white)) {
      xpos.insert(xp.position);
    }
    for (const auto& p : machine.GetSet(machine.CurrentPlayer())) {
      for (const auto& m : machine.EnumMoves(p.position)) {
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
            bool forcing = (depth == 1 && (/*status == Status::check ||*/ machine.CheckStatus() == Status::check || xpos.find(mm.to) != xpos.end())); // TODO: en passant
            float score = -TestSearch(machine, forcing ? depth : depth - 1, nodes, -betta, -alpha).first;
            if (!bestmove || score > bestmove->first) {
              bestmove = { score, machine.LastMoveNotation() };
              if (score > alpha) {
                alpha = score;
              }
              if (score >= betta) {
                machine.Undo();
                return *bestmove;
              }
            }
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
  ++nodes;
  boost::shared_ptr<IEngine> engine = boost::make_shared<GreedyEngine>();
  return std::make_pair(engine->EvalPosition(machine), std::string());
}

BOOST_AUTO_TEST_SUITE( GreedyEngineTest )

BOOST_AUTO_TEST_CASE( ConstructorTest )
{
  boost::shared_ptr<IEngine> engine = boost::make_shared<GreedyEngine>();
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");

  BOOST_REQUIRE(!engine->Start(*machine, 0));
  BOOST_CHECK(engine->EvalPosition(*machine) == 0);
}

BOOST_AUTO_TEST_CASE( StartTest )
{
  boost::shared_ptr<IEngine> engine = boost::make_shared<GreedyEngine>();
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
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
  boost::shared_ptr<IEngine> engine = boost::make_shared<GreedyEngine>();
  BOOST_REQUIRE_MESSAGE(engine, "Can't create ChessEngine!");

  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

#ifdef _DEBUG
  const int max_depth_testing = 1;
#else
  const int max_depth_testing = 2; // 4
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
      { "e4", 0.026f, 20 },{ "e5", -0.000f, 20 },{ "d4", 0.024f, 29 },{ "d5", 0.002f, 29 },{ "d4", 0.027f, 31 },{ "exf4", 0.977f, 95 },{ "exd5", 0.020f, 1107 },
      { "exf4", -0.020f, 411 },{ "fxe5", 0.981f, 410 },{ "Nc6", -0.981f, 210 },{ "dxc3", 0.990f, 186 },{ "Qd3", -0.980f, 66 },{ "g3", 0.997f, 2 },{ "Nc6", -0.955f, 120 },
      { "Nf3", 1.003f, 3 },{ "Bxf3", -0.964f, 343 },{ "d4", 1.000f, 15 },{ "Bxf3", -0.978f, 1397 },{ "Qd3", 1.010f, 22 },{ "Bxf3", -0.976f, 2538 },{ "gxf3", 0.976f, 1546 },
      { "Bc5", -0.976f, 516 },{ "dxe5", 2.977f, 254 },{ "Bc5", -2.977f, 233 },{ "Bf4", 2.997f, 20 },{ "Kb8", -2.974f, 302 },{ "Kd1", 2.984f, 1 },{ "Qxc3", -2.008f, 313 },
      { "Qe4", 2.008f, 24 },{ "Qxf3", -0.994f, 682 },{ "Kc1", 0.994f, 3 },{ "Qxh1", 3.972f, 891 },{ "Kxd2", -1.988f, 1051 },{ "Qxh1", 1.988f, 1049 },{ "Qd3", 2.987f, 3 },
      { "Qxh1", 1.991f, 454 },{ "Rb2", 2.984f, 1 },{ "Bxb2", 2.023f, 325 },{ "Qd3", 3.002f, 43 },{ "Qxh3", 0.004f, 26 },{ "Qd3", 3.001f, 28 },{ "Bxb2", 1.995f, 724 },
      { "Kb1", 2.998f, 1 },{ "Qd1", inff, 288 },{ "Rxd1", -inff, 126 },{ "Rxd1", inff, 126 }
    },
    {
      { "e4", 0.000f, 178 },{ "e5", -0.024f, 283 },{ "d4", 0.000f, 356 },{ "Nc6", -0.023f, 421 },{ "d4", 0.003f, 847 },{ "exf4", 0.977f, 4436 },{ "exd5", 0.020f, 8752 },
      { "exf4", -0.020f, 2028 },{ "fxe5", 0.981f, 4408 },{ "Qh4", -0.989f, 1295 },{ "dxc3", 0.990f, 1327 },{ "Qh4", -0.997f, 309 },{ "g3", 0.973f, 246 },
      { "Nc6", -0.983f, 354 },{ "Nf3", 0.964f, 607 },{ "Bxf3", -0.964f, 435 },{ "d4", 0.978f, 10152 },{ "Bxf3", -0.978f, 3423 },{ "Bf4", 0.982f, 14877 },
      { "Bxf3", -0.976f, 4985 },{ "gxf3", 0.976f, 6159 },{ "Ba3", -0.997f, 1430 },{ "dxe5", 2.977f, 5294 },{ "Bc5", -2.997f, 677 },{ "Qb1", 2.976f, 2043 },
      { "Qc4", -2.984f, 3741 },{ "Kd1", 2.008f, 313 },{ "Qxc3", -2.008f, 786 },{ "Bh3", 1.973f, 1265 },{ "Qxf3", -0.994f, 3699 },{ "Be2", 0.970f, 3827 },
      { "Qxh1", 3.972f, 8324 },{ "Kxd2", -1.988f, 2597 },{ "Qxh1", 1.988f, 4677 },{ "Qd3", -1.982f, 3216 },{ "Qxh1", 1.991f, 2087 },{ "Rb2", -2.023f, 325 },
      { "Bxb2", 2.023f, 375 },{ "Qg4", 2.981f, 638 },{ "Qxh3", 0.004f, 26 },{ "Qb5", -1.995f, 805 },{ "Bxb2", 1.995f, 770 },{ "Kb1", -inff, 288 },{ "Qd1", inff, 288 },
      { "Rxd1", -inff, 126 },{ "Rxd1", inff, 602 }
    },
    {
      { "d4", 0.026f, 2241 },{ "d5", 0.001f, 3386 },{ "Nc3", 0.023f, 6540 },{ "Nc6", 0.002f, 11086 },{ "Nf3", 0.023f, 68381 },{ "exf4", 1.004f, 44761 },
      { "exd5", 0.020f, 1183603 },{ "exf4", 0.008f, 194934 },{ "fxe5", 0.989f, 188373 },{ "Qh4", -0.965f, 58603 },{ "bxc3", 0.997f, 19465 },{ "Qh4", -0.973f, 23983 },
      { "g3", 0.999f, 4946 },{ "Qe4", 0.009f, 43574 },{ "Nf3", 0.964f, 1998 },{ "Bxf3", -0.964f, 124284 },{ "d4", 0.978f, 88336 },{ "Bxf3", -0.978f, 1089065 },
      { "Bf4", 0.982f, 103847 },{ "Bxf3", -0.976f, 839939 },{ "Kxf3", 0.999f, 36971 },{ "Kb8", -0.984f, 255949 },{ "dxe5", 2.997f, 97342 },{ "Qh5", -1.999f, 50131 },
      { "Qb1", 2.987f, 25624 },{ "Qc4", -2.008f, 102233 },{ "Kd1", 2.008f, 786 },{ "Qxc3", -1.973f, 60725 },{ "Bh3", 2.003f, 8496 },{ "Qxf3", -0.970f, 93016 },
      { "Be2", 1.010f, 35149 },{ "Qxh1", 4.002f, 308154 },{ "Kxd2", -1.988f, 152586 },{ "Qxh1", 2.002f, 206914 },{ "Qd3", -1.982f, 10981 },{ "Ba3", 2.023f, 42723 },
      { "Rb2", -2.023f, 375 },{ "Bxb2", 3.961f, 12624 },{ "Qg4", 3.012f, 843 },{ "Qxh3", 2.001f, 2822 },{ "Qb5", -1.995f, 1733 },{ "Qd2", inff, 12169 },
      { "Kb1", -inff, 288 },{ "Qd1", inff, 18796 },{ "Rxd1", -inff, 602 },{ "Rxd1", inff, 17616 }
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
        bestmove = TestSearch(*machine, depth, searched_nodes);
      }
      //char str[32];
      //sprintf_s(str, "%.3f", bestmove.first);
      //BOOST_TEST_MESSAGE("{ \"" + bestmove.second + "\", " + std::string(str) + "f, " + std::to_string(searched_nodes) + " }");
      infotest info2;
      BOOST_REQUIRE_MESSAGE(engine->Start(*machine, depth), "Can't start search " + std::to_string(depth) + " moves in depth at '" + m + "' move");
      BOOST_REQUIRE_MESSAGE(info2.wait(&*engine, 120000), "Searching timeout at move '" + m + "' with " + std::to_string(depth) + " moves in depth");
      if (std::abs(bestmove.first) == inff) {
        BOOST_CHECK(info2.bestscore == bestmove.first);
      } else {
        BOOST_CHECK_SMALL(info2.bestscore - bestmove.first, 0.001f);
      }
      BOOST_CHECK(info2.bestmove == bestmove.second);
      BOOST_CHECK(info2.nodes == searched_nodes);
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