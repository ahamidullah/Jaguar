#include <math.h>

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
	return (V.x != NAN) && (V.y != NAN) && (V.z != NAN);
}

bool Notzero(V3 V) {
	return (V.x != 0.0f) || (V.y != 0.0f) || (V.z != 0.0f);
}

V4 V3ToV4(V3 V, f32 w) {
	return {
		V.x,
		V.y,
		V.z,
		w,
	};
}

V2 operator-(V2 A, V2 B) {
	return {
		A.x - B.x,
		A.y - B.y,
	};
}

bool operator==(const V3 &A, const V3 &B) {
	return A.x == B.x && A.y == B.y && A.z == B.z;
}

V3 operator-(V3 V) {
	return {
		-V.x,
		-V.y,
		-V.z,
	};
}

V3 operator*(f32 s, V3 v)
{
	return
	{
		s * v.x,
		s * v.y,
		s * v.z,
	};
}

V3 operator/(V3 V, f32 S) {
	S = 1.0f / S;
	return {
		S * V.x,
		S * V.y,
		S * V.z,
	};
}

V3 operator+(V3 A, V3 B) {
	return {
		A.x + B.x,
		A.y + B.y,
		A.z + B.z,
	};
}

V3 operator-(V3 A, V3 B) {
	return {
		A.x - B.x,
		A.y - B.y,
		A.z - B.z,
	};
}

V3 &operator+=(V3 &a, V3 b)
{
	a = a + b;
	return a;
}

V3 &operator-=(V3 &a, V3 b)
{
	a = a - b;
	return a;
}

// @TODO: USE A SWITCH?
f32 V3::operator[](int i) const
{
	Assert(i >= 0 && i <= 2);
	if (i == 0)
	{
		return x;
	}
	if (i == 1)
	{
		return y;
	}
	return z;
}

// @TODO: USE A SwITCH!
f32 &V3::operator[](int i)
{
	Assert(i >= 0 && i <= 2);
	if (i == 0)
	{
		return x;
	}
	if (i == 1)
	{
		return y;
	}
	return z;
}

// @TODO: USE A SwITCH!
f32 &V2::operator[](int i)
{
	Assert(i >= 0 && i <= 1);
	if (i == 0)
	{
		return x;
	}
	return y;
}

V4 operator/(V4 v, f32 s)
{
	return
	{
		v.x / s,
		v.y / s,
		v.z / s,
		v.w / s,
	};
}

V4 operator+(V4 a, V4 b)
{
	return
	{
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w,
	};
}

V4 &operator/=(V4 &v, f32 s)
{
	v = v / s;
	return v;
}

V4 operator*(f32 s, V4 v)
{
	return
	{
		s * v.x,
		s * v.y,
		s * v.z,
		s * v.w,
	};
}

// @TODO: USE A SwITCH!
f32 V4::operator[](int i) const
{
	Assert(i >= 0 && i <= 3);
	if (i == 0)  return x;
	if (i == 1)  return y;
	if (i == 2)  return z;
	return w;
}

// @TODO: USE A SwITCH!
f32 &V4::operator[](int i)
{
	Assert(i >= 0 && i <= 3);
	if (i == 0)  return x;
	if (i == 1)  return y;
	if (i == 2)  return z;
	return w;
}

V3 &M3::operator[](int I)
{
	return *(V3 *)m[I];
}

V4 &M4::operator[](int I)
{
	return *(V4 *)m[I];
}

M4 IdentityMatrix()
{
	return
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
}

M4 operator*(M4 a, M4 b) {
	M4 result;
	for (auto i = 0; i < 4; i++)
	{
		for (auto j = 0; j < 4; j++)
		{
			result.m[i][j] = a.m[i][0] * b.m[0][j]
			               + a.m[i][1] * b.m[1][j]
			               + a.m[i][2] * b.m[2][j]
			               + a.m[i][3] * b.m[3][j];
		}
	}
	return result;
}

M4 operator*(f32 s, M4 m)
{
	return
	{
		s * m.m[0][0], s * m.m[0][1], s * m.m[0][2], s * m.m[0][3],
		s * m.m[1][0], s * m.m[1][1], s * m.m[1][2], s * m.m[1][3],
		s * m.m[2][0], s * m.m[2][1], s * m.m[2][2], s * m.m[2][3],
		s * m.m[3][0], s * m.m[3][1], s * m.m[3][2], s * m.m[3][3]
	};
}

V4 operator*(M4 m, V4 v)
{
	return
	{
		v.x*m[0][0] + v.y*m[0][1] + v.z*m[0][2]  + v.w*m[0][3],
		v.x*m[1][0] + v.y*m[1][1] + v.z*m[1][2]  + v.w*m[1][3],
		v.x*m[2][0] + v.y*m[2][1] + v.z*m[2][2]  + v.w*m[2][3],
		v.x*m[3][0] + v.y*m[3][1] + v.z*m[3][2]  + v.w*m[3][3],
	};
}

f32 LengthSquared(V3 V) {
	return V.x*V.x + V.y*V.y + V.z*V.z;
}

f32 Length(V3 V) {
	return SquareRoot(V.x * V.x + V.y * V.y + V.z * V.z);
}

V3 Normalize(V3 V)
{
	V3 Result = (1 / Length(V)) * V;
	Assert(NotNAN(Result));
	return Result;
}

f32 DotProduct(V3 A, V3 B)
{
	return A.x*B.x + A.y*B.y + A.z*B.z;
}

V3 CrossProduct(V3 A, V3 B)
{
	V3 Result =
	{
		A.y*B.z - B.y*A.z,
		A.z*B.x - B.z*A.x,
		A.x*B.y - B.x*A.y,
	};
	Assert(Notzero(Result));
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

void SetRotation(M4 *m, M3 r)
{
	m->m[0][0] = r.m[0][0];
	m->m[0][1] = r.m[0][1];
	m->m[0][2] = r.m[0][2];
	m->m[1][0] = r.m[1][0];
	m->m[1][1] = r.m[1][1];
	m->m[1][2] = r.m[1][2];
	m->m[2][0] = r.m[2][0];
	m->m[2][1] = r.m[2][1];
	m->m[2][2] = r.m[2][2];
}

void SetTranslation(M4 *m, V3 t)
{
	m->m[0][3] = t.x;
	m->m[1][3] = t.y;
	m->m[2][3] = t.z;
}

void SetScale(M4 *m, V3 s)
{
	for (s32 i = 0; i < 4; i++)
	{
		m->m[i][0] *= s.x;
		m->m[i][1] *= s.y;
		m->m[i][2] *= s.z;
	}
}

V3 GetTranslation(M4 m)
{
	return
	{
		m.m[0][3],
		m.m[1][3],
		m.m[2][3],
	};
}

M4 Transpose(M4 m)
{
	return
	{
		m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
		m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
		m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
		m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3],
	};
}

M4 PerspectiveProjection(f32 verticalFOV, f32 aspectRatio, f32 near, f32 far)
{
	f32 focalLength = 1.0f / tanf(verticalFOV / 2.0f);
	return
	{
		focalLength / aspectRatio,   0.0f,           0.0f,                 0.0f,
		0.0f,                        -focalLength,   0.0f,                 0.0f,
		0.0f,                        0.0f,           far / (near - far),   -(far * near) / (far - near),
		0.0f,                        0.0f,           -1.0f,                0.0f,
	};
}

// Assumes near is 0.01f and far is infinity.
M4 InfinitePerspectiveProjection(f32 verticalFOV, f32 aspectRatio)
{
	constexpr f32 near = 0.01f;
	f32 focalLength = 1 / Tan(verticalFOV / 2);
	return
	{
		focalLength,   0.0f,                         0.0f,     0.0f,
		0.0f,          -focalLength * aspectRatio,   0.0f,     0.0f,
		0.0f,          0.0f,                         -1.0f,    -2.0f * near,
		0.0f,          0.0f,                         -1.0f,    0.0f,
	};
}

// Assumes near is -1.0f and far is 1.0f.
M4 OrthographicProjection(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
	return
	{
		2.0f / (right - left),   0.0f,                     0.0f,                   -(right + left) / (right - left),
	    0.0f,                    -2.0f / (top - bottom),   0.0f,                   -(top + bottom) / (top - bottom),
	    0.0f,                    0.0f,                     -1.0f / (far - near),   -near / (far - near),
	    0.0f,                    0.0f,                     0.0f,                   1.0f,
	};
}

M4 ViewMatrix(V3 position, V3 forward)
{
	const V3 worldUp = {0.0f, 0.0f, 1.0f};
	forward = Normalize(forward);
	auto right = Normalize(CrossProduct(forward, worldUp));
	auto up = Normalize(CrossProduct(right, forward));
	return
	{
		right.x,      right.y,      right.z,      -DotProduct(right, position),
		up.x,         up.y,         up.z,         -DotProduct(up, position),
		-forward.x,   -forward.y,   -forward.z,   DotProduct(forward, position),
		0,            0,            0,            1.0f,
	};
}

M4 LookAt(V3 position, V3 target)
{
	return ViewMatrix(position, target - position);
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
