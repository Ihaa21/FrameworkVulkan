#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <intrin.h>

#define VALIDATION 1

#include "platform.h"
#include "win32_main.h"

global prog_state GlobalState;

inline LARGE_INTEGER Win32GetClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);

    return Result;
}

// TODO: This should be a F64??
inline f32 Win32GetSecondsBetween(LARGE_INTEGER End, LARGE_INTEGER Start)
{
    f32 Result = ((f32)(End.QuadPart - Start.QuadPart) / (f32)GlobalState.TimerFrequency);
    return Result;
}

inline FILETIME Win32GetLastFileTime(char* FileName)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA FileData;
    if (GetFileAttributesEx(FileName, GetFileExInfoStandard, &FileData))
    {
        LastWriteTime = FileData.ftLastWriteTime;
    }

    return LastWriteTime;
}

inline void Win32LoadDemoCode(prog_demo_code* DemoCode)
{
    // NOTE: Unload the game code
    if (DemoCode->DLL)
    {
        FreeLibrary(DemoCode->DLL);
        DemoCode->DLL = 0;
    }

    DemoCode->Init = 0;
    DemoCode->MainLoop = 0;

    // NOTE: Load the game code
    b32 IsValid = false;
    for (u32 LoadTryIndex = 0; LoadTryIndex < 100 && !IsValid; ++LoadTryIndex)
    {
        WIN32_FILE_ATTRIBUTE_DATA Ignored;
    
        // NOTE: We check if the lock file exists. The lock file is there so that we
        // don't load the dll before the pdb
        if (!GetFileAttributesEx(DemoCode->LockFilePath, GetFileExInfoStandard, &Ignored))
        {
            DemoCode->LastDLLFileTime = Win32GetLastFileTime(DemoCode->SourceDLLPath);
            CopyFileA(DemoCode->SourceDLLPath, DemoCode->TempDLLPath, FALSE);
            DemoCode->DLL = LoadLibraryA(DemoCode->TempDLLPath);

            if (!DemoCode->DLL)
            {
                InvalidCodePath;
            }

            // NOTE: Load in the functions from our DLL
            DemoCode->Init = (demo_init*)GetProcAddress(DemoCode->DLL, "Init");
            DemoCode->Destroy = (demo_destroy*)GetProcAddress(DemoCode->DLL, "Destroy");
            DemoCode->SwapChainChange = (demo_swapchain_change*)GetProcAddress(DemoCode->DLL, "SwapChainChange");
            DemoCode->CodeReload = (demo_code_reload*)GetProcAddress(DemoCode->DLL, "CodeReload");
            DemoCode->MainLoop = (demo_main_loop*)GetProcAddress(DemoCode->DLL, "MainLoop");
            
            IsValid = true;
        }
                
        Sleep(100);
    }
}

internal LRESULT CALLBACK Win32MainWindowCallBack(HWND Window, UINT Message, WPARAM WParam,
                                                  LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
            GlobalState.GameIsRunning = false;
        } break;

        case WM_SIZE:
        {
            if (WParam == SIZE_MINIMIZED)
            {
                GlobalState.Minimized = true;
            }
            else
            {
                GlobalState.Minimized = false;
                RECT Rect;
                Assert(GetClientRect(Window, &Rect));
                if (GlobalState.VkInitialized)
                {
                    u32 Width = Rect.right - Rect.left;
                    u32 Height = Rect.bottom - Rect.top;
                    GlobalState.DemoCode.SwapChainChange(Width, Height);
                }
            }
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{    
    GlobalState.GameIsRunning = true;

    // NOTE: Init Memory
    {
        LPVOID BaseAddress = (LPVOID)TeraBytes(2);
        
        GlobalState.ProgramMemorySize = MegaBytes(100);    
        GlobalState.ProgramMemory = VirtualAlloc(BaseAddress, GlobalState.ProgramMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!GlobalState.ProgramMemory)
        {
            InvalidCodePath;
        }
    }
    
    //
    // NOTE: Init window and display
    //
    u32 WindowWidth = 1920;
    u32 WindowHeight = 1080;
    {
        WNDCLASSA WindowClass = {};
        WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        WindowClass.lpfnWndProc = Win32MainWindowCallBack;
        WindowClass.hInstance = hInstance;
        WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
        WindowClass.lpszClassName = "DragonClass";

        if (!RegisterClassA(&WindowClass))
        {
            InvalidCodePath;
        }
        
        GlobalState.WindowHandle = CreateWindowExA(0,
                                                   WindowClass.lpszClassName,
                                                   "FrameworkVK",
                                                   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                                   CW_USEDEFAULT,
                                                   CW_USEDEFAULT,
                                                   WindowWidth,
                                                   WindowHeight,
                                                   0,
                                                   0,
                                                   hInstance,
                                                   0);

        if (!GlobalState.WindowHandle)
        {
            InvalidCodePath;
        }

        // NOTE: Update render dimensions to match client not window
        RECT Rect;
        Assert(GetClientRect(GlobalState.WindowHandle, &Rect));
        WindowWidth = Rect.right - Rect.left;
        WindowHeight = Rect.bottom - Rect.top;
        
        GlobalState.DeviceContext = GetDC(GlobalState.WindowHandle);
    }
    
    // NOTE: Load demo code
    HMODULE VulkanLib = {};
    {
#define BUILD_PATH1(Str1, Str2, Str3) Str1 #Str2 ## Str3
#define BUILD_PATH(Str1, Str2, Str3) BUILD_PATH1(Str1, Str2, Str3)
        GlobalState.DemoCode.SourceDLLPath = BUILD_PATH("..\\build_win32\\", DLL_NAME, ".dll");
        GlobalState.DemoCode.TempDLLPath = BUILD_PATH("..\\build_win32\\", DLL_NAME, "_temp.dll");
        GlobalState.DemoCode.LockFilePath = "..\\build_win32\\lock.tmp";
        Win32LoadDemoCode(&GlobalState.DemoCode);

        // NOTE: Load vulkan lib and get instance proc addr function
        VulkanLib = LoadLibrary("vulkan-1.dll");
        if (!VulkanLib)
        {
            InvalidCodePath;
        }
        
        GlobalState.DemoCode.Init(GlobalState.ProgramMemory, GlobalState.ProgramMemorySize, VulkanLib, hInstance, WindowWidth, WindowHeight,
                                  GlobalState.WindowHandle);
        GlobalState.VkInitialized = true;
    }

    frame_input PrevInput = {};
    frame_input CurrInput = {};
        
    LARGE_INTEGER TimerFrequency;
    QueryPerformanceFrequency(&TimerFrequency);
    GlobalState.TimerFrequency = TimerFrequency.QuadPart;

    LARGE_INTEGER LastFrameTime = Win32GetClock();

    u32 FrameCounter = 0;

    while (GlobalState.GameIsRunning)
    {
        FrameCounter += 1;
        
        // NOTE: Process inputs
        {
            PrevInput = CurrInput;

            // NOTE: Reload demo code DLL if the DLL changed
            FILETIME NewDLLFileTime = Win32GetLastFileTime(GlobalState.DemoCode.SourceDLLPath);
            if (CompareFileTime(&NewDLLFileTime, &GlobalState.DemoCode.LastDLLFileTime) != 0)
            {
                Win32LoadDemoCode(&GlobalState.DemoCode);
                GlobalState.DemoCode.CodeReload(VulkanLib, GlobalState.ProgramMemory, GlobalState.ProgramMemorySize);
            }
        
            MSG Message;
            while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
            {
                switch (Message.message)
                {
                    case WM_QUIT:
                    {
                        GlobalState.GameIsRunning = false;
                    } break;

                    case WM_SYSKEYDOWN:
                    case WM_SYSKEYUP:
                    case WM_KEYDOWN:
                    case WM_KEYUP:
                    {
                        u32 VKCode = (u32)Message.wParam;
                        b32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                        b32 IsDown = ((Message.lParam & (1 << 31)) == 0);

                        if (VKCode >= 0 && VKCode <= 255)
                        {
                            CurrInput.KeysDown[VKCode] = IsDown;
                        }
                    } break;
                    
                    default:
                    {
                        TranslateMessage(&Message);
                        DispatchMessageA(&Message);
                    }
                }
            }

            // NOTE: Mouse Input
            if (GetActiveWindow() == GlobalState.WindowHandle)
            {
                // TODO: Values don't make sense on second monitor
                POINT Win32MousePos;
                Assert(GetCursorPos(&Win32MousePos));
                Assert(ScreenToClient(GlobalState.WindowHandle, &Win32MousePos));
                CurrInput.MousePixelPos = V2i(Win32MousePos.x, Win32MousePos.y);

                RECT ClientRect;
                GetClientRect(GlobalState.WindowHandle, &ClientRect);

                // NOTE: Make the origin be the bottom left corner
                CurrInput.MousePixelPos.y = ClientRect.bottom - CurrInput.MousePixelPos.y;
                
                CurrInput.MouseNormalizedPos.x = f32(CurrInput.MousePixelPos.x) / (f32)(ClientRect.right - ClientRect.left);
                CurrInput.MouseNormalizedPos.y = f32(CurrInput.MousePixelPos.y) / (f32)(ClientRect.top - ClientRect.bottom);
                
                CurrInput.MouseDown = (GetKeyState(VK_LBUTTON) & 0x80) != 0;

                /*
                DebugPrintLog("Mouse: %i, %i, %i, %i, %f, %f\n",
                              Win32MousePos.x, Win32MousePos.y,
                              CurrInput.MousePixelPos.x, CurrInput.MousePixelPos.y,
                              CurrInput.MouseNormalizedPos.x, CurrInput.MouseNormalizedPos.y);
                */
            }
            else
            {
                CurrInput = {};
            }
        }

        if (!GlobalState.Minimized)
        {
            GlobalState.DemoCode.MainLoop(&PrevInput, &CurrInput, 1.0f / 60.0f);
        }
    }

    GlobalState.DemoCode.Destroy();
    
    return 0;
}
