#if !defined(ASTR_MATH)
#include <math.h>
#define PI 3.1415926535f

typedef struct v2
{
    float X;
    float Y;
}v2;

inline v2 operator*(v2 A, float B)
{
    v2 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return Result;
}

inline v2 operator*(float B, v2 A)
{
    v2 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return Result;
}

//NOTE: No real math operation. Made for convenience
inline v2 operator+(v2 A, float B)
{
    v2 Result;
    Result.X = A.X + B;
    Result.Y = A.Y + B;
    return Result;
}

inline v2 operator+(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return Result;
}

inline v2 operator-(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return Result;
}

inline v2 & operator+=(v2 &A, v2 B)
{
    A = A + B;
    return A;
}

inline v2 & operator*=(v2 &A, float B)
{
    A.X = A.X * B;
    A.Y = A.Y * B;
    return A;
}

inline v2 operator-(v2 A)
{
    v2 Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    return Result;
}

inline float Square(float A)
{
    float Result;
    Result = A*A;
    return Result;
}

inline float LengthSq(v2 A)
{
    float Result;
    Result = Square(A.X) + Square(A.Y);
    return Result;
}

inline float Sqrt(float A)
{
    float Result;
    Result = sqrtf(A);
    return Result;
}

inline v2 operator/(float B, v2 A)
{
    v2 Result;
    Result.X = B / A.X;
    Result.Y = B / A.Y;
    return Result;
}

inline void RotateV2(v2 *Vec, int Angle)
{
    float Radians = Angle * PI / 180.0f;
    v2 Result = *Vec;
    Result.X = cosf(Radians)*Vec->X - sinf(Radians)*Vec->Y;
    Result.Y = sinf(Radians)*Vec->X + cosf(Radians)*Vec->Y;
    *Vec = Result;
}

#define ASTR_MATH
#endif