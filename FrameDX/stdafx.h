// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <cinttypes>
#include <string>
#include <functional>
#include <ppl.h>
#include <concurrent_vector.h>
#include <chrono>
#include <ctime>
#include <iomanip>

using namespace std;