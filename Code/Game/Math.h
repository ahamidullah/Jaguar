#pragma once

struct V2 {
	f32 X, Y;
	f32 &operator[](int I);
};

struct V2s {
	s32 X, Y;
};

struct V2u {
	u32 X, Y;
};

struct V3 {
	f32 X, Y, Z;
	f32 operator[](int I) const;
	f32 &operator[](int I);
};

struct V4 {
	f32 X, Y, Z, W;
	f32 operator[](int I) const;
	f32 &operator[](int I);
};

struct M3 {
	f32 M[3][3];
	V3 &operator[](int I);
};

struct M4 {
	f32 M[4][4];
	V4 &operator[](int I);
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

struct Transform {
	V3 translation;
	Quaternion rotation;
};

#define PrintF32(F) PrintF32Actual(#F, (F))
#define PrintV3(V) PrintV3Actual(#V, (V))
#define PrintM4(M) PrintM4Actual(#M, (M))

constexpr f32 DEGREES_TO_RADIANS_MULTIPLIER = M_PI / 180.0;
constexpr f32 RADIANS_TO_DEGREES_MULTIPLIER = 180.0 / M_PI;

constexpr f32 DegreesToRadians(f32 Degrees) {
	return Degrees * DEGREES_TO_RADIANS_MULTIPLIER;
}

constexpr f32 RadiansToDegrees(f32 Radians) {
	return Radians * RADIANS_TO_DEGREES_MULTIPLIER;
}

template <typename T>
T Minimum(T A, T B) {
	if (A < B) {
		return A;
	}
	return B;
}

template <typename T>
T Maximum(T A, T B) {
	if (A > B) {
		return A;
	}
	return B;
}

u32 DivideAndRoundUp(u32 A, u32 B);
bool NotNAN(V3 V);
bool NotZero(V3 V);
V4 V3ToV4(V3 V, f32 W);
M4 IdentityMatrix();
f32 Length(V3 V);
f32 LengthSquared(V3 V);
V3 Normalize(V3 V);
f32 DotProduct(V3 A, V3 B);
V3 CrossProduct(V3 A, V3 B);
f32 SquareRoot();
