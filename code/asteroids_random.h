#if !defined(ASTR_RANDOM)

internal int32 GetRandomNumber(game_state *GameState, uint32 Range)
{
    uint32 Multiplier = 1;
    uint32 DigitsCount = 0;
    uint32 RangeTemp = Range;
    while((RangeTemp /= 10) != 0)
    {
        Multiplier *= 10;
    }
    Multiplier *= 10;

    int32 RandomValue = ((int32)(GameState->Time * 1000000 / Multiplier) % Range);
    if((int32)RandomValue % 2)
    {
        RandomValue *= -1;
    }
    return RandomValue;
}

internal uint32 GetRandomPositiveNumber(game_state *GameState, uint32 Range)
{
    uint32 Multiplier = 1;
    uint32 DigitsCount = 0;
    uint32 RangeTemp = Range;
    while((RangeTemp /= 10) != 0)
    {
        Multiplier *= 10;
    }
    Multiplier *= 10;

    uint32 RandomValue = ((int32)(GameState->Time * 1000000 / Multiplier) % Range);
    return RandomValue;
}

internal int32 Rerandomize(int32 *Value, int32 Range)
{
    *Value = (17*(*Value) + 97) % Range;
    if(*Value % 2 == 0)
    {
        *Value *= -1;
    }

    return *Value;
}

#define ASTR_RANDOM
#endif