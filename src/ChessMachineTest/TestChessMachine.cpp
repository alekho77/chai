#include "stdafx.h"

namespace Chai {
  namespace Chess {
    CHESSBOARD;

    typedef std::map< Position, std::set<Position> > Moves;

    template <class T>
    bool contains(const T& list, Type type) {
      return std::find_if(list.begin(), list.end(), [type](const auto& p) { return p.type == type; }) != list.end();
    }

    template <size_t N, class T>
    typename size_t count(const T(&list)[N], Type type) {
      return std::count_if(&(list[0]), &(list[N]), [type](const auto& p) { return p.type == type; });
    }

    template <class T>
    typename typename T::const_iterator at(const T& list, Position pos) {
      return std::find_if(list.begin(), list.end(), [pos](const auto& p) { return p.position == pos; });
    }

    template <class T>
    bool exactly(const T& list, Position pos, Type type) {
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

    std::set<Position> arr2vec(const Position* p) {
      std::set<Position> vec;
      if (p) {
        while (*p != BADPOS) {
          vec.insert(*(p++));
        }
      }
      return vec;
    }

    std::vector<std::string> split(const std::string& game) {
      std::vector<std::string> moves;
      boost::regex xreg("(?|(\\d+)\\.(?|([p,N,B,R,Q,K,1-8,a-h,x,=]+)|(O-O-O)|(O-O))\\+*\\s+(?|([p,N,B,R,Q,K,1-8,a-h,x,=]+)|(O-O-O)|(O-O))|(\\d+)\\.(?|([p,N,B,R,Q,K,1-8,a-h,x,=]+)|(O-O-O)|(O-O)))");
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

    std::string toStr(const Position& p) {
      return p.isValid() ? std::string(&(p.file), &(p.file) + 1) + std::string(&(p.rank), &(p.rank) + 1) : std::string("");
    }
    
    void testpos(const std::map<Type, Moves>& position, const std::vector<Piece>& pieces, const IChessMachine& machine) {
      static const std::map<Type, std::string> name = { { Type::pawn, "p" },{ Type::knight, "N" },{ Type::bishop, "B" },{ Type::rook, "R" },{ Type::queen, "Q" },{ Type::king, "K" } };
      for (const auto& p : position) {
        for (const auto& m : p.second) {
          BOOST_CHECK_MESSAGE(exactly(pieces, m.first, p.first), "The piece " + name.at(p.first) + " was not found at the position \"" + toStr(m.first) + "\"");
          BOOST_CHECK_MESSAGE(equal(arr2vec(machine.CheckMoves(m.first)), m.second), "Moves list does not match for piece " + name.at(p.first) + " at the position \"" + toStr(m.first) + "\"");
        }
      }
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

  BOOST_CHECK(machine->CheckStatus() == Status::invalid);
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
  testpos(white_pieces, white, *machine);

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
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 c6 2.Nc3 d5 3.Nf3 dxe4 4.Nxe4 Nf6 5.Qe2 Nbd7 6.Nd6#");
  BOOST_REQUIRE(moves.size() == 11);

  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    if (m == "dxe4") {
      const std::map<Type, Moves> white_pieces = {
        { Type::pawn,{ { a2,{ a4, a3 } },{ b2,{ b3, b4 } },{ c2,{} },{ d2,{ d4, d3 } },{ f2,{} },{ g2,{ g3, g4 } },{ h2,{ h3, h4 } } } },
        { Type::knight,{ { c3,{ b5, d5, a4, e4, b1, e2 } },{ f3,{ d4, e5, g5, h4, g1 } } } },
        { Type::bishop,{ { c1,{} },{ f1,{ e2, d3, c4, b5, a6 } } } },
        { Type::rook,{ { a1,{ b1 } },{ h1,{ g1 } } } },
        { Type::queen,{ { d1,{ e2 } } } },
        { Type::king,{ { e1,{ e2 } } } }
      };
      const std::map<Type, Moves> black_pieces = {
        { Type::pawn,{ { a7,{ a6, a5 } },{ b7,{ b6, b5 } },{ c6,{ c5 } },{ e4,{ e3, f3 } },{ e7,{ e6, e5 } },{ f7,{ f6, f5 } },{ g7,{ g6, g5 } },{ h7,{ h6, h5 } } } },
        { Type::knight,{ { b8,{ a6, d7 } },{ g8,{ h6, f6 } } } },
        { Type::bishop,{ { c8,{ d7, e6, f5, g4, h3 } },{ f8,{} } } },
        { Type::rook,{ { a8,{} },{ h8,{} } } },
        { Type::queen,{ { d8,{ d7, d6, d5, d4, d3, d2, c7, b6, a5 } } } },
        { Type::king,{ { e8,{ d7 } } } }
      };
      testpos(white_pieces, arr2vec(machine->GetSet(Set::white)), *machine);
      testpos(black_pieces, arr2vec(machine->GetSet(Set::black)), *machine);
    } else if (m == "Nd6" ) {
      BOOST_CHECK(machine->CheckStatus() == Status::checkmate);
      const std::map<Type, Moves> white_pieces = {
        { Type::pawn,{ { a2,{ a4, a3 } },{ b2,{ b3, b4 } },{ c2,{ c3, c4 } },{ d2,{ d4, d3 } },{ f2,{} },{ g2,{ g3, g4 } },{ h2,{ h3, h4 } } } },
        { Type::knight,{ { d6,{ c8, e8, b7, f7, b5, f5, c4, e4 } },{ f3,{ d4, e5, g5, h4, g1 } } } },
        { Type::bishop,{ { c1,{} },{ f1,{} } } },
        { Type::rook,{ { a1,{ b1 } },{ h1,{ g1 } } } },
        { Type::queen,{ { e2,{ e3, e4, e5, e6, e7, d3, c4, b5, a6, d1 } } } },
        { Type::king,{ { e1,{ d1 } } } }
      };
      const std::map<Type, Moves> black_pieces = {
        { Type::pawn,{ { a7,{} },{ b7,{} },{ c6,{} },{ e7,{} },{ f7,{} },{ g7,{} },{ h7,{} } } },
        { Type::knight,{ { d7,{} },{ f6,{} } } },
        { Type::bishop,{ { c8,{} },{ f8,{} } } },
        { Type::rook,{ { a8,{} },{ h8,{} } } },
        { Type::queen,{ { d8,{} } } },
        { Type::king,{ { e8,{} } } }
      };
      testpos(white_pieces, arr2vec(machine->GetSet(Set::white)), *machine);
      testpos(black_pieces, arr2vec(machine->GetSet(Set::black)), *machine);
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
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 d6 2.d4 Nd7 3.Bc4 g6 4.Nf3 Bg7 5.Bxf7+ Kxf7 6.Ng5+ Kf6 7.Qf3#");
  BOOST_REQUIRE(moves.size() == 13);
  for (auto m : moves) {
    if (m == "Bxf7") {
      BOOST_REQUIRE_MESSAGE(machine->Move("O-O"), "Can't make move O-O");
      const std::map<Type, Moves> white_pieces = {
        { Type::pawn,{ { a2,{ a4, a3 } },{ b2,{ b3, b4 } },{ c2,{ c3 } },{ d4,{ d5 } },{ e4,{ e5 } },{ f2,{} },{ g2,{ g3, g4 } },{ h2,{ h3, h4 } } } },
        { Type::knight,{ { b1,{ a3, c3, d2 } },{ f3,{ e5, g5, h4, e1, d2 } } } },
        { Type::bishop,{ { c1,{ d2, e3, f4, g5, h6 } },{ c4,{ b3, d5, e6, f7, b5, a6, d3, e2 } } } },
        { Type::rook,{ { a1,{} },{ f1,{ e1 } } } },
        { Type::queen,{ { d1,{ e1, e2, d2, d3 } } } },
        { Type::king,{ { g1,{ h1 } } } }
      };
      testpos(white_pieces, arr2vec(machine->GetSet(Set::white)), *machine);
      machine->Undo();
    }
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    if (m == "Bxf7") {
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      const std::map<Type, Moves> white_pieces = {
        { Type::pawn,{ { a2,{ a4, a3 } },{ b2,{ b3, b4 } },{ c2,{ c3, c4 } },{ d4,{ d5 } },{ e4,{ e5 } },{ f2,{} },{ g2,{ g3, g4 } },{ h2,{ h3, h4 } } } },
        { Type::knight,{ { b1,{ a3, c3, d2 } },{ f3,{ e5, g5, h4, g1, d2 } } } },
        { Type::bishop,{ { c1,{ d2, e3, f4, g5, h6 } },{ f7,{ e8, e6, d5, c4, b3, g8, g6 } } } },
        { Type::rook,{ { a1,{} },{ h1,{ g1, f1 } } } },
        { Type::queen,{ { d1,{ e2, d2, d3 } } } },
        { Type::king,{ { e1,{ f1, e2, d2, g1 /*O-O*/ } } } }
      };
      const std::map<Type, Moves> black_pieces = {
        { Type::pawn,{ { a7,{} },{ b7,{} },{ c7,{} },{ d6,{} },{ e7,{} },{ g6,{} },{ h7,{} } } },
        { Type::knight,{ { d7,{} },{ g8,{} } } },
        { Type::bishop,{ { c8,{} },{ g7,{} } } },
        { Type::rook,{ { a8,{} },{ h8,{} } } },
        { Type::queen,{ { d8,{} } } },
        { Type::king,{ { e8,{ f8, f7 } } } }
      };
      testpos(white_pieces, arr2vec(machine->GetSet(Set::white)), *machine);
      testpos(black_pieces, arr2vec(machine->GetSet(Set::black)), *machine);
    } else if (m == "Qf3") {
      BOOST_CHECK(machine->CheckStatus() == Status::checkmate);
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
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 c6 2.Nc3 d5 3.Nf3 dxe4 4.Nxe4 Bf5 5.Ng3 Bg6 6.h4 h6 7.Ne5 Bh7 8.Qh5 g6 9.Qf3 Nf6 10.Qb3 Qd5 11.Qxb7 Qxe5+ 12.Be2 Qd6 13.Qxa8 Qc7 14.a4 Bg7 15.Ra3 O-O 16.Rb3");
  BOOST_REQUIRE(moves.size() == 31);
  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    if (m == "Qxe5") {
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      const std::map<Type, Moves> white_pieces = {
        { Type::pawn,{ { a2,{} },{ b2,{} },{ c2,{} },{ d2,{} },{ f2,{} },{ g2,{} },{ h4,{} } } },
        { Type::knight,{ { g3,{ e2, e4 } } } },
        { Type::bishop,{ { c1,{} },{ f1,{ e2 } } } },
        { Type::rook,{ { a1,{} },{ h1,{} } } },
        { Type::queen,{ { b7,{} } } },
        { Type::king,{ { e1,{ d1 } } } }
      };
      const std::map<Type, Moves> black_pieces = {
        { Type::pawn,{ { a7,{ a6,a5 } },{ c6,{ c5 } },{ e7,{ e6 } },{ f7,{} },{ g6,{ g5 } },{ h6,{ h5 } } } },
        { Type::knight,{ { b8,{ a6,d7 } },{ f6,{ d7,g8,h5,g4,e4,d5 } } } },
        { Type::bishop,{ { f8,{ g7 } },{ h7,{ g8 } } } },
        { Type::rook,{ { a8,{} },{ h8,{ g8 } } } },
        { Type::queen,{ { e5,{ e6,e4,e3,e2,e1,d5,c5,b5,a5,f5,g5,h5,d6,c7,f4,g3,d4,c3,b2 } } } },
        { Type::king,{ { e8,{ d8, d7 /*because White to move, so we consider only the mobility*/ } } } }
      };
      testpos(white_pieces, arr2vec(machine->GetSet(Set::white)), *machine);
      testpos(black_pieces, arr2vec(machine->GetSet(Set::black)), *machine);
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
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 c6 2.d4 d5 3.exd5 cxd5 4.c4 Nf6 5.Nc3 Nc6 6.Bg5 Qb6 7.cxd5 Qxb2 8.Rc1 Nb4 9.Na4 Qxa2 10.Bc4 Bg4 11.Nf3 Bxf3 12.gxf3");
  BOOST_REQUIRE(moves.size() == 23);
  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
  }
}

BOOST_AUTO_TEST_CASE( HaplessQueenTest )
{
  /*
    Hapless QUEEN
    Scandinavian Defense
  */
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.e4 d5 2.exd5 Qxd5 3.Nc3 Qa5 4.d4 Nf6 5.Bd2 Nc6 6.Bb5 Bd7 7.Nd5 Qxb5 8.Nxc7+ Kd8 9.Nxb5");
  BOOST_REQUIRE(moves.size() == 8*2+1);
  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    if (m == "Bd7") {
      BOOST_REQUIRE_MESSAGE(machine->Move("Qe2"), "Can't make move Queen!");
      {
        const std::map<Type, Moves> black_pieces = {
          { Type::pawn,{ { a7,{ a6 } },{ b7,{ b6 } },{ c7,{} },{ e7,{ e6, e5 } },{ f7,{} },{ g7,{ g5, g6 } },{ h7,{ h5, h6 } } } },
          { Type::knight,{ { c6,{ b8, d8, e5, d4, b4 } },{ f6,{ g8, h5, g4, e4, d5 } } } },
          { Type::bishop,{ { d7,{ c8,e6,f5,g4,h3 } },{ f8,{} } } },
          { Type::rook,{ { a8,{ b8,c8,d8 } },{ h8,{ g8 } } } },
          { Type::queen,{ { a5,{ a6,b6,b5,b4,c3,a4,a3,a2 } } } },
          { Type::king,{ { e8,{ d8, c8 /*O-O-O*/} } } }
        };
        testpos(black_pieces, arr2vec(machine->GetSet(Set::black)), *machine);
      }
      BOOST_REQUIRE_MESSAGE(machine->Move("O-O-O"), "Can't make move O-O-O!");
      {
        const std::map<Type, Moves> black_pieces = {
          { Type::pawn,{ { a7,{ a6 } },{ b7,{ b6 } },{ c7,{} },{ e7,{ e6, e5 } },{ f7,{} },{ g7,{ g5, g6 } },{ h7,{ h5, h6 } } } },
          { Type::knight,{ { c6,{ b8, e5, d4, b4,b8 } },{ f6,{ e8,g8, h5, g4, e4, d5 } } } },
          { Type::bishop,{ { d7,{ e8,e6,f5,g4,h3 } },{ f8,{} } } },
          { Type::rook,{ { d8,{ e8 } },{ h8,{ g8 } } } },
          { Type::queen,{ { a5,{ a6,b6,b5,b4,c3,a4,a3,a2 } } } },
          { Type::king,{ { c8,{ b8 } } } }
        };
        const std::map<Type, Moves> white_pieces = {
          { Type::pawn,{ { a2,{ a3, a4 } },{ b2,{ b3,b4 } },{ c2,{} },{ d4,{ d5 } },{ f2,{ f3,f4 } },{ g2,{ g3,g4 } },{ h2,{ h3,h4 } } } },
          { Type::knight,{ { c3,{ a4,d5,e4,d1,b1 } },{ g1,{ f3,h3 } } } },
          { Type::bishop,{ { b5,{ a6,c6,c4,d3,a4 } },{ d2,{ e3,f4,g5,h6,c1 } } } },
          { Type::rook,{ { a1,{ b1,c1,d1 } },{ h1,{} } } },
          { Type::queen,{ { e2,{ d3,c4,e3,e4,e5,e6,e7,f3,g4,h5,f1,d1 } } } },
          { Type::king,{ { e1,{ d1,f1,c1 /*O-O-O*/ } } } }
        };
        testpos(white_pieces, arr2vec(machine->GetSet(Set::white)), *machine);
        testpos(black_pieces, arr2vec(machine->GetSet(Set::black)), *machine);
      }
      BOOST_REQUIRE_MESSAGE(machine->Move("O-O-O"), "Can't make move O-O-O!");
      {
        const std::map<Type, Moves> white_pieces = {
          { Type::pawn,{ { a2,{ a3, a4 } },{ b2,{ b3,b4 } },{ c2,{} },{ d4,{ d5 } },{ f2,{ f3,f4 } },{ g2,{ g3,g4 } },{ h2,{ h3,h4 } } } },
          { Type::knight,{ { c3,{ a4,d5,e4,b1 } },{ g1,{ f3,h3 } } } },
          { Type::bishop,{ { b5,{ a6,c6,c4,d3,a4 } },{ d2,{ e3,f4,g5,h6,e1 } } } },
          { Type::rook,{ { d1,{ e1,f1 } },{ h1,{} } } },
          { Type::queen,{ { e2,{ d3,c4,e3,e4,e5,e6,e7,f3,g4,h5,f1,e1 } } } },
          { Type::king,{ { c1,{ b1 } } } }
        };
        testpos(white_pieces, arr2vec(machine->GetSet(Set::white)), *machine);
      }
      machine->Undo();
      machine->Undo();
      machine->Undo();
    } else if (m == "Nxc7") {
      BOOST_CHECK(machine->CheckStatus() == Status::check);
    }
  }
}

BOOST_AUTO_TEST_CASE( HorseBetterQueenTest )
{
  /*
    HORSE IS BETTER THAN THE QUEEN
  */
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  machine->Start();

  const std::vector<std::string> moves = split("1.d4 d5 2.c4 e5 3.dxe5 d4 4.e3 Bb4+ 5.Bd2 dxe3 6.Bxb4 exf2+ 7.Ke2 fxg1=N+ 8.Rxg1 Bg4+");
  BOOST_REQUIRE(moves.size() == 8*2);
  for (auto m : moves) {
    BOOST_REQUIRE_MESSAGE(machine->Move(m.c_str()), "Can't make move " + m);
    if (m == "fxg1=N") {
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      const std::map<Type, Moves> black_pieces = {
        { Type::pawn,{ { a7,{ a6,a5 } },{ b7,{ b6,b5 } },{ c7,{ c6,c5 } },{ f7,{ f6, f5 } },{ g7,{ g5, g6 } },{ h7,{ h5, h6 } } } },
        { Type::knight,{ { b8,{ a6,c6,d7 } },{ g8,{ h6,f6,e7 } },{ g1, {e2,f3,h3} } } },
        { Type::bishop,{ { c8,{ d7,e6,f5,g4,h3 } } } },
        { Type::rook,{ { a8,{} },{ h8,{} } } },
        { Type::queen,{ { d8,{ d7,d6,d5,d4,d3,d2,d1,e7,f6,g5,h4 } } } },
        { Type::king,{ { e8,{ d7,e7,f8 /*only mobility*/ } } } }
      };
      testpos(black_pieces, arr2vec(machine->GetSet(Set::black)), *machine);
      const std::map<Type, Moves> white_pieces = {
        { Type::pawn,{ { a2,{} },{ b2,{} },{ c4,{} },{ e5, {} },{ g2,{} },{ h2,{} } } },
        { Type::knight,{ { b1,{} } } },
        { Type::bishop,{ { b4,{} },{ f1,{} } } },
        { Type::rook,{ { a1,{} },{ h1,{ g1 } } } },
        { Type::queen,{ { d1,{} } } },
        { Type::king,{ { e2,{ e1,e3,f2 } } } }
      };
      testpos(white_pieces, arr2vec(machine->GetSet(Set::white)), *machine);
      
      machine->Undo();
      BOOST_REQUIRE_MESSAGE(machine->Move("fxg1N"), "Can't make move fxg1N");
      BOOST_CHECK(machine->CheckStatus() == Status::check);
      testpos(black_pieces, arr2vec(machine->GetSet(Set::black)), *machine);
      testpos(white_pieces, arr2vec(machine->GetSet(Set::white)), *machine);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
