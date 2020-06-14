#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <dsound.h>

#include "win32_asteroids.h"
#include "asteroids.cpp"

global_variable bool32 Running;

global_variable win32_offscreen_buffer BackBuffer = {};
global_variable win32_keyboard_state KeyBoard = {};
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer = {};

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal BITMAPINFO Win32InitializeBitmapHeader(int32 Width, int32 Height)
{
    BITMAPINFO Result = {};

    Result.bmiHeader.biSize = sizeof(Result.bmiHeader);
    Result.bmiHeader.biWidth = Width;
    Result.bmiHeader.biHeight = Height;
    Result.bmiHeader.biPlanes = 1;
    Result.bmiHeader.biBitCount = 32;
    Result.bmiHeader.biCompression = BI_RGB;

    return Result;
}

inline win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    RECT Win32Rect;
    GetWindowRect(Window, &Win32Rect);
    Result.Width = Win32Rect.right - Win32Rect.left;
    Result.Height = Win32Rect.bottom - Win32Rect.top;

    return Result;
}

internal void Win32InitializeBackBuffer(HDC DeviceContext, win32_offscreen_buffer *Buffer,
                                        int32 Width, int32 Height)
{
    Buffer->DeviceContext = DeviceContext;
    Buffer->bmiHeader = Win32InitializeBitmapHeader(Width, Height);
    Buffer->Width = Width;
    Buffer->Height = Height;
    if(Buffer->Initialized == false)
    {
        Buffer->Data = VirtualAlloc(0, 4*Width*Height, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    }
    Buffer->Initialized = true;
    Assert(Buffer->Data);
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int32 Width, int32 Height)
{
    VirtualFree(Buffer->Data, 4*Buffer->Width*Buffer->Height, MEM_RELEASE);
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->Data = VirtualAlloc(0, 4*Buffer->Width*Buffer->Height,
                                MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    Buffer->bmiHeader = Win32InitializeBitmapHeader(Width, Height);
    CreateDIBitmap(Buffer->DeviceContext, &Buffer->bmiHeader.bmiHeader, 0, 0,
                    &Buffer->bmiHeader, DIB_RGB_COLORS);
}

internal void Win32StretchDIBits(win32_offscreen_buffer *Buffer)
{
    StretchDIBits(Buffer->DeviceContext, 0, 0, Buffer->Width, Buffer->Height,
                                0, 0, Buffer->Width, Buffer->Height, Buffer->Data,
                                &Buffer->bmiHeader, DIB_RGB_COLORS, SRCCOPY);
}

//NOTE: The way you call functions from platform layer is by adding their pointers to game_memory structure
internal read_file_result Win32ReadEntireFile(char *FileName)
{
    read_file_result Result = {};

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            DWORD BytesRead;
            Result.Contents = VirtualAlloc(0, FileSize.QuadPart, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
            ReadFile(FileHandle, Result.Contents, FileSize.QuadPart, &BytesRead, 0);
            if(BytesRead == FileSize.QuadPart)
            {
                Result.ContentsSize = BytesRead;
            }
        }
    }
    else
    {
        Assert("File wasn't opened");
    }
    CloseHandle(FileHandle);

    return Result;
}

LRESULT CALLBACK Win32MainWindowCallBack(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    HDC DeviceContext = GetDC(Window);

    switch(Message)
    {
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int32 Width = ClientRect.right - ClientRect.left;
            int32 Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(&BackBuffer, Width, Height);
            OutputDebugStringA("Success?\n");
        }break;
        case WM_DESTROY:
        {
            Running = false;
        }break;
        case WM_CLOSE:
        {
            Running = false;
        }break;
        case WM_ACTIVATEAPP:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int32 Width = ClientRect.right - ClientRect.left;
            int32 Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(&BackBuffer, Width, Height);
            Win32InitializeBackBuffer(DeviceContext, &BackBuffer, Width, Height);
        }break;
        case WM_KEYDOWN:
        {
            OutputDebugStringA("KeyStroke\n");
            if(WParam == VK_LEFT)
            {
                KeyBoard.LeftArrow.IsDown = true;
                KeyBoard.LeftArrow.WasPressed = true;
            }
            if(WParam == VK_RIGHT)
            {
                KeyBoard.RightArrow.IsDown = true;
                KeyBoard.RightArrow.WasPressed = true;
            }
            if(WParam == VK_UP)
            {
                KeyBoard.UpArrow.IsDown = true;
                KeyBoard.UpArrow.WasPressed = true;
            }
            if(WParam == VK_DOWN)
            {
                KeyBoard.DownArrow.IsDown = true;
                KeyBoard.DownArrow.WasPressed = true;
            }
            if(WParam == VK_RETURN)
            {
                KeyBoard.LeftArrow.IsDown = true;
                KeyBoard.LeftArrow.WasPressed = true;
            }
            if(WParam == 'A')
            {
                KeyBoard.A.IsDown = true;
                KeyBoard.A.WasPressed = true;
            }
            if(WParam == 'D')
            {
                KeyBoard.D.IsDown = true;
                KeyBoard.D.WasPressed = true;
            }
            if(WParam == VK_SPACE)
            {
                KeyBoard.Space.IsDown = true;
                KeyBoard.Space.WasPressed = true;
            }
        }break;
        case WM_KEYUP:
        {
            OutputDebugStringA("KeyStroke\n");
            if(WParam == VK_LEFT)
            {
                KeyBoard.LeftArrow.IsDown = false;
            }
            if(WParam == VK_RIGHT)
            {
                KeyBoard.RightArrow.IsDown = false;
            }
            if(WParam == VK_UP)
            {
                KeyBoard.UpArrow.IsDown = false;
            }
            if(WParam == VK_DOWN)
            {
                KeyBoard.DownArrow.IsDown = false;
            }
            if(WParam == VK_RETURN)
            {
                KeyBoard.LeftArrow.IsDown = false;
            }
            if(WParam == 'A')
            {
                KeyBoard.A.IsDown = false;
            }
            if(WParam == 'D')
            {
                KeyBoard.D.IsDown = false;
            }        
            if(WParam == VK_SPACE)
            {
                KeyBoard.Space.IsDown = false;
            }
        }break;

        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);

        }break;
    }

    return Result;
}

int WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCode)
{
    LARGE_INTEGER CyclesPerSecondResult;
    QueryPerformanceFrequency(&CyclesPerSecondResult);
    uint64 CyclesPerSecond = CyclesPerSecondResult.QuadPart;

    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallBack;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "AsteroidsWindowClass";
    Assert(RegisterClassA(&WindowClass));

    HWND WindowHandle = CreateWindowExA(0, "AsteroidsWindowClass", "Asteroids",
                                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    0, 0, Instance, 0);

    HDC DeviceContext = GetDC(WindowHandle);

    uint32 GameMemorySize = Megabyte;
    void* GameMemory = VirtualAlloc(0, Megabyte, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    memory_arena *GameMemoryArena = InitializeArena(GameMemory, GameMemorySize);
//TODO: Move GameState to purely game code
    game_state *GameState = PushStruct(GameMemoryArena, game_state);
    game_memory GameMemoryStruct = {};
    GameMemoryStruct.ReadEntireFile = Win32ReadEntireFile;
    GameMemoryStruct.MemoryArena = GameMemoryArena;

    if(WindowHandle)
    {
        Running = true;
        MSG Message;

        LARGE_INTEGER StartCounter;
        QueryPerformanceCounter(&StartCounter);
        while(Running)
        {
            while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
            {
                if(Message.message == WM_QUIT)
                {
                    Running = false;
                }

                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }

            GameUpdateAndRender(&BackBuffer, &KeyBoard, GameMemoryStruct, GameState);

            Win32StretchDIBits(&BackBuffer);

//NOTE: changing only Space button because its the only one that needs it
            KeyBoard.Space.WasPressed = false;

            LARGE_INTEGER EndCounter;
            QueryPerformanceCounter(&EndCounter);

            uint64 CyclesElapsed = EndCounter.QuadPart - StartCounter.QuadPart;
            float SecondsElapsed = (float)CyclesElapsed / (float)CyclesPerSecond;

            char TextBuffer[256];
            sprintf(TextBuffer, "SecondsElapsed: %f\n", SecondsElapsed);

            OutputDebugStringA(TextBuffer);

            GameState->Time += SecondsElapsed;

            StartCounter = EndCounter;
        }
    }
}