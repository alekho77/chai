// chessmachine.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "chessmachine.h"
#include "machine.h"

CHESSMACHINE_API Chai::Chess::IChessMachine* Chai::Chess::CreateChessMachine()
{
  return new ChessMachine();
}

CHESSMACHINE_API void Chai::Chess::DeleteChessMachine(Chai::Chess::IChessMachine* ptr)
{
  delete ptr;
}
