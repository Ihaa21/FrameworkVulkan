#pragma once
//
// NOTE: Prog State
//

struct prog_demo_code
{
    char* SourceDLLPath;
    char* TempDLLPath;
    char* LockFilePath;
    HMODULE DLL;
    FILETIME LastDLLFileTime;

    demo_init* Init;
    demo_destroy* Destroy;
    demo_swapchain_change* SwapChainChange;
    demo_code_reload* CodeReload;
    demo_main_loop* MainLoop;
};

struct prog_state
{
    b32 VkInitialized;
    
    mm ProgramMemorySize;
    void* ProgramMemory;
    
    b32 GameIsRunning;
    HDC DeviceContext;
    i64 TimerFrequency;
    prog_demo_code DemoCode;
    
    // NOTE: Window data
    HWND WindowHandle;
};
