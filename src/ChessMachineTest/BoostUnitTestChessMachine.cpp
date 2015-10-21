#include "stdafx.h"
#include "../chessmachine/chessmachine.h"
#include <boost/shared_ptr.hpp>

BOOST_AUTO_TEST_SUITE ( ChessMachineTest )

BOOST_AUTO_TEST_CASE( ConstructorTest )  
{
  boost::shared_ptr<Chai::Chess::IChessMachine> machine(Chai::Chess::CreateChessMachine(), Chai::Chess::DeleteChessMachine);
  BOOST_REQUIRE_MESSAGE(machine, "Can't create ChessMachine!");
}

BOOST_AUTO_TEST_SUITE_END()
