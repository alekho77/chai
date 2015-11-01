#include "stdafx.h"

namespace Chai {
  namespace Chess {
    CHESSBOARD;

    typedef std::map< Postion, std::set<Postion> > Moves;

    template <class T>
    bool contains(const T& list, Type type) {
      return std::find_if(list.begin(), list.end(), [type](const auto& p) { return p.type == type; }) != list.end();
    }

    template <size_t N, class T>
    typename size_t count(const T(&list)[N], Type type) {
      return std::count_if(&(list[0]), &(list[N]), [type](const auto& p) { return p.type == type; });
    }

    template <class T>
    typename typename T::const_iterator at(const T& list, Postion pos) {
      return std::find_if(list.begin(), list.end(), [pos](const auto& p) { return p.position == pos; });
    }

    template <class T>
    bool exactly(const T& list, Postion pos, Type type) {
      auto p = at(list, pos);
      if (p != list.end()) {
        return p->type == type;
      }
      return false;
    }

    template <class T>
    bool equal(const T& list1, const T& list2) {
      return list1 == list2;
    }

    std::vector<Piece> arr2vec(const Piece* p) {
      std::vector<Piece> vec;
      if (p) {
        while (p->type != Type::bad) {
          vec.push_back(*(p++));
        }
      }
      return vec;
    }

    std::set<Postion> arr2vec(const Postion* p) {
      std::set<Postion> vec;
      if (p) {
        while (*p != BADPOS) {
          vec.insert(*(p++));
        }
      }
      return vec;
    }

    std::vector<std::string> split(const std::string& game) {
      std::vector<std::string> moves;
      boost::regex xreg("(?|(\\d+)\\.([p,N,B,R,Q,K,1-8,a-h,x]+)\\s+([p,N,B,R,Q,K,1-8,a-h,x]+)|(\\d+)\\.([p,N,B,R,Q,K,1-8,a-h,x]+))");
      for (auto xit = make_regex_iterator(game, xreg); xit != boost::sregex_iterator(); ++xit) {
        auto& res = *xit;
        assert(res.size() == 4);
        int nm = std::stoi(res[1].str());
        assert(nm == (moves.size() / 2 + 1));
        moves.push_back(res[2].str());
        std::string bm = res[3].str();
        if (!bm.empty()) {
          moves.push_back(bm);
        }
      }
      return moves;
    }
  }
}


BOOST_AUTO_TEST_SUITE ( ChessMachineTest )

BOOST_AUTO_TEST_CASE( ConstructorTest )  
{
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  
  BOOST_REQUIRE(machine->GetSet(Set::white) == nullptr);
  BOOST_REQUIRE(machine->GetSet(Set::black) == nullptr);

  BOOST_CHECK(machine->CheckMoves(d4) == nullptr);
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

  std::vector<Piece> white = arr2vec(machine->GetSet(Set::white));
  BOOST_REQUIRE(white.size() == 16);
  BOOST_REQUIRE(!contains(white, Type::bad));

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
      BOOST_CHECK(exactly(white, m.first, p.first));
      BOOST_CHECK(equal(arr2vec(machine->CheckMoves(m.first)), m.second));
    }
  }

  std::vector<Piece> black = arr2vec(machine->GetSet(Set::black));
  BOOST_REQUIRE(black.size() == 16);
  BOOST_REQUIRE(!contains(black, Type::bad));

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
      BOOST_CHECK(exactly(black, m.first, p.first) && equal(arr2vec(machine->CheckMoves(m.first)), m.second));
    }
  }
}

BOOST_AUTO_TEST_CASE( InsidiousBunchTest)
{
  /*
    Insidious bunch

    Keres Arlamovsky
    Szczawno Zdroj, 1950
    Caro-Kann Defence
  */
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 c6 2.Nc3 d5 3.Nf3 dxe4 4.Nxe4 Nf6 5.Qe2 Nbd7 6.Nd6#");
  BOOST_REQUIRE(moves.size() == 11);

  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
  }
}

BOOST_AUTO_TEST_SUITE_END()
