#include "stdafx.h"

namespace Chai {
  namespace Chess {
    CHESSBOARD;

    template <class T>
    bool contains(const T& list, Type type) {
      return std::find_if(list.begin(), list.end(), [type](const auto& p) { return p.type == type; }) != list.end();
    }
    template <class T>
    typename T::const_iterator at(const T& list, Postion pos) {
      return std::find_if(list.begin(), list.end(), [pos](const auto& p) { return p.position == pos; });
    }
    template <class T>
    typename T::difference_type count(const T& list, Set set) {
      return std::count_if(list.begin(), list.end(), [set](const auto& p) {return p.set == set;});
    }
    template <class T>
    typename T::difference_type count(const T& list, Set set, Type type) {
      return std::count_if(list.begin(), list.end(), [set, type](const auto& p) {return p.set == set && p.type == type;});
    }
    template <class T>
    bool exactly(const T& list, Postion pos, Set set, Type type) {
      auto p = at(list, pos);
      if (p != list.end()) {
        return p->set == set && p->type == type;
      }
      return false;
    }
    template <class T>
    std::vector<Postion> moves(const T& list, Postion pos) {
      auto piece = at(list, pos);
      if (piece != list.end()) {
        std::vector<Postion> vm;
        for (const Postion& m : piece->moves) {
          if (m != BADPOS) {
             vm.push_back(m);
          }
        }
        return vm;
      }
      throw std::exception("Piece don't found!");
    }
    template <class T>
    bool equal(T& list1, T& list2) {
      std::sort(list1.begin(), list1.end());
      std::sort(list2.begin(), list2.end());
      return std::includes(list1.begin(), list1.end(), list2.begin(), list2.end());
    }
    template <class T>
    bool test(const T& list, Postion pos, Set set, Type type, std::vector<Postion> m) {
      return exactly(list, pos, set, type) && equal(moves(list, pos), m);
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

  Snapshot shot;
  memset(&shot, 0xee, sizeof(shot));
  BOOST_CHECK(machine->GetSnapshot(shot));

  const std::vector<Snapshot::Piece> pieces(shot.pieces, shot.pieces + _countof(shot.pieces));

  BOOST_REQUIRE(!contains(pieces, Type::bad));
  
  BOOST_CHECK(count(pieces, Set::white) == 16);

  BOOST_CHECK(count(pieces, Set::white, Type::pawn) == 8);
  BOOST_CHECK(test(pieces, a2, Set::white, Type::pawn, { a3, a4 }));
  BOOST_CHECK(test(pieces, b2, Set::white, Type::pawn, { b3, b4 }));
  BOOST_CHECK(test(pieces, c2, Set::white, Type::pawn, { c3, c4 }));
  BOOST_CHECK(test(pieces, d2, Set::white, Type::pawn, { d3, d4 }));
  BOOST_CHECK(test(pieces, e2, Set::white, Type::pawn, { e3, e4 }));
  BOOST_CHECK(test(pieces, f2, Set::white, Type::pawn, { f3, f4 }));
  BOOST_CHECK(test(pieces, g2, Set::white, Type::pawn, { g3, g4 }));
  BOOST_CHECK(test(pieces, h2, Set::white, Type::pawn, { h3, h4 }));

  BOOST_CHECK(count(pieces, Set::white, Type::knight) == 2);
  BOOST_CHECK(test(pieces, b1, Set::white, Type::knight, { a3, c3 }));
  BOOST_CHECK(test(pieces, g1, Set::white, Type::knight, { f3, h3 }));

  BOOST_CHECK(count(pieces, Set::white, Type::bishop) == 2);
  BOOST_CHECK(test(pieces, c1, Set::white, Type::bishop, {}));
  BOOST_CHECK(test(pieces, f1, Set::white, Type::bishop, {}));

  BOOST_CHECK(count(pieces, Set::white, Type::rook) == 2);
  BOOST_CHECK(test(pieces, a1, Set::white, Type::rook, {}));
  BOOST_CHECK(test(pieces, h1, Set::white, Type::rook, {}));

  BOOST_CHECK(count(pieces, Set::white, Type::queen) == 1);
  BOOST_CHECK(test(pieces, d1, Set::white, Type::queen, {}));

  BOOST_CHECK(count(pieces, Set::white, Type::king) == 1);
  BOOST_CHECK(test(pieces, e1, Set::white, Type::king, {}));

  BOOST_CHECK(count(pieces, Set::black) == 16);

  BOOST_CHECK(count(pieces, Set::black, Type::pawn) == 8);
  BOOST_CHECK(test(pieces, a7, Set::black, Type::pawn, { a6, a5 }));
  BOOST_CHECK(test(pieces, b7, Set::black, Type::pawn, { b6, b5 }));
  BOOST_CHECK(test(pieces, c7, Set::black, Type::pawn, { c6, c5 }));
  BOOST_CHECK(test(pieces, d7, Set::black, Type::pawn, { d6, d5 }));
  BOOST_CHECK(test(pieces, e7, Set::black, Type::pawn, { e6, e5 }));
  BOOST_CHECK(test(pieces, f7, Set::black, Type::pawn, { f6, f5 }));
  BOOST_CHECK(test(pieces, g7, Set::black, Type::pawn, { g6, g5 }));
  BOOST_CHECK(test(pieces, h7, Set::black, Type::pawn, { h6, h5 }));

  BOOST_CHECK(count(pieces, Set::black, Type::knight) == 2);
  BOOST_CHECK(test(pieces, b8, Set::black, Type::knight, { a6, c6 }));
  BOOST_CHECK(test(pieces, g8, Set::black, Type::knight, { f6, h6 }));

  BOOST_CHECK(count(pieces, Set::black, Type::bishop) == 2);
  BOOST_CHECK(test(pieces, c8, Set::black, Type::bishop, {}));
  BOOST_CHECK(test(pieces, f8, Set::black, Type::bishop, {}));
  
  BOOST_CHECK(count(pieces, Set::black, Type::rook) == 2);
  BOOST_CHECK(test(pieces, a8, Set::black, Type::rook, {}));
  BOOST_CHECK(test(pieces, h8, Set::black, Type::rook, {}));

  BOOST_CHECK(count(pieces, Set::black, Type::queen) == 1);
  BOOST_CHECK(test(pieces, d8, Set::black, Type::queen, {}));

  BOOST_CHECK(count(pieces, Set::black, Type::king) == 1);
  BOOST_CHECK(test(pieces, e8, Set::black, Type::king, {}));
}

BOOST_AUTO_TEST_SUITE_END()
