// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
// Windows Header Files:
#include <windows.h>

// TODO: reference additional headers your program requires here
#include <list>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <assert.h>

#include <boost/optional.hpp>

#define BOOST_REGEX_DYN_LINK
#include <boost/regex.hpp>
