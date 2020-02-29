#pragma once

struct V2
{
	f32 x, y;
	f32 &operator[](int i);
};

struct V2s
{
	s32 x, y;
};

struct V2u
{
	u32 x, y;
};

struct V3
{
	f32 x, y, z;
	f32 operator[](int i) const;
	f32 &operator[](int i);
};

struct V4
{
	f32 x, y, z, w;
	f32 operator[](int i) const;
	f32 &operator[](int i);
};

struct M3
{
	f32 m[3][3];
	V3 &operator[](int i);
};

struct M4
{
	f32 m[4][4];
	V4 &operator[](int i);
};

struct Quaternion {
	/*
	Quaternion() {}
	Quaternion(f32 x, f32 y, f32 z, f32 w) : x{x}, y{y}, z{z}, w{w} {}
	Quaternion(V3 v) : x{v.x}, y{v.y}, z{v.z}, w{0.0f} {}
	Quaternion(V3 axis, f32 angle) : im{sin(angle/2.0f)*axis}, w{cos(angle/2.0f)} {}
	*/
	f32 x, y, z, w;
};

#define PI 3.14159265358979323846264338327950288
constexpr f32 DEGREES_TO_RADIANS_MULTIPLIER = PI / 180.0;
constexpr f32 RADIANS_TO_DEGREES_MULTIPLIER = 180.0 / PI;

constexpr f32 DegreesToRadians(f32 degrees)
{
	return degrees * DEGREES_TO_RADIANS_MULTIPLIER;
}

constexpr f32 RadiansToDegrees(f32 radians)
{
	return radians * RADIANS_TO_DEGREES_MULTIPLIER;
}

template <typename T>
T Minimum(T a, T b)
{
	if (a < b)
	{
		return a;
	}
	return b;
}

template <typename T>
T Maximum(T a, T b)
{
	if (a > b)
	{
		return a;
	}
	return b;
}

V3 operator*(f32 s, V3 v);
V3 &operator+=(V3 &a, V3 b);
V3 &operator-=(V3 &a, V3 b);

u32 DivideAndRoundUp(u32 a, u32 b);
bool NotNAN(V3 v);
bool NotZero(V3 v);
V4 V3ToV4(V3 v, f32 w);
M4 IdentityMatrix();
f32 Length(V3 v);
f32 LengthSquared(V3 v);
V3 Normalize(V3 v);
f32 DotProduct(V3 a, V3 b);
V3 CrossProduct(V3 a, V3 b);
f32 SquareRoot();
