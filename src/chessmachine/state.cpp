#include "stdafx.h"
#include "state.h"

namespace Chai {
  namespace Chess {

    ChessState::ChessState()
      : whitePieces({ {Type::pawn, 0x01}, {Type::pawn,   0x11}, {Type::pawn,   0x21}, {Type::pawn,  0x31}, {Type::pawn, 0x41}, {Type::pawn,   0x51}, {Type::pawn,   0x61}, {Type::pawn, 0x71},
                      {Type::rook, 0x00}, {Type::knight, 0x10}, {Type::bishop, 0x20}, {Type::queen, 0x30}, {Type::king, 0x40}, {Type::bishop, 0x50}, {Type::knight, 0x60}, {Type::rook, 0x70} })
      , blackPieces({ {Type::pawn, 0x06}, {Type::pawn,   0x16}, {Type::pawn,   0x26}, {Type::pawn,  0x36}, {Type::pawn, 0x46}, {Type::pawn,   0x56}, {Type::pawn,   0x66}, {Type::pawn, 0x76},
                      {Type::rook, 0x07}, {Type::knight, 0x17}, {Type::bishop, 0x27}, {Type::queen, 0x37}, {Type::king, 0x47}, {Type::bishop, 0x57}, {Type::knight, 0x67}, {Type::rook, 0x77} })
    {
    }

  }
}