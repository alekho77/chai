// UciProxy.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "uciproxy.h"

boost::shared_ptr<Chai::Chess::IEngine> ConnectToEngine(const std::string& exefile, const std::string& logfile) {
  using namespace boost::process;
  boost::filesystem::path exe(exefile);
  if (boost::filesystem::exists(exe)) {
    boost::system::error_code ec;
    child child_engine =  execute(initializers::run_exe(exe), initializers::set_on_error(ec));
    if (!ec) {

    }
  }
  return boost::shared_ptr<Chai::Chess::IEngine>();
}

Chai::Chess::UciProxy::UciProxy(boost::process::child&& engine, const std::string & logfile)
  : childEngine(engine)
{

}

Chai::Chess::UciProxy::~UciProxy()
{
  boost::process::terminate(childEngine);
}
