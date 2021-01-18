#pragma once

// NOTE: This is here because MSVC complains fopen isnt safe
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define VK_NO_PROTOTYPES 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#include <vulkan.h>

#include "platform.h"

//#include "ui\ui.h"

#include "render.h"
#include "render.cpp"

#include "assets.h"
#include "assets.cpp"

#include "camera.h"
#include "camera.cpp"

#include "ui\ui.h"
