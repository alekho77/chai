#include "stdafx.h"

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

    const Postion badpos = {0, 0};
    BOOST_CHECK(shot.whitePieces[i].position.file == badpos.file && shot.whitePieces[i].position.rank == badpos.rank);
    BOOST_CHECK(shot.blackPieces[i].position.file == badpos.file && shot.blackPieces[i].position.rank == badpos.rank);

    for (int m = 0; m < _countof(Snapshot::Piece::moves); m++)
    {
      BOOST_CHECK(!shot.whitePieces[i].moves[m].threat);
      BOOST_CHECK(!shot.blackPieces[i].moves[m].threat);

      BOOST_CHECK(shot.whitePieces[i].moves[m].postion.file == badpos.file && shot.whitePieces[i].moves[m].postion.rank == badpos.rank);
      BOOST_CHECK(shot.blackPieces[i].moves[m].postion.file == badpos.file && shot.blackPieces[i].moves[m].postion.rank == badpos.rank);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
