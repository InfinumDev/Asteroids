#if !defined(ASTR_H)

struct player
{
    //NOTE: P is the center of the player square
    v2 P;
    v2 dP;
    uint32 Hearts;
    bool32 Alive;
    v2 v2Pixels[512];
    uint32 PixelsAmmount;
    float Angle;
    float TimeOfDeath;
};

struct projectile
{
    v2 P;
    v2 dP;
};

struct asteroid
{
    uint32 Size;
    v2 CenterP;
    v2 dP;
};

struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    void* Pixels;
};

union digit_bitmaps
{
    //TODO: do i need this nameless struct?
    struct
    {
        loaded_bitmap Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine;
    };

    loaded_bitmap E[10];
};

//TODO: is this good practise to make theese arrays of objects and indecies?
struct game_state
{
    bool32 Initialized;
    player Player;

    asteroid Asteroids[256];
    uint32 AsteroidsIndex[256];
    uint32 AsteroidCount;
    //NOTE: Time in seconds
    float Time;
    loaded_bitmap HeartBMP;
    digit_bitmaps Digits;

    uint32 Score;

    projectile Projectiles[45];
    uint32 ProjectileIndex[45];
    uint32 ProjectileCount;
    float LastTimeProjectileFired;
};

#pragma pack(push, 1)
struct bitmap_header
{
    uint16 FileType; 
	uint32 FileSize;     
	uint16 Reserved1;    
	uint16 Reserved2;    
	uint32 BitmapOffset;  
    uint32 Size;             
	int32 Width;
	int32 Height;
	uint16 Planes;
	uint16 BitsPerPixel;
    uint32 Compression;
    uint32 SizeOfBitMap;
    int32 HorzResolution;
    int32 VertResolution;
    uint32 ColorsUsed;
    uint32 ColorImportant;

    uint8 RedMask;
    uint8 GreenMask;
    uint8 BlueMask;
};
#pragma pack(pop)

#define ASTR_H
#endif