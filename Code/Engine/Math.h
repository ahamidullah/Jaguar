#pragma once

#include "Common.h"

#define FloatEpsilon FLT_EPSILON

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

	f32 &operator[](int i);
	f32 operator[](int i) const;
};

constexpr auto WorldRightVector = V3{1.0f, 0.0f, 0.0f};
constexpr auto WorldForwardVector = V3{0.0f, 1.0f, 0.0f};
constexpr auto WorldUpVector = V3{0.0f, 0.0f, 1.0f};

struct V4
{
	f32 x, y, z, w;

	f32 &operator[](int i);
	f32 operator[](int i) const;
};

struct M3
{
	f32 m[3][3];

	V3 &operator[](int i);
	V3 &operator[](int i) const;
};

struct M4
{
	f32 m[4][4];

	V4 &operator[](int i);
};

constexpr auto IdentityMatrix = M4
{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

struct Quaternion
{
	f32 x, y, z, w;
};

constexpr auto IdentityQuaternion = Quaternion{0.0f, 0.0f, 0.0f, 1.0f};

struct EulerAngles
{
	f32 pitch, yaw, roll;
};

struct Sphere
{
	V3 center;
	f32 radius;
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

#define PrintF32(f) PrintF32Actual(#f, (f))
void PrintF32Actual(const char *name, f32 number);
#define PrintV3(v) PrintV3Actual(#v, (v))
void PrintV3Actual(const char *name, V3 v);
#define PrintQuaternion(q) PrintQuaternionActual(#q, (q))
void PrintQuaternionActual(const char *name, Quaternion q);
#define PrintM4(m) PrintM4Actual(#m, (m))
void PrintM4Actual(const char *name, M4 m);
template <typename T>

f32 SquareRoot();
f32 Tan(f32 f);
f32 Sin(f32 f);
f32 Cos(f32 f);
f32 Acos(f32 f);
f32 Abs(f32 f);

s64 DivideAndRoundUp(s64 a, s64 b);
s64 AlignTo(s64 number, s64 alignment);
s64 AlignmentOffset(s64 number, s64 alignment);
s64 Minimum(s64 a, s64 b);
s64 Maximum(s64 a, s64 b);

bool NotNAN(V3 v);
bool NotZero(V3 v);
f32 Length(V3 v);
f32 LengthSquared(V3 v);
V3 Normalize(V3 v);
f32 DotProduct(V3 a, V3 b);
V3 CrossProduct(V3 a, V3 b);
V3 Lerp(V3 start, V3 end, f32 t);
V4 V3ToV4(V3 v, f32 w);
V3 operator-(V3 v);
V3 operator*(f32 s, V3 v);
V3 operator/(V3 v, f32 s);
V3 operator+(V3 a, V3 b);
V3 operator-(V3 a, V3 b);
bool operator==(const V3 &a, const V3 &b);
V3 &operator+=(V3 &a, V3 b);
V3 &operator-=(V3 &a, V3 b);

M3 CreateMatrix(V3 column1, V3 column2, V3 column3);
void SetRotationMatrix(M4 *m, M3 r);
Quaternion ToQuaternion(M3 m);
M4 CreateViewMatrix(V3 position, V3 forward);
M4 CreateInfinitePerspectiveProjectionMatrix(f32 near, f32 verticalFOV, f32 aspectRatio);
M4 CreateOrthographicProjectionMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
M4 operator*(M4 a, M4 b);
M4 operator*(f32 s, M4 m);
V4 operator*(M4 m, V4 v);

Quaternion CreateQuaternion(f32 x, f32 y, f32 z, f32 w);
Quaternion CreateQuaternion(V3 axis, f32 angle);
Quaternion CreateQuaternion(f32 yaw, f32 pitch, f32 roll);
Quaternion CreateQuaternion(V3 forward);
Quaternion operator*(Quaternion a, Quaternion b);
Quaternion operator*(f32 s, Quaternion q);
Quaternion Concatenate(Quaternion a, Quaternion b);
Quaternion Conjugate(Quaternion q);
V3 Rotate(V3 v, Quaternion q);
Quaternion Rotate(Quaternion q, f32 pitch, f32 yaw, f32 roll);
M3 ToMatrix(Quaternion q);
f32 NormSquared(Quaternion q);
Quaternion Normalize(Quaternion q);
Quaternion Lerp(Quaternion a, Quaternion b, f32 t);
V3 CalculateRightVector(const Quaternion &q);
V3 CalculateForwardVector(const Quaternion &q);
V3 CalculateUpVector(const Quaternion &q);
EulerAngles ToAngles(Quaternion q);

Quaternion ToQuaternion(EulerAngles euler);
