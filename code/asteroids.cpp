#include "astreoids_math.h"
#include "asteroids.h"
#include "asteroids_random.h"

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
void* PushSize_(memory_arena *Arena, size_t Size)
{
    Assert(Arena->Free >= Size);
    void* Result = Arena->NextFree;
    Arena->NextFree += Size;
    Arena->Used += Size;
    Arena->Free -= Size;
    return Result;
}

internal memory_arena *InitializeArena(void *Memory, size_t WholeSize)
{
    memory_arena *MemArena = (memory_arena *)Memory;
    MemArena->WholeSize = WholeSize;
    MemArena->Used = sizeof(memory_arena);
    MemArena->NextFree = (size_t *)Memory + MemArena->Used;
    MemArena->Free = MemArena->WholeSize - MemArena->Used;
    return MemArena;
}

inline void MapToCameraSpace(win32_offscreen_buffer *Buffer, v2 *P)
{
    if(P->X > Buffer->Width)
    {
        P->X = 0;
    }
    if(P->X < 0)
    {
        P->X = Buffer->Width;
    }
    if(P->Y > Buffer->Height)
    {
        P->Y = 0;
    }
    if(P->Y < 0)
    {
        P->Y = Buffer->Height;
    }
}

//IMPORTANT: Pixel value packing is 0xAARRGGBB
//NOTE: loads bmp file as white on black not to think of memory layout of RGB values because we only have white pixels on screen
internal loaded_bitmap LoadBMPBW(char *FileName, game_memory GameMemory)
{
    loaded_bitmap Result = {};
    read_file_result ReadResult = GameMemory.ReadEntireFile(FileName);
    if(ReadResult.ContentsSize > 0)
    {
        bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
        Result.Width = Header->Width;
        Result.Height = Header->Height;
        Result.Pixels = (uint32 *)((uint8 *)ReadResult.Contents + Header->BitmapOffset);
        uint32 *Changer = (uint32 *)Result.Pixels;
        for(int Row = 0; Row < Result.Width; ++Row)
        {
            for(int Column = 0; Column < Result.Height; ++Column)
            {
                if(*Changer)
                {
                    *Changer = UINT32_MAX;
                }
                else
                {
                    //NOTE: making pixel transperant
                    *Changer = 0xff000000;
                }
                ++Changer;
            }
        }
    }
    return Result;
}

internal digit_bitmaps LoadDigitsBMP(game_memory GameMemory)
{
    digit_bitmaps Result;
    Result.Zero = LoadBMPBW("../bmp/zero.bmp", GameMemory);
    Result.One = LoadBMPBW("../bmp/one.bmp", GameMemory);
    Result.Two = LoadBMPBW("../bmp/two.bmp", GameMemory);
    Result.Three = LoadBMPBW("../bmp/three.bmp", GameMemory);
    Result.Four = LoadBMPBW("../bmp/four.bmp", GameMemory);
    Result.Five = LoadBMPBW("../bmp/five.bmp", GameMemory);
    Result.Six = LoadBMPBW("../bmp/six.bmp", GameMemory);
    Result.Seven = LoadBMPBW("../bmp/seven.bmp", GameMemory);
    Result.Eight = LoadBMPBW("../bmp/eight.bmp", GameMemory);
    Result.Nine = LoadBMPBW("../bmp/nine.bmp", GameMemory);
    return Result;
}

internal void LoadPlayerBMP(game_memory GameMemory, game_state *GameState, char *FileName)
{
    read_file_result ReadResult = GameMemory.ReadEntireFile(FileName);
    if(ReadResult.ContentsSize > 0)
    {
        bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
        uint32 *DataPointer = (uint32 *)((uint8 *)ReadResult.Contents + Header->BitmapOffset);
        //NOTE: figure out better name for this variables
        v2 *v2StorageP = GameState->Player.v2Pixels;
        for(int Column = 0; Column < Header->Height; ++Column)
        {
            for(int Row = 0; Row < Header->Width; ++Row)
            {
                if(*DataPointer == 0xffffffff)
                {
                    *v2StorageP = {(float)Header->Width/2 - Row, (float)Header->Height/2 - Column};
                    ++v2StorageP;
                    ++GameState->Player.PixelsAmmount;
                }
                ++DataPointer;
            }
        }
    }
}

internal void TransformPixelVectors(player *Player)
{
    for(int PixelIndex = 0; PixelIndex < Player->PixelsAmmount; ++PixelIndex)
    {
        RotateV2(&Player->v2Pixels[PixelIndex], Player->Angle);
    }
}

inline bool32 IsInBufferBounds(win32_offscreen_buffer *Buffer, uint32 *Dest)
{
    if(Dest >= (uint32 *)Buffer->Data && Dest < (uint32 *)Buffer->Data + Buffer->Height * Buffer->Width)
    {
        return true;
    }
    return false;
}

//TODO: fix wrapping around top-bottom borders of the screen
internal void DrawPlayer(win32_offscreen_buffer *Buffer, player *Player)
{
    uint32 *DestPlayerP = (uint32 *)Buffer->Data + (int32)Player->P.Y*Buffer->Width + (int32)Player->P.X;
    uint32 *Dest = DestPlayerP;
    for(uint32 PixelIndex = 0; PixelIndex < Player->PixelsAmmount; ++PixelIndex)
    {
        Dest = DestPlayerP;
        Dest += -(int32)Player->v2Pixels[PixelIndex].Y * Buffer->Width + (int32)Player->v2Pixels[PixelIndex].X;
        //TODO: is this IF statement efficient?
        if(IsInBufferBounds(Buffer, Dest))
        {
            *Dest = 0xFFFFFFFF;
        }
    }
}

internal void TransformAndDrawPlayer(win32_offscreen_buffer *Buffer, player *Player)
{
    player PlayerCopy = *Player;
    TransformPixelVectors(&PlayerCopy);
    DrawPlayer(Buffer, &PlayerCopy);
}

//NOTE: Bresenham circle algorythm
inline void PutPixel(win32_offscreen_buffer *Buffer, int X, int Y, uint32 PixelValue = 0xffffffff)
{
    uint32 *DataPointer = (uint32 *)Buffer->Data;
    if(Buffer->Width*Y + X < 0)
    {
        return;
    }
    if(Buffer->Width*Y + X > Buffer->Width*Buffer->Height)
    {
        return;
    }
    DataPointer += Buffer->Width*Y + X;
    *DataPointer = PixelValue;
}

internal void DrawCirclePoints(win32_offscreen_buffer *Buffer, v2 Center, int X, int Y)
{
    PutPixel(Buffer, Center.X + X, Center.Y + Y); 
    PutPixel(Buffer, Center.X - X, Center.Y + Y); 
    PutPixel(Buffer, Center.X + X, Center.Y - Y); 
    PutPixel(Buffer, Center.X - X, Center.Y - Y); 
    PutPixel(Buffer, Center.X + Y, Center.Y + X); 
    PutPixel(Buffer, Center.X - Y, Center.Y + X); 
    PutPixel(Buffer, Center.X + Y, Center.Y - X); 
    PutPixel(Buffer, Center.X - Y, Center.Y - X);
}

inline void DrawCircle(win32_offscreen_buffer *Buffer, v2 Center, int R)
{
    int D = 3 - (2*R);
    int X = 0;
    int Y = R;
    DrawCirclePoints(Buffer, Center, X, Y);
    while(X <= Y)
    {    
        ++X;
        if(D <= 0)
        {
            D = D + 4*X + 10;
        }
        else
        {
            D = D + 4*(X - Y) + 10;
            --Y;
        }
        DrawCirclePoints(Buffer, Center, X, Y);
    }
}

internal uint32 AddAsteroid(game_state *GameState, uint32 Size, v2 P, v2 dP)
{
    asteroid *Asteroid;
    Assert(GameState->AsteroidCount < ArrayCount(GameState->Asteroids));
    for(uint32 Index = 0; Index < ArrayCount(GameState->Asteroids); ++Index)
    {
        if(GameState->AsteroidsIndex[Index] == 0)
        {
            GameState->AsteroidsIndex[Index] = true;
            Asteroid = GameState->Asteroids + Index;
            ++GameState->AsteroidCount;
            Asteroid->CenterP = P;
            Asteroid->Size = Size;
            Asteroid->dP = dP;
            return Index;
        }
    }
    return -1;
}

internal void RemoveAsteroid(game_state *GameState, uint32 Index)
{
    if(GameState->AsteroidsIndex[Index])
    {
        --GameState->AsteroidCount;
        *(GameState->Asteroids + Index) = {};
        GameState->AsteroidsIndex[Index] = 0;
    }
}

//NOTE: Caller has to check for the presence of asteroid
internal void MoveAsteroid(game_state *GameState, uint32 Index, float dt)
{
    asteroid *Asteroid = GameState->Asteroids + Index;
    Asteroid->CenterP += Asteroid->dP*dt;
}

inline void DrawAsteroid(win32_offscreen_buffer *Buffer, asteroid *Asteroid)
{
    DrawCircle(Buffer, Asteroid->CenterP, Asteroid->Size);
}

internal uint32 AddProjectile(game_state *GameState, v2 P, v2 dP)
{
    projectile *Projectile;
    Assert(GameState->ProjectileCount < ArrayCount(GameState->Projectiles));
    for(uint32 Index = 0; Index < ArrayCount(GameState->Projectiles); ++Index)
    {
        if(GameState->ProjectileIndex[Index] == 0)
        {
            GameState->ProjectileIndex[Index] = true;
            Projectile = GameState->Projectiles + Index;
            ++GameState->ProjectileCount;
            Projectile->P = P;
            Projectile->dP = dP;
            return Index;
        }
    }
    return -1;
}

internal void RemoveProjectile(game_state *GameState, uint32 Index)
{
    if(GameState->ProjectileIndex[Index])
    {
        --GameState->ProjectileCount;
        GameState->ProjectileIndex[Index] = 0;
        GameState->Projectiles[Index] = {};
    }
}

internal void MoveProjectile(game_state *GameState, uint32 Index, float dt)
{
    projectile *Projectile = GameState->Projectiles + Index;
    Projectile->P += Projectile->dP*dt;

    for(uint32 AstrIndex = 0; AstrIndex < ArrayCount(GameState->Asteroids); ++AstrIndex)
    {
        if(GameState->AsteroidsIndex[AstrIndex])
        {
            asteroid* Asteroid = GameState->Asteroids + AstrIndex;
            float AsteroidProjectileDSq = LengthSq(Asteroid->CenterP - Projectile->P);
            if(AsteroidProjectileDSq <= Square(0.5f*Asteroid->Size + Asteroid->Size/2))
            {
//TODO: clean this up, remove repetetinvness
                RemoveProjectile(GameState, Index);
                uint32 AsteroidSize = Asteroid->Size;
                v2 AsteroidP = Asteroid->CenterP;
                RemoveAsteroid(GameState, AstrIndex);
                if(AsteroidSize > 16)
                {
                    int32 RandomNumber = GetRandomNumber(GameState, 20);
                    AddAsteroid(GameState, AsteroidSize/2, AsteroidP, v2{(float)RandomNumber, (float)Rerandomize(&RandomNumber, 25)});
                    AddAsteroid(GameState, AsteroidSize/2, AsteroidP, 0.77f*v2{(float)Rerandomize(&RandomNumber, 25), (float)Rerandomize(&RandomNumber, 25)});
                }
                GameState->Score += 50;
                return;
            }
        }
    }
}

inline void DrawProjectile(win32_offscreen_buffer *Buffer, projectile *Projectile)
{
    uint32 *Dest = (uint32 *)Buffer->Data + (int32)Projectile->P.Y*Buffer->Width + (int32)Projectile->P.X;
    uint32 *DestTemp = Dest;
//TODO: is IsInBufferBounds spamming effient enough?
    if(IsInBufferBounds(Buffer, DestTemp))
    {
        *DestTemp = 0xffffffff;
    }
    ++DestTemp;
    if(IsInBufferBounds(Buffer, DestTemp))
    {
        *DestTemp = 0xffffffff;
    }
    --DestTemp;
    --DestTemp;
    if(IsInBufferBounds(Buffer, DestTemp))
    {
        *DestTemp = 0xffffffff;
    }
    DestTemp = Dest + Buffer->Width;
    if(IsInBufferBounds(Buffer, DestTemp))
    {
        *DestTemp = 0xffffffff;
    }
    DestTemp = Dest - Buffer->Width;
    if(IsInBufferBounds(Buffer, DestTemp))
    {
        *DestTemp = 0xffffffff;
    }
}

inline bool32 IsProjectileInScreenBounds(win32_offscreen_buffer *Buffer, game_state *GameState, uint32 Index)
{
    projectile *Projectile = GameState->Projectiles + Index;
    if(Projectile->P.X < 0 || Projectile->P.X > Buffer->Width || Projectile->P.Y < 0 || Projectile->P.Y > Buffer->Height)
    {
        return false;
    }
    return true;
}

internal void DrawRectangle(win32_offscreen_buffer *Buffer, int32 StartX, int32 StartY, 
                            int32 EndX, int32 EndY)
{
    if(StartX < 0)
    {
        StartX = 0;
    }
    if(StartY < 0)
    {
        StartY = 0;
    }
    if(EndX >= Buffer->Width)
    {
        EndX = Buffer->Width;
    }
    if(EndY >= Buffer->Height)
    {
        EndY = Buffer->Height;
    }

    uint32 *Pixel = (uint32 *)Buffer->Data + Buffer->Width*StartY + StartX;
    for(int32 Y = StartY; Y < EndY; ++Y)
    {
        for(int32 X = StartX; X < EndX; ++X)
        {
            *Pixel = 0xFFFFFFFF;
            ++Pixel;
        }
        Pixel += Buffer->Width - (EndX - StartX);
    }
}

internal void DrawBitMap(win32_offscreen_buffer *Buffer, v2 P, loaded_bitmap *BitMap)
{
    uint32 *Source = (uint32 *)BitMap->Pixels;
    uint32 *Dest = (uint32 *)Buffer->Data;
    Dest += Buffer->Width*(uint32)P.Y + (uint32)P.X;
    for(int Row = 0; Row < BitMap->Height; ++Row)
    {
        for(int Column = 0; Column < BitMap->Width; ++Column)
        {
            if(*Source == 0xff000000)
            {
                ++Source;
                ++Dest;
                continue;
            }
            *Dest = *Source;
            ++Source;
            ++Dest;
        }
        Dest -= BitMap->Width;
        Dest += Buffer->Width;
    }
}

internal void DrawScore(win32_offscreen_buffer *Buffer, game_state *GameState)
{
    uint32 ScoreTemp = GameState->Score;

    uint32 DigitsCount = 0;
    while((ScoreTemp /= 10) != 0)
    {
        ++DigitsCount;
    }
    ++DigitsCount;

    uint32 Divider = 1;
    for(int i = 0; i < DigitsCount - 1; ++i)
    {
        Divider *= 10;
    }

    v2 StartingPoint = {0, (float)Buffer->Height - GameState->Digits.Zero.Height};
    
    for(int Digit = 0; Digit < DigitsCount; ++Digit)
    {
        DrawBitMap(Buffer, StartingPoint, &GameState->Digits.E[GameState->Score/Divider % 10]);
        StartingPoint.X += GameState->Digits.E[GameState->Score/Divider % 10].Width;
        Divider /= 10;
    }
}

internal void SpawnMoreAsteroids(game_state *GameState, uint32 Ammount, v2 BufferWidthHeight)
{
    int32 InitialSize = 0;
    v2 MaxDistanceFromPlayer = {BufferWidthHeight.X - 300, BufferWidthHeight.Y - 300};
    if(Ammount % 2)
    {
        InitialSize = 64;
    }
    else
    {
        InitialSize = 32;
    }
    int32 PRandomNumberX = GetRandomNumber(GameState, MaxDistanceFromPlayer.X);
    int32 PRandomNumberY = GetRandomNumber(GameState, MaxDistanceFromPlayer.Y);
    int32 dPRandomNumber = GetRandomNumber(GameState, 25);
    for(uint32 Index = 0; Index < Ammount; ++Index)
    {
        if(PRandomNumberX % 2)
        {
            InitialSize = 64;
        }
        else
        {
            InitialSize = 32;
        }
        AddAsteroid(GameState, InitialSize, 
                        v2{(float)PRandomNumberX, (float)PRandomNumberY}, 
                        v2{(float)dPRandomNumber, (float)Rerandomize(&dPRandomNumber, 25)});
        Rerandomize(&PRandomNumberX, MaxDistanceFromPlayer.X);
        Rerandomize(&PRandomNumberY, MaxDistanceFromPlayer.Y);
        Rerandomize(&dPRandomNumber, 20);
        Rerandomize(&InitialSize, 2);
    }
}

internal void MovePlayer(game_state *GameState, v2 ddP, float dt)
{
    player *Player = &GameState->Player;
    v2 PlayerWidthHeight = {30.0f, 30.0f};
    
    if(LengthSq(ddP) > 1.0f)
    {
        ddP = 1.0f/LengthSq(ddP) * ddP;
    }

    //NOTE: Constans are configured through deduction monkaS
    float PlayerSpeed = 7.0f;
    ddP *= PlayerSpeed;
    //NOTE: Deacceletation below
    //ddP += -0.1f*Player->dP;

    //NOTE: collision detertion is a afwul, its not 100% generic, but it works
    for(uint32 AstrIndex = 0; AstrIndex < ArrayCount(GameState->Asteroids); ++AstrIndex)
    {
        if(GameState->AsteroidsIndex[AstrIndex])
        {
            asteroid* Asteroid = GameState->Asteroids + AstrIndex;
            float AsteroidPlayerDSq = LengthSq(Asteroid->CenterP - Player->P);
            if(AsteroidPlayerDSq <= LengthSq(0.5f*PlayerWidthHeight + 0.5f*Asteroid->Size))
            {
                --Player->Hearts;
                Player->Alive = false;
                GameState->Player.TimeOfDeath = GameState->Time;
                return;
            }
        }
    }

    Player->P += 0.5f*ddP*dt*dt + Player->dP*dt;
    float MaxSpeedSq = 400.0f;
    //TODO: Figure this equation when we have ddP point to two different axises
    //Make MaxSpeedSq smaller i think
    if(LengthSq(Player->dP + ddP * dt) > MaxSpeedSq)
    {
    }
    else
    {
        Player->dP += ddP * dt;
    }
}

internal void GameUpdateAndRender(win32_offscreen_buffer *Buffer, 
                                    win32_keyboard_state *KeyBoard, game_memory GameMemory,
                                    game_state *GameState)
{
    if(!GameState->Initialized)
    {
        GameState->Initialized = true;
        GameState->Player.P.X = Buffer->Width/2;
        GameState->Player.P.Y = Buffer->Height/2;
        GameState->Player.Hearts = 3;
        GameState->Player.PixelsAmmount = 0;
        GameState->Player.Alive = false;
        GameState->Player.Angle = 0;
        GameState->Player.TimeOfDeath = 0;
        GameState->HeartBMP = LoadBMPBW("../bmp/Heart.bmp", GameMemory);
        GameState->Digits = LoadDigitsBMP(GameMemory);
        LoadPlayerBMP(GameMemory, GameState, "../bmp/player.bmp");
        GameState->Score = 0;
        GameState->LastTimeProjectileFired = 0;
    }

    uint32 BackGroundPixelValue = 0x00000000;
    
    uint32 *Pixel = (uint32 *)Buffer->Data;

    for(int Row = 0; Row < Buffer->Width; ++Row)
    {
        for(int Column = 0; Column < Buffer->Height; ++Column)
        {
            *Pixel = BackGroundPixelValue;
            ++Pixel;
        }
    }

    v2 ddP = {0.0f, 0.0f};
    if(KeyBoard->RightArrow.IsDown)
    {
        ddP.X = 1.0f;
    }
    if(KeyBoard->LeftArrow.IsDown)
    {
        ddP.X = -1.0f;
    }
    if(KeyBoard->UpArrow.IsDown)
    {
        ddP.Y = 1.0f;
    }
    if(KeyBoard->DownArrow.IsDown)
    {
        ddP.Y = -1.0f;
    }
    if(KeyBoard->A.IsDown)
    {
        GameState->Player.Angle -= 1.0f;
    }
    if(KeyBoard->D.IsDown)
    {
        GameState->Player.Angle += 1.0f;
    }
    if(KeyBoard->Space.IsDown)
    {
        if(GameState->Time - GameState->LastTimeProjectileFired > 0.2f && GameState->Player.Alive)
        {
            float ProjectileSpeed = 100.0f;
            v2 UnitVector = {0.0f, 1.0f};
            UnitVector *= ProjectileSpeed;
//TODO: figure out why projectile angle has to be negative?
            RotateV2(&UnitVector, -GameState->Player.Angle);
            AddProjectile(GameState, GameState->Player.P, UnitVector);
//NOTE: time wrapping over FLOAT_MAX?
            GameState->LastTimeProjectileFired = GameState->Time;
        }
        if(GameState->Player.Alive == false && (GameState->Time - GameState->Player.TimeOfDeath > 1.5f))
        {
            GameState->Player.Alive = true;
            GameState->Player.P.X = Buffer->Width/2;
            GameState->Player.P.Y = Buffer->Height/2;
            GameState->Player.dP = {};
        }
    }

    float dt = 0.02f;

//Loop of projectiles
    for(int Index = 0; Index < ArrayCount(GameState->Projectiles); ++Index)
    {
        if(GameState->ProjectileIndex[Index])
        {
            MoveProjectile(GameState, Index, dt);
            DrawProjectile(Buffer, &GameState->Projectiles[Index]);
            if(!IsProjectileInScreenBounds(Buffer, GameState, Index))
            {
                RemoveProjectile(GameState, Index);
            }
        }
    }

    //TODO: fix the X-ordinate drawing wrapping when drawing asteroid
    for(int Index = 0; Index < ArrayCount(GameState->Asteroids); ++Index)
    {
        if(GameState->AsteroidsIndex[Index])
        {
            MapToCameraSpace(Buffer, &((GameState->Asteroids + Index)->CenterP));
            MoveAsteroid(GameState, Index, dt);
            DrawAsteroid(Buffer, &GameState->Asteroids[Index]);
        }
        if(GameState->AsteroidCount == 0 && GameState->Time > 0.5f)
        {
            uint32 MaxAsteroidsAroundScreen = Buffer->Width / 64 + Buffer->Height / 64 - 4;
            v2 BufferWidthHeight = {(float)Buffer->Width, (float)Buffer->Height};
            SpawnMoreAsteroids(GameState, GetRandomPositiveNumber(GameState, MaxAsteroidsAroundScreen), BufferWidthHeight);
            Index = 0;
        }
    }

    if(GameState->Player.Alive)
    {    
        MapToCameraSpace(Buffer, &GameState->Player.P);
        Assert(GameState->Player.P.X <= Buffer->Width && GameState->Player.P.X >= 0);
        Assert(GameState->Player.P.Y <= Buffer->Height && GameState->Player.P.Y >= 0);
        MovePlayer(GameState, ddP, dt);
        TransformAndDrawPlayer(Buffer, &GameState->Player);
    }

    int32 TestRandomValue = GetRandomNumber(GameState, 100);

    DrawScore(Buffer, GameState);

    if(GameState->Player.Hearts > 0)
    {
        v2 RightHeartOnCreen = {(float)Buffer->Width - GameState->HeartBMP.Width, (float)Buffer->Height - GameState->HeartBMP.Height};
        for(int HeartIndex = 0; HeartIndex < GameState->Player.Hearts; ++HeartIndex)
        {
            DrawBitMap(Buffer, RightHeartOnCreen, &GameState->HeartBMP);
            RightHeartOnCreen.X -= GameState->HeartBMP.Width;
        }
    }
    else
    {
        exit(0);
    }
}