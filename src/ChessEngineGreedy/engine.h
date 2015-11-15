#pragma once
#include "../chessmachine/chessmachine.h"

namespace Chai {
namespace Chess {

class GreedyEngine : public IEngine
{
public:
  GreedyEngine();
  //virtual ~GreedyEngine();

  bool Start(const IMachine& position, int depth, int timeout) override;
  void Stop() override;
  void ProcessInfo(const IInfoCall* cb) override;

private:
  boost::shared_ptr<IMachine> chessMachine;
  boost::asio::io_service service;
};

}
}

