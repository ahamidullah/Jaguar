#include <math.h>

#include "Math.h"

f32 SquareRoot(f32 F) {
	return sqrt(F);
}

f32 Tan(f32 F) {
	return tan(F);
}

f32 Sin(f32 F) {
	return sin(F);
}

f32 Cos(f32 F) {
	return cos(F);
}

u32 DivideAndRoundUp(u32 A, u32 B) {
	return (A + (B - 1)) / B;
}

bool NotNAN(V3 V) {
	return (V.X != NAN) && (V.Y != NAN) && (V.Z != NAN);
}

bool NotZero(V3 V) {
	return (V.X != 0.0f) || (V.Y != 0.0f) || (V.Z != 0.0f);
}

V4 V3ToV4(V3 V, f32 W) {
	return {
		V.X,
		V.Y,
		V.Z,
		W,
	};
}

V2 operator-(V2 A, V2 B) {
	return {
		A.X - B.X,
		A.Y - B.Y,
	};
}

bool operator==(const V3 &A, const V3 &B) {
	return A.X == B.X && A.Y == B.Y && A.Z == B.Z;
}

V3 operator-(V3 V) {
	return {
		-V.X,
		-V.Y,
		-V.Z,
	};
}

V3 operator*(f32 S, V3 V) {
	return {
		S * V.X,
		S * V.Y,
		S * V.Z,
	};
}

V3 operator/(V3 V, f32 S) {
	S = 1.0f / S;
	return {
		S * V.X,
		S * V.Y,
		S * V.Z,
	};
}

V3 operator+(V3 A, V3 B) {
	return {
		A.X + B.X,
		A.Y + B.Y,
		A.Z + B.Z,
	};
}

V3 operator-(V3 A, V3 B) {
	return {
		A.X - B.X,
		A.Y - B.Y,
		A.Z - B.Z,
	};
}

V3 &operator+=(V3 &A, V3 B) {
	A = A + B;
	return A;
}

V3 &operator-=(V3 &A, V3 B) {
	A = A - B;
	return A;
}

// @TODO: USE A SWITCH!
f32 V3::operator[](int I) const {
	Assert(I >= 0 && I <= 2);
	if (I == 0)  return X;
	if (I == 1)  return Y;
	return Z;
}

// @TODO: USE A SWITCH!
f32 &V3::operator[](int I) {
	Assert(I >= 0 && I <= 2);
	if (I == 0)  return X;
	if (I == 1)  return Y;
	return Z;
}

// @TODO: USE A SWITCH!
f32 &V2::operator[](int I) {
	Assert(I >= 0 && I <= 1);
	if (I == 0)  return X;
	return Y;
}

V4 operator/(V4 V, f32 S) {
	return {
		V.X / S,
		V.Y / S,
		V.Z / S,
		V.W / S,
	};
}

V4 operator+(V4 A, V4 B) {
	return {
		A.X + B.X,
		A.Y + B.Y,
		A.Z + B.Z,
		A.W + B.W,
	};
}

V4 &operator/=(V4 &V, f32 S) {
	V = V / S;
	return V;
}

V4 operator*(f32 S, V4 V) {
	return {
		S * V.X,
		S * V.Y,
		S * V.Z,
		S * V.W,
	};
}

// @TODO: USE A SWITCH!
f32 V4::operator[](int I) const {
	Assert(I >= 0 && I <= 3);
	if (I == 0)  return X;
	if (I == 1)  return Y;
	if (I == 2)  return Z;
	return W;
}

// @TODO: USE A SWITCH!
f32 &V4::operator[](int I) {
	Assert(I >= 0 && I <= 3);
	if (I == 0)  return X;
	if (I == 1)  return Y;
	if (I == 2)  return Z;
	return W;
}

V3 &M3::operator[](int I) {
	return *(V3 *)M[I];
}

V4 &M4::operator[](int I) {
	return *(V4 *)M[I];
}

M4 IdentityMatrix() {
	return {{
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f},
	}};
}

M4 operator*(M4 A, M4 B) {
	M4 Result;
	for (s32 I = 0; I < 4; I++) {
		for (s32 J = 0; J < 4; J++) {
			Result.M[I][J] = A.M[I][0] * B.M[0][J]
			               + A.M[I][1] * B.M[1][J]
			               + A.M[I][2] * B.M[2][J]
			               + A.M[I][3] * B.M[3][J];
		}
	}
	return Result;
}

M4 operator*(f32 S, M4 M) {
	return {
		S * M.M[0][0], S * M.M[0][1], S * M.M[0][2], S * M.M[0][3],
		S * M.M[1][0], S * M.M[1][1], S * M.M[1][2], S * M.M[1][3],
		S * M.M[2][0], S * M.M[2][1], S * M.M[2][2], S * M.M[2][3],
		S * M.M[3][0], S * M.M[3][1], S * M.M[3][2], S * M.M[3][3]
	};
}

V4 operator*(M4 M, V4 V) {
	return {
		V.X*M[0][0] + V.Y*M[0][1] + V.Z*M[0][2]  + V.W*M[0][3],
		V.X*M[1][0] + V.Y*M[1][1] + V.Z*M[1][2]  + V.W*M[1][3],
		V.X*M[2][0] + V.Y*M[2][1] + V.Z*M[2][2]  + V.W*M[2][3],
		V.X*M[3][0] + V.Y*M[3][1] + V.Z*M[3][2]  + V.W*M[3][3],
	};
}

f32 LengthSquared(V3 V) {
	return V.X*V.X + V.Y*V.Y + V.Z*V.Z;
}

f32 Length(V3 V) {
	return SquareRoot(V.X * V.X + V.Y * V.Y + V.Z * V.Z);
}

#if 0
V3 subtract_v3(V3 a, V3 b) {
	return (V3){
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
	};
}

V3 add_v3(V3 a, V3 b) {
	return (V3){
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
	};
}

V3 scale_v3(f32 s, V3 v) {
	return (V3){
		s * v.x,
		s * v.y,
		s * v.z,
	};
}

M4 multiply_m4(M4 a, M4 b) {
	M4 result;
	for (s32 i = 0; i < 4; i++) {
		for (s32 j = 0; j < 4; j++) {
			result.m[i][j] = a.m[i][0] * b.m[0][j]
			               + a.m[i][1] * b.m[1][j]
			               + a.m[i][2] * b.m[2][j]
			               + a.m[i][3] * b.m[3][j];
		}
	}
	return result;
}
#endif

V3 Normalize(V3 V) {
	V3 Result = (1 / Length(V)) * V;
	Assert(NotNAN(Result));
	return Result;
}

f32 DotProduct(V3 A, V3 B) {
	return A.X*B.X + A.Y*B.Y + A.Z*B.Z;
}

V3 CrossProduct(V3 A, V3 B) {
	V3 Result = (V3){
		A.Y*B.Z - B.Y*A.Z,
		A.Z*B.X - B.Z*A.X,
		A.X*B.Y - B.X*A.Y,
	};
	Assert(NotZero(Result));
	return Result;
}

/*
M4 inverse(M4 m) {
	const V3 *a = (V3 *)&m.m[0];
	const V3 *b = (V3 *)&m.m[1];
	const V3 *c = (V3 *)&m.m[2];
	const V3 *d = (V3 *)&m.m[3];

	f32 x = m.m[3][0];
	f32 y = m.m[3][1];
	f32 z = m.m[3][2];
	f32 w = m.m[3][3];

	V3 s = cross_product(*a, *b);
	V3 t = cross_product(*c, *d);
	V3 u = (y * *a) - (x * *b);
	V3 v = (w * *c) - (z * *d);

	f32 inverse_determinant = 1.0f / (dot_product(s, v) + dot_product(t, u));
	s = inverse_determinant * s;
	t = inverse_determinant * t;
	u = inverse_determinant * u;
	v = inverse_determinant * v;

	V3 r0 = cross_product(*b, v) + (y * t);
	V3 r1 = cross_product(v, *a) - (x * t);
	V3 r2 = cross_product(*d, u) + (w * s);
	V3 r3 = cross_product(u, *c) - (z * s);

	// @TODO: Transpose.
	return M4{
		r0.x, r0.y, r0.z, -dot_product(*b, t),
		r1.x, r1.y, r1.z, dot_product(*a, t),
		r2.x, r2.y, r2.z, -dot_product(*d, s),
		r3.x, r3.y, r3.z, dot_product(*c, s),
	};
}
*/

void SetRotation(M4 *M, M3 R) {
	M->M[0][0] = R.M[0][0];
	M->M[0][1] = R.M[0][1];
	M->M[0][2] = R.M[0][2];
	M->M[1][0] = R.M[1][0];
	M->M[1][1] = R.M[1][1];
	M->M[1][2] = R.M[1][2];
	M->M[2][0] = R.M[2][0];
	M->M[2][1] = R.M[2][1];
	M->M[2][2] = R.M[2][2];
}

void SetTranslation(M4 *M, V3 T) {
	M->M[0][3] = T.X;
	M->M[1][3] = T.Y;
	M->M[2][3] = T.Z;
}

void SetScale(M4 *M, V3 S) {
	for (s32 I = 0; I < 4; I++) {
		M->M[I][0] *= S.X;
		M->M[I][1] *= S.Y;
		M->M[I][2] *= S.Z;
	}
}

V3 GetTranslation(M4 M) {
	return {
		M.M[0][3],
		M.M[1][3],
		M.M[2][3]
	};
}

M4 Transpose(M4 M) {
	return {{
		{M.M[0][0], M.M[1][0], M.M[2][0], M.M[3][0]},
		{M.M[0][1], M.M[1][1], M.M[2][1], M.M[3][1]},
		{M.M[0][2], M.M[1][2], M.M[2][2], M.M[3][2]},
		{M.M[0][3], M.M[1][3], M.M[2][3], M.M[3][3]},
	}};
}

M4 PerspectiveProjection(f32 VerticalFOV, f32 AspectRatio, f32 Near, f32 Far) {
	f32 FocalLength = 1.0f / tanf(VerticalFOV / 2.0f);
	return {{
		{FocalLength / AspectRatio,   0.0f,           0.0f,                 0.0f},
		{0.0f,                        -FocalLength,   0.0f,                 0.0f},
		{0.0f,                        0.0f,           Far / (Near - Far),   -(Far * Near) / (Far - Near)},
		{0.0f,                        0.0f,           -1.0f,                0.0f},
	}};
}

// Assumes near is 0.01f and far is infinity.
M4 InfinitePerspectiveProjection(f32 VerticalFOV, f32 AspectRatio) {
	constexpr f32 Near = 0.01f;
	f32 FocalLength = 1 / Tan(VerticalFOV / 2);
	return {{
		{FocalLength,   0.0f,                         0.0f,     0.0f},
		{0.0f,          -FocalLength * AspectRatio,   0.0f,     0.0f},
		{0.0f,          0.0f,                         -1.0f,    -2.0f * Near},
		{0.0f,          0.0f,                         -1.0f,    0.0f},
	}};
}

// Assumes near is -1.0f and far is 1.0f.
M4 OrthographicProjection(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Near, f32 Far) {
	return (M4){{
		{2.0f / (Right - Left),   0.0f,                     0.0f,                   -(Right + Left) / (Right - Left)},
	    {0.0f,                    -2.0f / (Top - Bottom),   0.0f,                   -(Top + Bottom) / (Top - Bottom)},
	    {0.0f,                    0.0f,                     -1.0f / (Far - Near),   -Near / (Far - Near)},
	    {0.0f,                    0.0f,                     0.0f,                   1.0f},
	}};
}

M4 ViewMatrix(V3 Position, V3 Forward, V3 Side, V3 Up) {
	return (M4){{
		{Side.X,       Side.Y,       Side.Z,       -DotProduct(Side, Position)},
		{Up.X,         Up.Y,         Up.Z,         -DotProduct(Up, Position)},
		{-Forward.X,   -Forward.Y,   -Forward.Z,   DotProduct(Forward, Position)},
		{0,            0,            0,            1.0f},
	}};
}

M4 LookAt(V3 position, V3 target, V3 worldUp) {
	// @TODO: Move this out to a create basis function.
	V3 forward = Normalize(target - position);
	V3 side = Normalize(CrossProduct(forward, worldUp));
	V3 up = Normalize(CrossProduct(side, forward));
	return ViewMatrix(position, forward, side, up);
}

/*
Quaternion operator*(Quaternion q1, Quaternion q2) {
	return {
		q1.w * q2.x + q2.w * q1.x + q1.y * q2.z - q2.y * q1.z,
		q1.w * q2.y + q2.w * q1.y + q2.x * q1.z - q1.x * q2.z,
		q1.w * q2.z + q2.w * q1.z + q1.x * q2.y - q2.x * q1.y,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
	};
}

Quaternion operator*(f32 s, Quaternion q) {
	return {
		s * q.x,
		s * q.y,
		s * q.z,
		s * q.w,
	};
}

Quaternion conjugate(Quaternion q) {
	return {-q.x, -q.y, -q.z, q.w};
}

V3 rotate_vector(V3 v, Quaternion q) {
	Quaternion result = q * Quaternion{v.x, v.y, v.z, 0.0} * conjugate(q);
	return {result.x, result.y, result.z};
}

M3 to_matrix(Quaternion q) {
	f32 x2 = q.x * q.x;
	f32 y2 = q.y * q.y;
	f32 z2 = q.z * q.z;
	f32 xy = q.x * q.y;
	f32 xz = q.x * q.z;
	f32 yz = q.y * q.z;
	f32 wx = q.w * q.x;
	f32 wy = q.w * q.y;
	f32 wz = q.w * q.z;

	return M3{
		1.0f - 2.0f * (y2 + z2),  2.0f * (xy - wz),         2.0f * (xz + wy),
		2.0f * (xy + wz),         1.0f - 2.0f * (x2 + z2),  2.0f * (yz - wx),
		2.0f * (xz - wy),         2.0f * (yz + wx),         1.0f - 2.0f * (x2 + y2),
	};
}

V3 lerp(V3 start, V3 end, f32 t) {
	return start + t * (end - start);
}

f32 norm_squared(Quaternion q) {
	return q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w; 
}

Quaternion normalize(Quaternion q) {
	f32 inverse_norm_squared = 1.0f / (f32)sqrt(norm_squared(q));
	return {q.x*inverse_norm_squared, q.y*inverse_norm_squared, q.z*inverse_norm_squared, q.w*inverse_norm_squared};
}

Quaternion lerp(Quaternion a, Quaternion b, f32 t) {
	f32 dot_product = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	f32 one_minus_t = 1.0f - t;

	Quaternion result;

	if (dot_product < 0.0f)
	{
		result.x = one_minus_t * a.x + t * -b.x;
		result.y = one_minus_t * a.y + t * -b.y;
		result.z = one_minus_t * a.z + t * -b.z;
		result.w = one_minus_t * a.w + t * -b.w;
	} else {
		result.x = one_minus_t * a.x + t * b.x;
		result.y = one_minus_t * a.y + t * b.y;
		result.z = one_minus_t * a.z + t * b.z;
		result.w = one_minus_t * a.w + t * b.w;
	}

	normalize(result);

	return result;
}
*/
