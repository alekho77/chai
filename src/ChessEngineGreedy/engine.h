#pragma once
#include "../chessmachine/chessmachine.h"

namespace Chai {
namespace Chess {

class GreedyEngine : public IEngine
{
public:
  GreedyEngine();
  //virtual ~GreedyEngine();

  bool Start(const IInfoCall* cb, const IMachine& position, int depth, int timeout) override;
  void Stop() override;
};

}
}

