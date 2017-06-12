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

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "machinetestutils.h"
#include "../chessmachine/machine.h"