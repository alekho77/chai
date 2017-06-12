#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>

#include "machinetestutils.h"
#include "../chessmachine/machine.h"

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

using namespace Chai::Chess;

CHESSBOARD;

BOOST_AUTO_TEST_SUITE ( ChessMachineTest )

BOOST_AUTO_TEST_CASE( ConstructorTest )  
{
  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  
  BOOST_REQUIRE(machine->GetSet(Set::white).empty());
  BOOST_REQUIRE(machine->GetSet(Set::black).empty());

  BOOST_CHECK(machine->EnumMoves(d4).empty());

  BOOST_CHECK(machine->CheckStatus() == Status::invalid);
  BOOST_CHECK(machine->CurrentPlayer() == Set::unknown);
  BOOST_CHECK(machine->LastMoveNotation().empty());
}

BOOST_AUTO_TEST_CASE( StartTest )
{
  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  BOOST_REQUIRE(d4 == d4);
  BOOST_REQUIRE(a4 < d4);
  BOOST_REQUIRE(a4 < a5);
  BOOST_REQUIRE(!(h4 < a1));

  machine->Start();
  BOOST_CHECK(machine->CheckStatus() == Status::normal);
  BOOST_CHECK(machine->CurrentPlayer() == Set::white);
  BOOST_CHECK(machine->LastMoveNotation() == std::string());

  Pieces white = machine->GetSet(Set::white);
  BOOST_REQUIRE(white.size() == 16);
  BOOST_REQUIRE(!contains(white, Type::bad));

  const std::map<Type, TestMoves> white_pieces = {
    { Type::pawn,{ { a2, { a4, a3 } },{ b2, {b3, b4 } },{ c2,{ c3, c4 } },{ d2,{ d4, d3 } },{ e2,{ e3, e4 } },{ f2,{ f3, f4 } },{ g2,{ g3, g4 } },{ h2,{ h3, h4 } } } },
    { Type::knight, { { b1,{ a3, c3 } },{ g1,{ f3, h3 } } } },
    { Type::bishop, { { c1,{} },{ f1,{} } } },
    { Type::rook, { { a1,{} }, { h1,{} } } },
    { Type::queen, { { d1,{} } } },
    { Type::king, { { e1,{} } } }
  };
  testpos(white_pieces, white, *machine);

  Pieces black = machine->GetSet(Set::black);
  BOOST_REQUIRE(black.size() == 16);
  BOOST_REQUIRE(!contains(black, Type::bad));

  const std::map<Type, TestMoves> black_pieces = {
    { Type::pawn,{ { a7,{ a6, a5 } },{ b7,{ b6, b5 } },{ c7,{ c6, c5 } },{ d7,{ d6, d5 } },{ e7,{ e6, e5 } },{ f7,{ f6, f5 } },{ g7,{ g6, g5 } },{ h7,{ h6, h5 } } } },
    { Type::knight,{ { b8,{ c6, a6 } },{ g8,{ h6, f6 } } } },
    { Type::bishop,{ { c8,{} },{ f8,{} } } },
    { Type::rook,{ { a8,{} },{ h8,{} } } },
    { Type::queen,{ { d8,{} } } },
    { Type::king,{ { e8,{} } } }
  };
  testpos(black_pieces, black, *machine);
}

BOOST_AUTO_TEST_CASE( InsidiousBunchTest)
{
  /*
    Insidious bunch

    Keres Arlamovsky
    Szczawno Zdroj, 1950
    Caro-Kann Defence
  */
  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 c6 2.Nc3 d5 3.Nf3 dxe4 4.Nxe4 Nf6 5.Qe2 Nbd7 6.Nd6#");
  BOOST_REQUIRE(moves.size() == 11);

  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    BOOST_CHECK_MESSAGE(machine->LastMoveNotation() == m, "Can't take move " + m);
    if (m == "dxe4") {
      BOOST_TEST_MESSAGE(m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      BOOST_CHECK(machine->CurrentPlayer() == Set::white);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{ a4, a3 } },{ b2,{ b3, b4 } },{ c2,{} },{ d2,{ d4, d3 } },{ f2,{} },{ g2,{ g3, g4 } },{ h2,{ h3, h4 } } } },
        { Type::knight,{ { c3,{ b5, d5, a4, e4, b1, e2 } },{ f3,{ d4, e5, g5, h4, g1 } } } },
        { Type::bishop,{ { c1,{} },{ f1,{ e2, d3, c4, b5, a6 } } } },
        { Type::rook,{ { a1,{ b1 } },{ h1,{ g1 } } } },
        { Type::queen,{ { d1,{ e2 } } } },
        { Type::king,{ { e1,{ e2 } } } }
      };
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a7,{ a6, a5 } },{ b7,{ b6, b5 } },{ c6,{ c5 } },{ e4,{ e3, f3 } },{ e7,{ e6, e5 } },{ f7,{ f6, f5 } },{ g7,{ g6, g5 } },{ h7,{ h6, h5 } } } },
        { Type::knight,{ { b8,{ a6, d7 } },{ g8,{ h6, f6 } } } },
        { Type::bishop,{ { c8,{ d7, e6, f5, g4, h3 } },{ f8,{} } } },
        { Type::rook,{ { a8,{} },{ h8,{} } } },
        { Type::queen,{ { d8,{ d7, d6, d5, d4, d3, d2, c7, b6, a5 } } } },
        { Type::king,{ { e8,{ d7 } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (m == "Nd6" ) {
      BOOST_TEST_MESSAGE(m);
      BOOST_CHECK(machine->CheckStatus() == Status::checkmate);
      BOOST_CHECK(machine->CurrentPlayer() == Set::black);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{ a4, a3 } },{ b2,{ b3, b4 } },{ c2,{ c3, c4 } },{ d2,{ d4, d3 } },{ f2,{} },{ g2,{ g3, g4 } },{ h2,{ h3, h4 } } } },
        { Type::knight,{ { d6,{ c8, e8, b7, f7, b5, f5, c4, e4 } },{ f3,{ d4, e5, g5, h4, g1 } } } },
        { Type::bishop,{ { c1,{} },{ f1,{} } } },
        { Type::rook,{ { a1,{ b1 } },{ h1,{ g1 } } } },
        { Type::queen,{ { e2,{ e3, e4, e5, e6, e7, d3, c4, b5, a6, d1 } } } },
        { Type::king,{ { e1,{ d1 } } } }
      };
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a7,{} },{ b7,{} },{ c6,{} },{ e7,{} },{ f7,{} },{ g7,{} },{ h7,{} } } },
        { Type::knight,{ { d7,{} },{ f6,{} } } },
        { Type::bishop,{ { c8,{} },{ f8,{} } } },
        { Type::rook,{ { a8,{} },{ h8,{} } } },
        { Type::queen,{ { d8,{} } } },
        { Type::king,{ { e8,{} } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    }
  }
}

BOOST_AUTO_TEST_CASE( HamletAmateurTest )
{
  /*
    Hamlet Amateur
    Vienna 1899
    Defence Pirc-Ufimtsev
  */
  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 d6 2.d4 Nd7 3.Bc4 g6 4.Nf3 Bg7 5.Bxf7+ Kxf7 6.Ng5+ Kf6 7.Qf3#");
  BOOST_REQUIRE(moves.size() == 13);
  for (auto m : moves) {
    if (m == "Bxf7") {
      BOOST_TEST_MESSAGE("Before " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      BOOST_CHECK(machine->CurrentPlayer() == Set::white);
      BOOST_REQUIRE_MESSAGE(machine->Move("O-O"), "Can't make move O-O");
      BOOST_CHECK(machine->LastMoveNotation() == std::string("O-O"));
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{ a4, a3 } },{ b2,{ b3, b4 } },{ c2,{ c3 } },{ d4,{ d5 } },{ e4,{ e5 } },{ f2,{} },{ g2,{ g3, g4 } },{ h2,{ h3, h4 } } } },
        { Type::knight,{ { b1,{ a3, c3, d2 } },{ f3,{ e5, g5, h4, e1, d2 } } } },
        { Type::bishop,{ { c1,{ d2, e3, f4, g5, h6 } },{ c4,{ b3, d5, e6, f7, b5, a6, d3, e2 } } } },
        { Type::rook,{ { a1,{} },{ f1,{ e1 } } } },
        { Type::queen,{ { d1,{ e1, e2, d2, d3 } } } },
        { Type::king,{ { g1,{ h1 } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      machine->Undo();
    }
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    BOOST_CHECK_MESSAGE(machine->LastMoveNotation() == m, "Can't take move " + m);
    if (m == "Bxf7") {
      BOOST_TEST_MESSAGE("After " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      BOOST_CHECK(machine->CurrentPlayer() == Set::black);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{ a4, a3 } },{ b2,{ b3, b4 } },{ c2,{ c3, c4 } },{ d4,{ d5 } },{ e4,{ e5 } },{ f2,{} },{ g2,{ g3, g4 } },{ h2,{ h3, h4 } } } },
        { Type::knight,{ { b1,{ a3, c3, d2 } },{ f3,{ e5, g5, h4, g1, d2 } } } },
        { Type::bishop,{ { c1,{ d2, e3, f4, g5, h6 } },{ f7,{ e8, e6, d5, c4, b3, g8, g6 } } } },
        { Type::rook,{ { a1,{} },{ h1,{ g1, f1 } } } },
        { Type::queen,{ { d1,{ e2, d2, d3 } } } },
        { Type::king,{ { e1,{ f1, e2, d2, g1 /*O-O*/ } } } }
      };
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a7,{} },{ b7,{} },{ c7,{} },{ d6,{} },{ e7,{} },{ g6,{} },{ h7,{} } } },
        { Type::knight,{ { d7,{} },{ g8,{} } } },
        { Type::bishop,{ { c8,{} },{ g7,{} } } },
        { Type::rook,{ { a8,{} },{ h8,{} } } },
        { Type::queen,{ { d8,{} } } },
        { Type::king,{ { e8,{ f8, f7 } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (m == "Qf3") {
      BOOST_TEST_MESSAGE(m);
      BOOST_CHECK(machine->CheckStatus() == Status::checkmate);
      BOOST_CHECK(machine->CurrentPlayer() == Set::black);
    }
  }
}

BOOST_AUTO_TEST_CASE( DebutSubtletyTest )
{
  /*
    Debut subtlety
    Lasker MULLER
    1934
    Caro-Kann Defence
  */
  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 c6 2.Nc3 d5 3.Nf3 dxe4 4.Nxe4 Bf5 5.Ng3 Bg6 6.h4 h6 7.Ne5 Bh7 8.Qh5 g6 9.Qf3 Nf6 10.Qb3 Qd5 11.Qxb7 Qxe5+ 12.Be2 Qd6 13.Qxa8 Qc7 14.a4 Bg7 15.Ra3 O-O 16.Rb3");
  BOOST_REQUIRE(moves.size() == 31);
  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    BOOST_CHECK_MESSAGE(machine->LastMoveNotation() == m, "Can't take move " + m);
    if (m == "Qxe5") {
      BOOST_TEST_MESSAGE(m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      BOOST_CHECK(machine->CurrentPlayer() == Set::white);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{} },{ b2,{} },{ c2,{} },{ d2,{} },{ f2,{} },{ g2,{} },{ h4,{} } } },
        { Type::knight,{ { g3,{ e2, e4 } } } },
        { Type::bishop,{ { c1,{} },{ f1,{ e2 } } } },
        { Type::rook,{ { a1,{} },{ h1,{} } } },
        { Type::queen,{ { b7,{} } } },
        { Type::king,{ { e1,{ d1 } } } }
      };
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a7,{ a6,a5 } },{ c6,{ c5 } },{ e7,{ e6 } },{ f7,{} },{ g6,{ g5 } },{ h6,{ h5 } } } },
        { Type::knight,{ { b8,{ a6,d7 } },{ f6,{ d7,g8,h5,g4,e4,d5 } } } },
        { Type::bishop,{ { f8,{ g7 } },{ h7,{ g8 } } } },
        { Type::rook,{ { a8,{} },{ h8,{ g8 } } } },
        { Type::queen,{ { e5,{ e6,e4,e3,e2,e1,d5,c5,b5,a5,f5,g5,h5,d6,c7,f4,g3,d4,c3,b2 } } } },
        { Type::king,{ { e8,{ d8, d7 /*because White to move, so we consider only the mobility*/ } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    }
  }
}

BOOST_AUTO_TEST_CASE( DangerousReidTest )
{
  /*
    DANGEROUS REID
    Botvinnik Spielmann
    Moscow 1935
    Caro-Kann Defence
  */
  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 c6 2.d4 d5 3.exd5 cxd5 4.c4 Nf6 5.Nc3 Nc6 6.Bg5 Qb6 7.cxd5 Qxb2 8.Rc1 Nb4 9.Na4 Qxa2 10.Bc4 Bg4 11.Nf3 Bxf3 12.gxf3");
  BOOST_REQUIRE(moves.size() == 23);
  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    BOOST_CHECK(machine->CheckStatus() == Status::normal);
    BOOST_CHECK_MESSAGE(machine->LastMoveNotation() == m, "Can't take move " + m);
  }
}

BOOST_AUTO_TEST_CASE( HaplessQueenTest )
{
  /*
    Hapless QUEEN
    Scandinavian Defense
  */
  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 d5 2.exd5 Qxd5 3.Nc3 Qa5 4.d4 Nf6 5.Bd2 Nc6 6.Bb5 Bd7 7.Nd5 Qxb5 8.Nxc7+ Kd8 9.Nxb5");
  BOOST_REQUIRE(moves.size() == 8*2+1);
  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    BOOST_CHECK_MESSAGE(machine->LastMoveNotation() == m, "Can't take move " + m);
    if (m == "Bd7") {
      BOOST_TEST_MESSAGE(m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      BOOST_CHECK(machine->CurrentPlayer() == Set::white);
      BOOST_REQUIRE_MESSAGE(machine->Move("Qe2"), "Can't make move Queen!");
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      {
        const std::map<Type, TestMoves> black_pieces = {
          { Type::pawn,{ { a7,{ a6 } },{ b7,{ b6 } },{ c7,{} },{ e7,{ e6, e5 } },{ f7,{} },{ g7,{ g5, g6 } },{ h7,{ h5, h6 } } } },
          { Type::knight,{ { c6,{ b8, d8, e5, d4, b4 } },{ f6,{ g8, h5, g4, e4, d5 } } } },
          { Type::bishop,{ { d7,{ c8,e6,f5,g4,h3 } },{ f8,{} } } },
          { Type::rook,{ { a8,{ b8,c8,d8 } },{ h8,{ g8 } } } },
          { Type::queen,{ { a5,{ a6,b6,b5,b4,c3,a4,a3,a2 } } } },
          { Type::king,{ { e8,{ d8, c8 /*O-O-O*/} } } }
        };
        testpos(black_pieces, machine->GetSet(Set::black), *machine);
      }
      BOOST_REQUIRE_MESSAGE(machine->Move("O-O-O"), "Can't make move O-O-O!");
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      BOOST_CHECK(machine->LastMoveNotation() == std::string("O-O-O"));
      {
        const std::map<Type, TestMoves> black_pieces = {
          { Type::pawn,{ { a7,{ a6 } },{ b7,{ b6 } },{ c7,{} },{ e7,{ e6, e5 } },{ f7,{} },{ g7,{ g5, g6 } },{ h7,{ h5, h6 } } } },
          { Type::knight,{ { c6,{ b8, e5, d4, b4 } },{ f6,{ e8,g8, h5, g4, e4, d5 } } } },
          { Type::bishop,{ { d7,{ e8,e6,f5,g4,h3 } },{ f8,{} } } },
          { Type::rook,{ { d8,{ e8 } },{ h8,{ g8 } } } },
          { Type::queen,{ { a5,{ a6,b6,b5,b4,c3,a4,a3,a2 } } } },
          { Type::king,{ { c8,{ b8 } } } }
        };
        const std::map<Type, TestMoves> white_pieces = {
          { Type::pawn,{ { a2,{ a3, a4 } },{ b2,{ b3,b4 } },{ c2,{} },{ d4,{ d5 } },{ f2,{ f3,f4 } },{ g2,{ g3,g4 } },{ h2,{ h3,h4 } } } },
          { Type::knight,{ { c3,{ a4,d5,e4,d1,b1 } },{ g1,{ f3,h3 } } } },
          { Type::bishop,{ { b5,{ a6,c6,c4,d3,a4 } },{ d2,{ e3,f4,g5,h6,c1 } } } },
          { Type::rook,{ { a1,{ b1,c1,d1 } },{ h1,{} } } },
          { Type::queen,{ { e2,{ d3,c4,e3,e4,e5,e6,e7,f3,g4,h5,f1,d1 } } } },
          { Type::king,{ { e1,{ d1,f1,c1 /*O-O-O*/ } } } }
        };
        testpos(white_pieces, machine->GetSet(Set::white), *machine);
        testpos(black_pieces, machine->GetSet(Set::black), *machine);
      }
      BOOST_REQUIRE_MESSAGE(machine->Move("O-O-O"), "Can't make move O-O-O!");
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      BOOST_CHECK(machine->LastMoveNotation() == std::string("O-O-O"));
      {
        const std::map<Type, TestMoves> white_pieces = {
          { Type::pawn,{ { a2,{ a3, a4 } },{ b2,{ b3,b4 } },{ c2,{} },{ d4,{ d5 } },{ f2,{ f3,f4 } },{ g2,{ g3,g4 } },{ h2,{ h3,h4 } } } },
          { Type::knight,{ { c3,{ a4,d5,e4,b1 } },{ g1,{ f3,h3 } } } },
          { Type::bishop,{ { b5,{ a6,c6,c4,d3,a4 } },{ d2,{ e3,f4,g5,h6,e1 } } } },
          { Type::rook,{ { d1,{ e1,f1 } },{ h1,{} } } },
          { Type::queen,{ { e2,{ d3,c4,e3,e4,e5,e6,e7,f3,g4,h5,f1,e1 } } } },
          { Type::king,{ { c1,{ b1 } } } }
        };
        testpos(white_pieces, machine->GetSet(Set::white), *machine);
      }
      machine->Undo();
      machine->Undo();
      machine->Undo();
    } else if (m == "Nxc7") {
      BOOST_TEST_MESSAGE(m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      BOOST_CHECK(machine->CurrentPlayer() == Set::black);
    }
  }
}

BOOST_AUTO_TEST_CASE( HorseBetterQueenTest )
{
  /*
    HORSE IS BETTER THAN THE QUEEN
  */
  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.d4 d5 2.c4 e5 3.dxe5 d4 4.e3 Bb4+ 5.Bd2 dxe3 6.Bxb4 exf2+ 7.Ke2 fxg1=N+ 8.Rxg1 Bg4+");
  BOOST_REQUIRE(moves.size() == 8*2);
  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    BOOST_CHECK_MESSAGE(machine->LastMoveNotation() == m, "Can't take move " + m);
    if (m == "fxg1=N") {
      BOOST_TEST_MESSAGE(m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      BOOST_CHECK(machine->CurrentPlayer() == Set::white);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a7,{ a6,a5 } },{ b7,{ b6,b5 } },{ c7,{ c6,c5 } },{ f7,{ f6, f5 } },{ g7,{ g5, g6 } },{ h7,{ h5, h6 } } } },
        { Type::knight,{ { b8,{ a6,c6,d7 } },{ g8,{ h6,f6,e7 } },{ g1, {e2,f3,h3} } } },
        { Type::bishop,{ { c8,{ d7,e6,f5,g4,h3 } } } },
        { Type::rook,{ { a8,{} },{ h8,{} } } },
        { Type::queen,{ { d8,{ d7,d6,d5,d4,d3,d2,d1,e7,f6,g5,h4 } } } },
        { Type::king,{ { e8,{ d7,e7,f8 /*only mobility*/ } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{} },{ b2,{} },{ c4,{} },{ e5, {} },{ g2,{} },{ h2,{} } } },
        { Type::knight,{ { b1,{} } } },
        { Type::bishop,{ { b4,{} },{ f1,{} } } },
        { Type::rook,{ { a1,{} },{ h1,{ g1 } } } },
        { Type::queen,{ { d1,{} } } },
        { Type::king,{ { e2,{ e1,e3,f2 } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      
      machine->Undo();
      BOOST_REQUIRE_MESSAGE(machine->Move("fxg1N"), "Can't make move fxg1N");
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
    }
  }
}

BOOST_AUTO_TEST_CASE( EnPassantTest )
{
  boost::shared_ptr<IMachine> machine = boost::make_shared<ChessMachine>();
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("\
1.d4 d6 2.d5 e5 3.dxe6 fxe6 4.Na3 b5 5.Nh3 b4 6.c4 bxc3 7.bxc3 d5 8.c4 d4 9.e4\n\
g5 10.f4 d3 11.fxg5 h5 12.Nf4 e5 13.g6 a5 14.Bb2 c6 15.Qxd3 Qxd3 16.Rd1 Bg4\n\
17.Rxd3 Na6 18.c5 Rd8 19.g3 Bh3 20.Be2 h4 21.Rf1 Bxc5 22.gxh4 Nh6 23.Bg4 a4\n\
24.Be6 exf4 25.Rxf4 Rxd3 26.Ke2 Be7 27.Bxh3 Bxh4 28.g7 Ng4 29.e5 Rxh3 30.\
gxh8=Q+ Kd7 31.Rf7+ Ke6 32.Qf6+ Kd5 33.e6 Nb4 34.Qd8+ Kxe6 35.Qe7+ Kd5 36.Qe5+\n\
Nxe5 37.Bxe5 Kxe5 38.Re7+ Kf5 39.Nc4 Bxe7 40.a3 Rd3 41.h4 c5 42.axb4 cxb4 43.\
h5 Bh4 44.h6 Rh3 45.h7 Rh2+ 46.Ke3 b3 47.h8=Q b2 48.Qh5+ Ke6 49.Nxb2 a3 50.\
Qe8+ Kd6 51.Qe4 Rxb2 52.Qxh4 a2 53.Qd4+ Ke7 54.Qxb2 a1=Q 55.Qxa1 Kd7 56.Qd4+\n\
Kc7 57.Ke4 Kc6 58.Ke5 Kc7 59.Qd6+ Kb7 60.Kd5 Kc8 61.Kc6 *");
  BOOST_REQUIRE(moves.size() == 60*2+1);
  int nm = 0;
  for (auto m : moves) {
    const int move = nm / 2 + 1;
    BOOST_CHECK(machine->CurrentPlayer() == (nm & 1 ? Set::black : Set::white));
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m + " (#" + std::to_string(move) + ")");
    BOOST_CHECK(machine->CurrentPlayer() == (nm & 1 ? Set::white : Set::black));
    BOOST_CHECK_MESSAGE(machine->LastMoveNotation() == m, "Can't take move " + m);
    if (move == 2 && m == "e5") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{a3,a4} },{ b2,{b3,b4} },{ c2,{c3,c4} },{ d5,{e6/*en passant*/}},{ e2,{e3,e4} },{f2,{f3,f4}},{ g2,{g3,g4} },{ h2,{h3,h4} } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
    } else if (move == 3 && m == "dxe6") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{ a3,a4 } },{ b2,{ b3,b4 } },{ c2,{ c3,c4 } },{ e6,{ e7,f7 } },{ e2,{ e3,e4 } },{ f2,{ f3,f4 } },{ g2,{ g3,g4 } },{ h2,{ h3,h4 } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a7,{ a6,a5 } },{ b7,{ b6,b5 } },{ c7,{ c6,c5 } },{d6,{d5}},{ f7,{ f6, f5, e6 } },{ g7,{ g5, g6 } },{ h7,{ h5, h6 } } } }
      };
      const auto black = machine->GetSet(Set::black);
      testpos(black_pieces, black, *machine);
      BOOST_REQUIRE(at(black, e5) == black.end());
    } else if (move == 6 && m == "c4") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a7,{ a6,a5 } },{ b4,{ b3,a3,c3/*en passant*/ } },{ c7,{ c6,c5 } },{ d6,{d5} },{ e6,{ e5 } },{ g7,{ g6,g5 } },{ h7,{ h5,h6 } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 6 && m == "bxc3") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a7,{ a6,a5 } },{ c3,{ b2,c2 } },{ c7,{ c6,c5 } },{ d6,{ d5 } },{ e6,{ e5 } },{ g7,{ g6,g5 } },{ h7,{ h5,h6 } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{} },{ b2,{ b3,b4,c3 } },{e2,{e3,e4}},{ f2,{ f3,f4 }},{ g2,{ g3,g4 } },{ h2,{} } } }
      };
      const auto white = machine->GetSet(Set::white);
      testpos(white_pieces, white, *machine);
      BOOST_REQUIRE(at(white, c4) == white.end());
    } else if (move == 9 && m == "e4") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a7,{ a6,a5 } },{ c7,{ c6,c5 } },{ d4,{ d3,e3 /*en passant*/ } },{ e6,{ e5 } },{ g7,{ g6,g5 } },{ h7,{ h5,h6 } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 10 && m == "f4") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a7,{ a6,a5 } },{ c7,{ c6,c5 } },{ d4,{ d3 /*no more en passant*/ } },{ e6,{ e5 } },{ g5,{ g4,f4 } },{ h7,{ h5,h6 } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 11 && m == "h5") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{} },{ c4,{ c5 } },{ e4,{ e5 } },{ g5,{ g6,h6/*en passant*/ } },{ g2,{ g3,g4 } },{ h2,{} } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
    } else if (move == 12 && m == "e5") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{} },{ c4,{ c5 } },{ e4,{} },{ g5,{ g6/*no more en passant*/ } },{ g2,{ g3,g4 } },{ h2,{h3,h4} } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
    } else if (nm == 14 * 2 + 1 && m == "Qxd3") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::king,{ { e1,{ f2 /*impossible castling*/ } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
    } else if (move == 18 && m == "c5") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::king,{ { e8,{ e7 /*impossible castling*/ } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 20 && m == "h4") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::king,{ { e1,{ f2,d2,d1 /*impossible castling*/ } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
    } else if (move == 23 && m == "Bg4") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::king,{ { e8,{ e7,f8,g8 /*possible castling*/ } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 24 && m == "Be6") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::king,{ { e8,{ e7,f8 /*no more possible castling*/ } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 27 && m == "Bxh3") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::king,{ { e8,{ d8 /*impossible castling*/ } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 30 && m == "gxh8=Q") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{} },{ e5,{e6} },{ h2,{} } } },
        { Type::knight,{ { a3,{b5,c4,c2,b1} } } },
        { Type::bishop,{ { b2,{c3,d4,c1,a1} } } },
        { Type::rook,{ { f4,{f5,f6,f7,f8,f3,f2,f1,g4,e4,d4,c4,b4,a4} } } },
        { Type::queen,{ { h8,{g8,f8,e8,g7,f6,h7,h6,h5,h4} } } },
        { Type::king,{ { e2,{ d3,e3,f3,f2,d2,d1,e1,f1 /*only the mobility*/ } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a4,{} },{ c6,{} } } },
        { Type::knight,{ { a6,{} },{ g4,{} } } },
        { Type::bishop,{ { h4,{} } } },
        { Type::rook,{ { h3,{} } } },
        { Type::queen,{} },
        { Type::king,{ { e8,{ d7,e7 } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 31 && m == "Rf7") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    } else if (move == 32 && m == "Qf6") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    } else if (move == 34 && m == "Qd8") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    } else if (move == 35 && m == "Qe7") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    } else if (move == 36 && m == "Qe5") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    } else if (move == 38 && m == "Re7") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { a2,{} },{ h2,{} } } },
        { Type::knight,{ { a3,{ b5,c4,c2,b1 } } } },
        { Type::bishop,{} },
        { Type::rook,{ { e7,{ e8,e6,e5,a7,b7,c7,d7,f7,g7,h7 } } } },
        { Type::queen,{} },
        { Type::king,{ { e2,{ d3,e3,f3,f2,d2,d1,e1,f1 /*only the mobility*/ } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a4,{} },{ c6,{} } } },
        { Type::knight,{ { b4,{} } } },
        { Type::bishop,{ { h4,{e7} } } },
        { Type::rook,{ { h3,{} } } },
        { Type::queen,{} },
        { Type::king,{ { e5,{ d6,f6,d5,f5,d4,f4 } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 45 && m == "Rh2") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{ { h7,{} } } },
        { Type::knight,{ { c4,{} } } },
        { Type::bishop,{} },
        { Type::rook,{} },
        { Type::queen,{} },
        { Type::king,{ { e2,{ d3,e3,f3,d1,f1 } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a4,{a3}},{ b4,{b3} } } },
        { Type::knight,{} },
        { Type::bishop,{ { h4,{ g5,f6,e7,d8,g3,f2,e1 } } } },
        { Type::rook,{ { h2,{h3,h1,g2,f2,e2} } } },
        { Type::queen,{} },
        { Type::king,{ { f5,{ e6,f6,g6,g5,e4,f4,g4,e5 /*only the mobility*/} } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 47 && m == "h8=Q") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{} },
        { Type::knight,{ { c4,{a5,b6,d6,e5,d2,b2,a3} } } },
        { Type::bishop,{} },
        { Type::rook,{} },
        { Type::queen,{ {h8,{a8,b8,c8,d8,e8,f8,g8,a1,b2,c3,d4,e5,f6,g7,h7,h6,h5,h4}} } },
        { Type::king,{ { e3,{ d4,e4,f4,d3,f3,d2,e2,f2 /*only the mobility*/} } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{ { a4,{ a3 } },{ b3,{ b2 } } } },
        { Type::knight,{} },
        { Type::bishop,{ { h4,{ g5,f6,e7,d8,g3,f2,e1 } } } },
        { Type::rook,{ { h2,{ h3,h1,a2,b2,c2,d2,e2,f2,g2 } } } },
        { Type::queen,{} },
        { Type::king,{ { f5,{ e6,g6,g5,g4 } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 48 && m == "Qh5") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    } else if (move == 50 && m == "Qe8") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    } else if (move == 53 && m == "Qd4") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    } else if (move == 54 && m == "a1=Q") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::normal);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{} },
        { Type::knight,{} },
        { Type::bishop,{} },
        { Type::rook,{} },
        { Type::queen,{ { b2,{ a1,c3,d4,e5,f6,g7,h8,a3,c1,b1,b3,b4,b5,b6,b7,b8,a2,c2,d2,e2,f2,g2,h2 } } } },
        { Type::king,{ { e3,{ d4,e4,f4,d3,f3,d2,e2,f2 } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{} },
        { Type::knight,{} },
        { Type::bishop,{} },
        { Type::rook,{} },
        { Type::queen,{ {a1,{a2,a3,a4,a5,a6,a7,a8,b1,c1,d1,e1,f1,g1,h1,b2}} } },
        { Type::king,{ { e7,{ d8,e8,f8,d7,f7,d6,e6,f6 /*only the mobility*/ } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    } else if (move == 56 && m == "Qd4") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    } else if (move == 59 && m == "Qd6") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    } else if (move == 61 && m == "Kc6") {
      BOOST_TEST_MESSAGE(std::to_string(move) + " " + m);
      BOOST_CHECK(machine->CheckStatus() == Status::stalemate);
      const std::map<Type, TestMoves> white_pieces = {
        { Type::pawn,{} },
        { Type::knight,{} },
        { Type::bishop,{} },
        { Type::rook,{} },
        { Type::queen,{ { d6,{ d1,d2,d3,d4,d5,d7,d8,e6,f6,g6,h6,a3,b4,c5,e7,f8,b8,c7,e5,f4,g3,h2 } } } },
        { Type::king,{ { c6,{ b7,c7,d7,b6,b5,c5,d5 } } } }
      };
      testpos(white_pieces, machine->GetSet(Set::white), *machine);
      const std::map<Type, TestMoves> black_pieces = {
        { Type::pawn,{} },
        { Type::knight,{} },
        { Type::bishop,{} },
        { Type::rook,{} },
        { Type::queen,{} },
        { Type::king,{ { c8,{ /*stalemate*/ } } } }
      };
      testpos(black_pieces, machine->GetSet(Set::black), *machine);
    }
    nm++;
  }
}

BOOST_AUTO_TEST_SUITE_END()
