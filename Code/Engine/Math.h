#pragma once

#include "Basic/String.h"
#include "Common.h"

#define FloatEpsilon FLT_EPSILON
#define Pi 3.14159265358979323846264338327950288
#define DegreesToRadians(d) (d * (Pi / 180.0))
#define RadiansToDegrees(r) (r * (180.0 / Pi))

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

struct V4
{
	f32 x, y, z, w;

	f32 &operator[](int i);
	f32 operator[](int i) const;
};

struct EulerAngles;
struct Quaternion;

struct V3
{
	f32 x, y, z;

	f32 &operator[](int i);
	f32 operator[](int i) const;
	V3 Rotate(Quaternion q);
	bool NotNAN();
	bool NotZero();
	f32 Length();
	f32 LengthSquared();
	V3 Normal();
	V3 Abs();
	V4 ToV4(f32 w);
};
V3 operator/(V3 v, f32 s);
V3 operator+(V3 a, V3 b);
V3 operator-(V3 a, V3 b);
bool operator==(V3 a, V3 b);
V3 &operator+=(V3 &a, V3 b);
V3 &operator-=(V3 &a, V3 b);
V3 operator*(f32 s, V3 v);

f32 VectorDotProduct(V3 a, V3 b);
V3 VectorCrossProduct(V3 a, V3 b);
V3 VectorLerp(V3 start, V3 end, f32 t);

constexpr auto WorldRightVector = V3{1.0f, 0.0f, 0.0f};
constexpr auto WorldForwardVector = V3{0.0f, 1.0f, 0.0f};
constexpr auto WorldUpVector = V3{0.0f, 0.0f, 1.0f};

struct AxisAngle
{
	V3 axis;
	f32 angle;
};

struct Sphere
{
	V3 center;
	f32 radius;
};

struct M3
{
	f32 m[3][3];

	V3 &operator[](int i);
	V3 &operator[](int i) const;
	M3 Transpose();
	Quaternion ToQuaternion();
	EulerAngles ToAngles();
};

M3 NewMatrix(V3 column1, V3 column2, V3 column3);

struct M4
{
	f32 m[4][4];

	V4 &operator[](int i);
	void SetScale(V3 s);
	void SetRotation(M3 r);
	void SetTranslation(V3 t);
	M4 Transpose();
	Quaternion ToQuaternion();
};
M4 operator*(f32 s, M4 m);
M4 operator*(M4 a, M4 b);
V4 operator*(M4 m, V4 v);

M4 ViewMatrix(V3 position, V3 forward);
M4 InfinitePerspectiveProjectionMatrix(f32 near, f32 verticalFOV, f32 aspectRatio);
M4 OrthographicProjectionMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);

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

	Quaternion Conjugate();
	f32 NormalSquared();
	Quaternion Normal();
	V3 Right();
	V3 Forward();
	V3 Up();
	V3 RotateVector(V3 v);
	EulerAngles ToAngles();
	M3 ToMatrix();
};
Quaternion operator*(Quaternion a, Quaternion b);
Quaternion operator*(f32 s, Quaternion q);

Quaternion NewQuaternion(V3 forward);
Quaternion NewQuaternionFromAxisAngle(AxisAngle aa);
Quaternion NewQuaternionFromEuler(EulerAngles ea);
Quaternion operator*(f32 s, Quaternion q);
Quaternion Concatenate(Quaternion a, Quaternion b);
Quaternion QuaternionLerp(Quaternion a, Quaternion b, f32 t);

constexpr auto IdentityQuaternion = Quaternion{0.0f, 0.0f, 0.0f, 1.0f};

struct EulerAngles
{
	f32 pitch, yaw, roll;

	Quaternion ToQuaternion();
};

#define PrintF32(f) PrintF32Actual(#f, (f))
void PrintF32Actual(String name, f32 number);
#define PrintV3(v) PrintV3Actual(#v, (v))
void PrintV3Actual(String name, V3 v);
#define PrintQuaternion(q) PrintQuaternionActual(#q, (q))
void PrintQuaternionActual(String name, Quaternion q);
#define PrintM4(m) PrintM4Actual(#m, (m))
void PrintM4Actual(String name, M4 m);
