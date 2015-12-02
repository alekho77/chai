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

struct Move {
  Piece piece;
  Position to;
  Type promotion;
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
  const int max_depth_testing = 3; // 4
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
      { "e4", 0.026f, 20 },{ "e5", -0.000f, 20 },{ "d4", 0.024f, 29 },{ "d5", 0.002f, 29 },{ "d4", 0.027f, 31 },{ "exf4", 0.977f, 112 },{ "exd5", 0.020f, 1703 },
      { "exf4", -0.020f, 631 },{ "fxe5", 0.981f, 700 },{ "Nc6", -0.981f, 250 },{ "dxc3", 0.990f, 209 },{ "Qd3", -0.980f, 66 },{ "g3", 0.997f, 2 },{ "Nc6", -0.955f, 179 },
      { "Nf3", 1.003f, 3 },{ "Bxf3", -0.964f, 400 },{ "d4", 1.000f, 15 },{ "Bxf3", -0.978f, 2291 },{ "Qd3", 1.010f, 22 },{ "Bxf3", -0.976f, 4278 },{ "gxf3", 0.976f, 3163 },
      { "Bc5", -0.976f, 2402 },{ "dxe5", 2.977f, 698 },{ "Bc5", -2.977f, 316 },{ "Be3", 2.997f, 41 },{ "Kb8", -2.974f, 389 },{ "Kd1", 2.984f, 1 },{ "Qxc3", -2.008f, 530 },
      { "Qe4", 2.008f, 24 },{ "Qxf3", -0.994f, 361 },{ "Kc1", 0.994f, 3 },{ "Qxh1", 3.972f, 492 },{ "Kxd2", -1.988f, 808 },{ "Qxh1", 1.988f, 806 },{ "Qd3", 2.987f, 3 },
      { "Qxh1", 1.991f, 907 },{ "Rb2", 2.984f, 1 },{ "Bxb2", 2.023f, 270 },{ "Qd3", 3.002f, 52 },{ "Qxh3", 0.004f, 35 },{ "Qd3", 3.001f, 28 },{ "Bxb2", 1.995f, 515 },
      { "Kb1", 2.998f, 1 },{ "Qd1", inff, 225 },{ "Rxd1", -inff, 98 },{ "Rxd1", inff, 98 }
    },
    {
      { "e4", 0.000f, 165 },{ "e5", -0.024f, 275 },{ "d4", 0.000f, 335 },{ "Nc6", -0.023f, 428 },{ "d4", 0.003f, 4673 },{ "exf4", 0.977f, 112 },{ "exd5", 0.020f, 44676 },
      { "exf4", -0.020f, 689 },{ "fxe5", 0.981f, 14596 },{ "Qh4", -0.989f, 255 },{ "dxc3", 0.990f, 2809 },{ "Qh4", -0.997f, 351 },{ "g3", 0.973f, 305 },
      { "Nc6", -0.983f, 416 },{ "Nf3", 0.964f, 436 },{ "Bxf3", -0.964f, 400 },{ "d4", 0.978f, 18358 },{ "Bxf3", -0.978f, 2291 },{ "Bf4", 0.982f, 84959 },
      { "Bxf3", -0.976f, 4278 },{ "gxf3", 0.976f, 7398 },{ "Ba3", -0.997f, 4948 },{ "dxe5", 2.977f, 30359 },{ "Bc5", -2.997f, 974 },{ "Qb1", 2.976f, 5995 },
      { "Qc4", -2.984f, 501 },{ "Kd1", 2.008f, 530 },{ "Qxc3", -2.008f, 583 },{ "Bh3", 1.973f, 793 },{ "Qxf3", -0.994f, 1343 },{ "Be2", 0.970f, 2487 },
      { "Qxh1", 3.972f, 492 },{ "Kxd2", -1.988f, 9134 },{ "Qxh1", 1.988f, 806 },{ "Qd3", -1.982f, 2939 },{ "Qxh1", 1.991f, 907 },{ "Rb2", -2.023f, 270 },
      { "Bxb2", 2.023f, 270 },{ "Qg4", 2.981f, 182 },{ "Qxh3", 0.004f, 35 },{ "Qb5", -1.995f, 1558 },{ "Bxb2", 1.995f, 515 },{ "Kb1", -inff, 225 },{ "Qd1", inff, 225 },
      { "Rxd1", -inff, 98 },{ "Rxd1", inff, 599 }
    },
    {
      { "d4", 0.026f, 2281 },{ "d5", 0.001f, 2729 },{ "Nc3", 0.023f, 8395 },{ "Nc6", 0.002f, 10334 },{ "Nf3", 0.023f, 36267 },{ "exf4", 1.004f, 14701 },
      { "exd5", 0.020f, 433859 },{ "exf4", 0.008f, 60591 },{ "fxe5", 0.989f, 19353 },{ "Qh4", -0.965f, 75697 },{ "dxc3", 0.997f, 21951 },{ "Qh4", -0.973f, 38269 },
      { "g3", 0.999f, 830 },{ "Qe4", 0.009f, 42973 },{ "Nf3", 0.964f, 436 },{ "Bxf3", -0.964f, 79517 },{ "d4", 0.978f, 18358 },{ "Bxf3", -0.978f, 505006 },
      { "Bf4", 0.982f, 84959 },{ "Bxf3", -0.976f, 588818 },{ "Kxf3", 0.999f, 13123 },{ "Kb8", -0.984f, 349471 },{ "dxe5", 2.997f, 114599 },{ "Qh5", -1.999f, 41717 },
      { "Qb1", 2.987f, 7646 },{ "Qc4", -2.008f, 45123 },{ "Kd1", 2.008f, 583 },{ "Qxc3", -1.973f, 37369 },{ "Bh3", 2.003f, 2170 },{ "Qxf3", -0.970f, 45966 },
      { "Be2", 1.010f, 12536 },{ "Qxh1", 4.002f, 15624 },{ "Kxd2", -1.988f, 9134 },{ "Qxh1", 2.002f, 13919 },{ "Qd3", -1.982f, 2939 },{ "Ba3", 2.023f, 18705 },
      { "Rb2", -2.023f, 270 },{ "Bxb2", 3.961f, 24436 },{ "Qg4", 3.012f, 266 },{ "Qxh3", 2.001f, 6067 },{ "Qb5", -1.995f, 1558 },{ "Qd2", inff, 1937 },{ "Kb1", -inff, 225 },
      { "Qd1", inff, 225 },{ "Rxd1", -inff, 599 },{ "Rxd1", inff, 29162 }
    },
    {
    //  { "Nf3", -0.004f, 12312 },{ "Nc6", -1.033f, 37777 },{ "Qh5", -1.008f, 60589 },{ "Bb4", -1.038f, 76496 },{ "a3", -1.024f, 65139 },{ "exf4", -0.030f, 36218 },
    //  { "fxe5", -0.015f, 23394 },{ "exf4", -0.062f, 17962 },{ "Bb5", -0.035f, 37995 },{ "Nxc3", -1.014f, 29664 },{ "dxc3", -0.004f, 49722 },{ "Qh4", -0.999f, 68305 },
    //  { "Ke2", -2.050f, 4805 },{ "Qe4", -0.019f, 10927 },{ "Nf3", -2.055f, 5821 },{ "Bxf3", -0.992f, 6638 },{ "h3", -2.027f, 45306 },{ "f5", -1.001f, 13929 },
    //  { "h3", -2.009f, 64603 },{ "g6", -0.994f, 25501 },{ "gxf3", -0.051f, 33434 },{ "f6", -1.988f, 51200 },{ "dxe5", -0.035f, 42538 },{ "Qh5", -2.026f, 13307 },
    //  { "Qc1", -0.061f, 11326 },{ "Qc4", -2.034f, 8520 },{ "Kd1", -0.030f, 11955 },{ "Qxc3", -2.003f, 9010 },{ "Bh3", -1.072f, 8356 },{ "Qxf3", -1.999f, 22056 },
    //  { "Be2", -2.060f, 22215 },{ "Qxh1", 2.937f, 19268 },{ "Kxd2", -6.060f, 20218 },{ "Rd8", 1.001f, 29302 },{ "Kc1", -6.058f, 8657 },{ "Ba3", 1.950f, 12876 },
    //  { "Rb2", -8.032f, 2358 },{ "Bxb2", 3.934f, 2895 },{ "Qg4", -2.039f, 2742 },{ "Qxh3", 1.987f, 1252 },{ "Qb5", -5.023f, 15459 },{ "Bxb2", 0.952f, 4285 },
    //  { "Kb1", -inff, 79 },{ "Qd1", inff, 86 },{ "Rxd1", -inff, 4213 },{ "Rxd1", inff, 66572 }
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
      BOOST_REQUIRE_MESSAGE(info2.wait(&*engine, 60000), "Searching timeout at move '" + m + "' with " + std::to_string(depth) + " moves in depth");
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