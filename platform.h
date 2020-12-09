#pragma once

#include <windows.h>
#include "math\math.h"
#include "memory\memory.h"
#include "string\string.h"

struct frame_input
{
    // NOTE: Mouse input
    b32 MouseDown;
    v2i MousePixelPos;
    v2 MouseNormalizedPos;
    f32 MouseScroll;

    // NOTE: Keyboard input
    b32 KeysDown[255];
};

#define DEMO_INIT(name) void name(void* ProgramMemory, mm ProgramMemorySize, HMODULE VulkanLib, HINSTANCE hInstance, u32 WindowWidth, u32 WindowHeight, HWND WindowHandle)
typedef DEMO_INIT(demo_init);

#define DEMO_DESTROY(name) void name()
typedef DEMO_DESTROY(demo_destroy);

#define DEMO_CODE_RELOAD(name) void name(HMODULE VulkanLib, void* ProgramMemory, mm ProgramMemorySize)
typedef DEMO_CODE_RELOAD(demo_code_reload);

#define DEMO_MAIN_LOOP(name) void name(frame_input* PrevInput, frame_input* CurrInput, f32 FrameTime)
typedef DEMO_MAIN_LOOP(demo_main_loop);
