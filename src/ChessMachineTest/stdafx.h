// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WIN32
    #include "targetver.h"
#endif

#include <stdlib.h>
#include <memory>
#include <vector>
#include <set>
#include <algorithm>

#include <boost/shared_ptr.hpp>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../chessmachine/chessmachine.h"