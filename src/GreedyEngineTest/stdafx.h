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
#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/tuple/tuple.hpp>

#include "chessmachine.h"
#include "machinetestutils.h"
