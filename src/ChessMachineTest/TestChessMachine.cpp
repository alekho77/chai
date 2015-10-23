#include "stdafx.h"

namespace Chai {
  namespace Chess {
    CHESSBOARD;

    template <class T>
    class watcher {
     public:
      watcher(const T& l) : list(l) {}
      bool contains(Type type) {
        return std::find_if(list.begin(), list.end(), [type](const Snapshot::Piece& p) { return p.type == type; }) != list.end();
      }
      bool at(Postion pos, Type type) {
        return std::find_if(list.begin(), list.end(), [pos, type](const Snapshot::Piece& p) { return p.type == type && p.position == pos; }) != list.end();
      }
     private:
      const T& list;
    };

    template <class T>
    bool find(const T& list, Type type) { return watcher<T>(list).contains(type); }
    template <class T>
    bool find(const T& list, Postion pos, Type type) { return watcher<T>(list).at(pos, type); }
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

  BOOST_REQUIRE(_countof(shot.whitePieces) == _countof(shot.blackPieces));
  for (int i = 0; i < _countof(shot.whitePieces); i++)
  {
    BOOST_CHECK(shot.whitePieces[i].type == Type::bad);
    BOOST_CHECK(shot.blackPieces[i].type == Type::bad);

    BOOST_CHECK(shot.whitePieces[i].position == BADPOS);
    BOOST_CHECK(shot.blackPieces[i].position == BADPOS);

    for (int m = 0; m < _countof(Snapshot::Piece::moves); m++)
    {
      BOOST_CHECK(!shot.whitePieces[i].moves[m].threat);
      BOOST_CHECK(!shot.blackPieces[i].moves[m].threat);

      BOOST_CHECK(shot.whitePieces[i].moves[m].postion == BADPOS);
      BOOST_CHECK(shot.blackPieces[i].moves[m].postion == BADPOS);
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

  const std::vector<Snapshot::Piece> white(shot.whitePieces, shot.whitePieces + _countof(shot.whitePieces));
  const std::vector<Snapshot::Piece> black(shot.blackPieces, shot.blackPieces + _countof(shot.blackPieces));

  BOOST_CHECK(!find(white, Type::bad));

  //BOOST_CHECK();
}

BOOST_AUTO_TEST_SUITE_END()
