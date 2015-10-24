#include "stdafx.h"

namespace Chai {
  namespace Chess {
    CHESSBOARD;

    template <class T>
    class watcher {
     public:
      watcher(const T& l) : list(l) {}
      bool contains(Type type) {
        return std::find_if(list.begin(), list.end(), [type](const auto& p) { return p.type == type; }) != list.end();
      }
      bool at(Postion pos, Type type) {
        return std::find_if(list.begin(), list.end(), [pos, type](const auto& p) { return p.type == type && p.position == pos; }) != list.end();
      }
      typename T::difference_type count(Set set) {
        return std::count_if(list.begin(), list.end(), [set](const auto& p) {return p.set == set;});
      }
      typename T::difference_type count(Set set, Type type) {
        return std::count_if(list.begin(), list.end(), [set, type](const auto& p) {return p.set == set && p.type == type;});
      }
    private:
      const T& list;
    };

    template <class T>
    bool contains(const T& list, Type type) { return watcher<T>(list).contains(type); }
    template <class T>
    bool find(const T& list, Postion pos, Type type) { return watcher<T>(list).at(pos, type); }
    template <class T>
    typename T::difference_type count(const T& list, Set set) { return watcher<T>(list).count(set); }
    template <class T>
    typename T::difference_type count(const T& list, Set set, Type type) { return watcher<T>(list).count(set, type); }
  }
}


BOOST_AUTO_TEST_SUITE ( ChessMachineTest )

BOOST_AUTO_TEST_CASE( ConstructorTest )  
{
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
  
  Snapshot shot;
  memset(&shot, 0xee, sizeof(shot));
  BOOST_CHECK(!machine->GetSnapshot(shot));

  BOOST_CHECK(_countof(shot.pieces) == 32);
  for (const Snapshot::Piece& p: shot.pieces)
  {
    BOOST_CHECK(p.set == Set::unknown);
    BOOST_CHECK(p.type == Type::bad);
    BOOST_CHECK(p.position == BADPOS);

    for (const Snapshot::Field& f : p.moves)
    {
      BOOST_CHECK(!f.threat);
      BOOST_CHECK(f.postion == BADPOS);
    }
  }
}

BOOST_AUTO_TEST_CASE( StartTest )
{
  using namespace Chai::Chess;
  boost::shared_ptr<IChessMachine> machine(CreateChessMachine(), DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");

  machine->Start();

  Snapshot shot;
  memset(&shot, 0xee, sizeof(shot));
  BOOST_CHECK(machine->GetSnapshot(shot));

  const std::vector<Snapshot::Piece> pieces(shot.pieces, shot.pieces + _countof(shot.pieces));

  BOOST_REQUIRE(!contains(pieces, Type::bad));
  
  BOOST_CHECK(count(pieces, Set::white) == 16);
  BOOST_CHECK(count(pieces, Set::white, Type::pawn) == 8);
  BOOST_CHECK(count(pieces, Set::white, Type::knight) == 2);
  BOOST_CHECK(count(pieces, Set::white, Type::bishop) == 2);
  BOOST_CHECK(count(pieces, Set::white, Type::rook) == 2);
  BOOST_CHECK(count(pieces, Set::white, Type::queen) == 1);
  BOOST_CHECK(count(pieces, Set::white, Type::king) == 1);
  
  BOOST_CHECK(count(pieces, Set::black) == 16);
  BOOST_CHECK(count(pieces, Set::black, Type::pawn) == 8);
  BOOST_CHECK(count(pieces, Set::black, Type::knight) == 2);
  BOOST_CHECK(count(pieces, Set::black, Type::bishop) == 2);
  BOOST_CHECK(count(pieces, Set::black, Type::rook) == 2);
  BOOST_CHECK(count(pieces, Set::black, Type::queen) == 1);
  BOOST_CHECK(count(pieces, Set::black, Type::king) == 1);
}

BOOST_AUTO_TEST_SUITE_END()
