// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <assert.h>
#include <stdio.h>
#include <tchar.h>

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <SDL.h>
#undef main
#include <SDL_syswm.h>

#include <glm/glm.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>

#include "types.h"