#pragma once

namespace Chai {
namespace Chess {
  class UciProxy : public IEngine
  {
  public:
    UciProxy(boost::process::child&& engine, const std::string& logfile);
    ~UciProxy() override;

    bool Start(const IMachine& position, int depth) override;
    void Stop() override;
    void ProcessInfo(IInfoCall* cb) override;
    float EvalPosition(const IMachine& position) const override;

  private:
    boost::process::child childEngine;
  };
}
}