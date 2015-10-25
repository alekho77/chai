#include "stdafx.h"

namespace Chai {
  namespace Chess {
    CHESSBOARD;

    typedef std::map< Postion, std::set<Postion> > Moves;

    template <size_t N, class T>
    bool contains(const T (&list)[N], Type type) {
      return std::find_if(&(list[0]), &(list[N]), [type](const auto& p) { return p.type == type; }) != &(list[N]);
    }
    template <size_t N, class T>
    typename size_t count(const T(&list)[N], Type type) {
      return std::count_if(&(list[0]), &(list[N]), [type](const auto& p) { return p.type == type; });
    }
    template <size_t N, class T>
    typename const T* at(const T(&list)[N], Postion pos) {
      return std::find_if(&(list[0]), &(list[N]), [pos](const auto& p) { return p.position == pos; });
    }
    template <size_t N, class T>
    bool exactly(const T(&list)[N], Postion pos, Type type) {
      auto p = at(list, pos);
      if (p != &(list[N])) {
        return p->type == type;
      }
      return false;
    }
    template <class T>
    bool equal(const T& list1, const T& list2) {
      return list1 == list2;
    }
    template <size_t N, class T>
    std::set<Postion> moves(const T(&list)[N]) {
      std::set<Postion> ms;
      for (const auto& m : list) {
        if (m.move != BADPOS) {
          ms.insert(m.move);
        }
      }
      return ms;
    }
  }
}


BOOST_AUTO_TEST_SUITE ( ChessMachineTest )

BOOST_AUTO_TEST_CASE( ConstructorTest )  
{
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  
  SetPieces white = machine->GetSet(Set::white);
  SetPieces black = machine->GetSet(Set::black);

  BOOST_CHECK(white.count == 0);
  BOOST_CHECK(black.count == 0);

  for (auto p : white.pieces) {
    BOOST_CHECK(p.type == Type::bad);
    BOOST_CHECK(p.position == BADPOS);
  }
  for (auto p : black.pieces) {
    BOOST_CHECK(p.type == Type::bad);
    BOOST_CHECK(p.position == BADPOS);
  }

  PieceMoves moves = machine->CheckMoves(d4);
  BOOST_CHECK(moves.count == 0);
  for (auto m : moves.moves) {
    BOOST_CHECK(!m.threat);
    BOOST_CHECK(m.move == BADPOS);
  }
}

BOOST_AUTO_TEST_CASE( StartTest )
{
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  BOOST_REQUIRE(d4 == d4);
  BOOST_REQUIRE(a4 < d4);
  BOOST_REQUIRE(a4 < a5);
  BOOST_REQUIRE(!(h4 < a1));

  machine->Start();

  SetPieces white = machine->GetSet(Set::white);
  BOOST_REQUIRE(white.count == 16);
  BOOST_REQUIRE(!contains(white.pieces, Type::bad));

  const std::map<Type, Moves> white_pieces = {
    { Type::pawn,{ { a2,{ a4, a3 } },{ b2,{ b3, b4 } },{ c2,{ c3, c4 } },{ d2,{ d4, d3 } },{ e2,{ e3, e4 } },{ f2,{ f3, f4 } },{ g2,{ g3, g4 } },{ h2,{ h3, h4 } } } },
    { Type::knight, { { b1,{ a3, c3 } },{ g1,{ f3, h3 } } } },
    { Type::bishop, { { c1,{} },{ f1,{} } } },
    { Type::rook, { { a1,{} }, { h1,{} } } },
    { Type::queen, { { d1,{} } } },
    { Type::king, { { e1,{} } } }
  };
  for (const auto& p : white_pieces) {
    for (const auto& m : p.second) {
      BOOST_CHECK(exactly(white.pieces, m.first, p.first) && equal(moves(machine->CheckMoves(m.first).moves), m.second));
    }
  }

  SetPieces black = machine->GetSet(Set::black);
  BOOST_REQUIRE(black.count == 16);
  BOOST_REQUIRE(!contains(black.pieces, Type::bad));

  const std::map<Type, Moves> black_pieces = {
    { Type::pawn,{ { a7,{ a6, a5 } },{ b7,{ b6, b5 } },{ c7,{ c6, c5 } },{ d7,{ d6, d5 } },{ e7,{ e6, e5 } },{ f7,{ f6, f5 } },{ g7,{ g6, g5 } },{ h7,{ h6, h5 } } } },
    { Type::knight,{ { b8,{ c6, a6 } },{ g8,{ h6, f6 } } } },
    { Type::bishop,{ { c8,{} },{ f8,{} } } },
    { Type::rook,{ { a8,{} },{ h8,{} } } },
    { Type::queen,{ { d8,{} } } },
    { Type::king,{ { e8,{} } } }
  };
  for (const auto& p : black_pieces) {
    for (const auto& m : p.second) {
      BOOST_CHECK(exactly(black.pieces, m.first, p.first) && equal(moves(machine->CheckMoves(m.first).moves), m.second));
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
