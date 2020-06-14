#if !defined(WIN32ASTR)

#define Assert(Expresion) if(!(Expresion)) {int *Ass = 0; *Ass = 0;}
#define ArrayCount(Array) sizeof(Array) / sizeof(Array[0])

typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int32 bool32;
typedef int8_t int8;
typedef uint8_t uint8;

#define local_persist static
#define global_variable static
#define internal static

#define Megabyte 1000000

struct memory_arena
{
    size_t WholeSize;
    size_t Used;
    size_t Free;
    size_t *NextFree;
};

struct win32_offscreen_buffer
{
    HDC DeviceContext;
    BITMAPINFO bmiHeader;
    int32 Width;
    int32 Height;
    void *Data;
    bool32 Initialized;
};

struct win32_window_dimension
{
    int32 Width;
    int32 Height;
};

struct button_state
{
    bool32 IsDown;
    bool32 WasPressed;
};

//TODO: not bool32 but WasDown IsDown struct
struct win32_keyboard_state
{
    button_state LeftArrow;
    button_state RightArrow;
    button_state UpArrow;
    button_state DownArrow;
    button_state Enter;
    button_state Space;
    button_state A;
    button_state D;
};

typedef struct read_file_result
{
    uint32 ContentsSize;
    void *Contents;
} read_file_result;

struct game_memory
{
    memory_arena *MemoryArena;
    read_file_result (*ReadEntireFile)(char *FileName);
};

#define WIN32ASTR_H
#endif