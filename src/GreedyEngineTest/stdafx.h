// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WIN32
    #include "targetver.h"
#endif

#define BOOST_ALL_DYN_LINK
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/container/static_vector.hpp>

#include "../ChessEngineGreedy/greedyengine.h"
#include "../ChessMachineTest/machinetestutils.h"
