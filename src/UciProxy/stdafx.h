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

#define BOOST_ALL_DYN_LINK
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/regex.hpp>

#include "chessmachine.h"
