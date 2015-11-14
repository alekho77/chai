#pragma once
#include "../chessmachine/chessmachine.h"

#ifdef CHESSENGINEGREEDY_EXPORTS
#define CHESSENGINEGREEDY_API __declspec(dllexport)
#else
#define CHESSENGINEGREEDY_API __declspec(dllimport)
#endif

namespace Chai {
  namespace Chess {
    CHESSENGINEGREEDY_API IMachine* CreateGreedyEngine(const IInfoCall* cb = nullptr);
    CHESSENGINEGREEDY_API void DeleteGreedyEngine(IMachine* ptr);
  }
}