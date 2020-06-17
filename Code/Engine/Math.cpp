#include "Math.h"
#include "GPU.h"

#include "Code/Basic/Log.h"

void PrintM4Actual(String name, M4 m)
{
	ConsolePrint("%k:\n", name);
	for (auto i = 0; i < 4; i++)
	{
		ConsolePrint("\t%f %f %f %f\n", m[i][0], m[i][1], m[i][2], m[i][3]);
	}
}

void PrintV3Actual(String name, V3 v)
{
	ConsolePrint("%k: %f %f %f\n", name, v.x, v.y, v.z);
}

void PrintQuaternionActual(String name, Quaternion q)
{
	ConsolePrint("%k: %f %f %f %f\n", name, q.x, q.y, q.z, q.w);
}

void PrintF32Actual(String name, f32 number)
{
	ConsolePrint("%k: %f\n", name, number);
}

f32 SquareRoot(f32 x)
{
	return sqrt(x);
}

f32 Tan(f32 x)
{
	return tan(x);
}

f32 Atan2(f32 x, f32 y)
{
	return atan2(x, y);
}

f32 Sin(f32 x)
{
	return sin(x);
}

f32 Asin(f32 x)
{
	return asin(x);
}

f32 Cos(f32 x)
{
	return cos(x);
}

f32 Acos(f32 x)
{
	return acos(x);
}

void SinCos(f32 angle, f32 *sin, f32 *cos)
{
	// @TODO
	*sin = Sin(angle);
	*cos = Cos(angle);
}

f32 Abs(f32 x)
{
	return fabs(x);
}

f32 Fmod(f32 x, f32 mod)
{
	return fmodf(x, mod);
}

s64 DivideAndRoundUp(s64 x, s64 y)
{
	auto result = s64{x / y};
	if (x % y != 0)
	{
		result += 1;
	}
	return result;
}

bool NotNAN(V3 v)
{
	return (v.x != NAN) && (v.y != NAN) && (v.z != NAN);
}

bool NotZero(V3 v)
{
	return (v.x != 0.0f) || (v.y != 0.0f) || (v.z != 0.0f);
}

s64 AlignTo(s64 number, s64 alignment)
{
	auto remainder = number % alignment;
	if (remainder == 0)
	{
		return number;
	}
	return number + alignment - remainder;
}

s64 AlignmentOffset(s64 number, s64 alignment)
{
	auto remainder = number % alignment;
	if (remainder == 0)
	{
		return 0;
	}
	return alignment - remainder;
}

s64 Minimum(s64 a, s64 b)
{
	if (a < b)
	{
		return a;
	}
	return b;
}

s64 Maximum(s64 a, s64 b)
{
	if (a > b)
	{
		return a;
	}
	return b;
}

V4 V3ToV4(V3 v, f32 w)
{
	return
	{
		v.x,
		v.y,
		v.z,
		w,
	};
}

V2 operator-(V2 a, V2 b)
{
	return
	{
		a.x - b.x,
		a.y - b.y,
	};
}

bool operator==(const V3 &a, const V3 &b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

V3 operator-(V3 v)
{
	return {
		-v.x,
		-v.y,
		-v.z,
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

V3 operator/(V3 v, f32 s)
{
	s = 1.0f / s;
	return
	{
		s * v.x,
		s * v.y,
		s * v.z,
	};
}

V3 operator+(V3 a, V3 b)
{
	return
	{
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
	};
}

V3 operator-(V3 a, V3 b)
{
	return
	{
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
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

V3 &M3::operator[](int i)
{
	return *(V3 *)m[i];
}

V3 &M3::operator[](int i) const
{
	return *(V3 *)m[i];
}

V4 &M4::operator[](int i)
{
	return *(V4 *)m[i];
}

M4 operator*(M4 a, M4 b)
{
	M4 result;
	for (auto i = 0; i < 4; i++)
	{
		for (auto j = 0; j < 4; j++)
		{
			result[i][j] = a[i][0] * b[0][j]
			             + a[i][1] * b[1][j]
			             + a[i][2] * b[2][j]
			             + a[i][3] * b[3][j];
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

f32 LengthSquared(V3 v)
{
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

f32 Length(V3 v)
{
	return SquareRoot(v.x * v.x + v.y * v.y + v.z * v.z);
}

V3 Normalize(V3 v)
{
	V3 Result = (1 / Length(v)) * v;
	Assert(NotNAN(Result));
	return Result;
}

f32 DotProduct(V3 a, V3 b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

V3 CrossProduct(V3 a, V3 b)
{
	V3 result =
	{
		a.y * b.z - b.y * a.z,
		a.z * b.x - b.z * a.x,
		a.x * b.y - b.x * a.y,
	};
	Assert(NotZero(result));
	return result;
}

V3 Abs(V3 v)
{
	return
	{
		Abs(v.x),
		Abs(v.y),
		Abs(v.z),
	};
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

M3 CreateMatrix(V3 column1, V3 column2, V3 column3)
{
	return
	{
		column1.x, column2.x, column3.x,
		column1.y, column2.y, column3.y,
		column1.z, column2.z, column3.z,
	};
}

V3 operator*(M3 m, V3 v)
{
	return
	{
		v.x*m[0][0] + v.y*m[0][1] + v.z*m[0][2],
		v.x*m[1][0] + v.y*m[1][1] + v.z*m[1][2],
		v.x*m[2][0] + v.y*m[2][1] + v.z*m[2][2],
	};
}

void CreateUprightOrthonormalBasis(V3 forward, V3 *right, V3 *up)
{
	if (Abs(forward) == WorldUpVector)
	{
		*right = WorldRightVector;
	}
	else
	{
		*right = Normalize(CrossProduct(forward, WorldUpVector));
	}
	*up = Normalize(CrossProduct(*right, forward));
}

void SetRotation(M4 *m, M3 r)
{
	(*m)[0][0] = r[0][0];
	(*m)[0][1] = r[0][1];
	(*m)[0][2] = r[0][2];
	(*m)[1][0] = r[1][0];
	(*m)[1][1] = r[1][1];
	(*m)[1][2] = r[1][2];
	(*m)[2][0] = r[2][0];
	(*m)[2][1] = r[2][1];
	(*m)[2][2] = r[2][2];
}

void SetTranslation(M4 *m, V3 t)
{
	(*m)[0][3] = t.x;
	(*m)[1][3] = t.y;
	(*m)[2][3] = t.z;
}

void SetScale(M4 *m, V3 s)
{
	for (s32 i = 0; i < 4; i++)
	{
		(*m)[i][0] *= s.x;
		(*m)[i][1] *= s.y;
		(*m)[i][2] *= s.z;
	}
}

M4 Transpose(M4 m)
{
	return
	{
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3],
	};
}

M3 Transpose(M3 m)
{
	return
	{
		m[0][0], m[1][0], m[2][0],
		m[0][1], m[1][1], m[2][1],
		m[0][2], m[1][2], m[2][2],
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

M4 CreateInfinitePerspectiveProjectionMatrix(f32 near, f32 verticalFOV, f32 aspectRatio)
{
	// Assumes camera space is looking down positive z.
	if (usingVulkanAPI)
	{
		// Vulkan NDC:
		//   right-handed
		//   x points right, and y points down, z points forward
		//   x from -1 to 1, y from -1 to 1, and z from 0 to 1 (half of opengl)
		// @TODO: Divide z by 1/2?
		auto focalLength = 1.0f / Tan(verticalFOV / 2.0f);
		return
		{
			focalLength,   0.0f,                         0.0f,     0.0f,
			0.0f,          -focalLength * aspectRatio,   0.0f,     0.0f,
			0.0f,          0.0f,                         1.0f,    -2.0f * near,
			0.0f,          0.0f,                         1.0f,    0.0f,
		};
	}
	else
	{
		Abort("Unknown render API.");
	}
}

M4 CreateOrthographicProjectionMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
	return
	{
		2.0f / (right - left),   0.0f,                     0.0f,                   -(right + left) / (right - left),
	    0.0f,                    -2.0f / (top - bottom),   0.0f,                   -(top + bottom) / (top - bottom),
	    0.0f,                    0.0f,                     -1.0f / (far - near),   -near / (far - near),
	    0.0f,                    0.0f,                     0.0f,                   1.0f,
	};
}

M4 CreateViewMatrix(V3 position, V3 forward)
{
	forward = Normalize(forward);
	V3 right, up;
	CreateUprightOrthonormalBasis(forward, &right, &up);
	return
	{
		right.x,      right.y,      right.z,      -DotProduct(right, position),
		up.x,         up.y,         up.z,         -DotProduct(up, position),
		forward.x,    forward.y,    forward.z,    -DotProduct(forward, position),
		0,            0,            0,            1.0f,
	};
}

Quaternion ToQuaternion(M3 m)
{
	Quaternion result;
	auto trace = m[0][0] + m[1][1] + m[2][2];
	if (trace > 0)
	{
		auto s = 0.5f / SquareRoot(trace + 1.0f);
		result.w = 0.25f / s;
		result.x = (m[2][1] - m[1][2]) * s;
		result.y = (m[0][2] - m[2][0]) * s;
		result.z = (m[1][0] - m[0][1]) * s;
	}
	else
	{
		if (m[0][0] > m[1][1] && m[0][0] > m[2][2])
		{
			auto s = 2.0f * SquareRoot(1.0f + m[0][0] - m[1][1] - m[2][2]);
			result.w = (m[2][1] - m[1][2] ) / s;
			result.x = 0.25f * s;
			result.y = (m[0][1] + m[1][0] ) / s;
			result.z = (m[0][2] + m[2][0] ) / s;
		}
		else if (m[1][1] > m[2][2])
		{
			auto s = 2.0f * SquareRoot(1.0f + m[1][1] - m[0][0] - m[2][2]);
			result.w = (m[0][2] - m[2][0] ) / s;
			result.x = (m[0][1] + m[1][0] ) / s;
			result.y = 0.25f * s;
			result.z = (m[1][2] + m[2][1] ) / s;
		}
		else
		{
			auto s = 2.0f * SquareRoot(1.0f + m[2][2] - m[0][0] - m[1][1]);
			result.w = (m[1][0] - m[0][1] ) / s;
			result.x = (m[0][2] + m[2][0] ) / s;
			result.y = (m[1][2] + m[2][1] ) / s;
			result.z = 0.25f * s;
		}
	}
	return result;
}

EulerAngles ToAngles(M3 m)
{
	EulerAngles result;
	auto s = SquareRoot(m[0][0] * m[0][0] + m[0][1] * m[0][1]);
	if (s > FLOAT_EPSILON)
	{
		result.roll = -Atan2(m[0][2], s);
		result.yaw = Atan2(m[0][1], m[0][0]);
		result.pitch = Atan2(m[1][2], m[2][2]);
	}
	else
	{
		if (m[0][2] < 0.0f)
		{
			result.roll = 90.0f;
		}
		else
		{
			result.roll = -90.0f;
		}
		result.yaw = -Atan2(m[1][0], m[1][1]);
		result.pitch = 0.0f;
	}
	return result;
}

Quaternion CreateQuaternion(V3 axis, f32 angle)
{
	auto imaginary = Sin(angle / 2.0f) * axis;
	return
	{
		imaginary.x,
		imaginary.y,
		imaginary.z,
		Cos(angle / 2.0f),
	};
}

Quaternion CreateQuaternion(V3 forward)
{
	forward = Normalize(forward);
	V3 right, up;
	CreateUprightOrthonormalBasis(forward, &right, &up);
	return Normalize(ToQuaternion(CreateMatrix(right, forward, up)));
}

Quaternion operator*(Quaternion a, Quaternion b)
{
	return
	{
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
	};
}

Quaternion operator*(f32 s, Quaternion q)
{
	return
	{
		s * q.x,
		s * q.y,
		s * q.z,
		s * q.w,
	};
}

Quaternion Conjugate(Quaternion q)
{
	return
	{
		-q.x,
		-q.y,
		-q.z,
		q.w
	};
}

V3 Rotate(V3 v, Quaternion q)
{
	Quaternion result = q * Quaternion{v.x, v.y, v.z, 0.0} * Conjugate(q);
	return
	{
		result.x,
		result.y,
		result.z,
	};
}

// @TODO: Clean this up.
M3 ToMatrix(Quaternion q)
{
	M3	mat;
	float	wx, wy, wz;
	float	xx, yy, yz;
	float	xy, xz, zz;
	float	x2, y2, z2;

	x2 = q.x + q.x;
	y2 = q.y + q.y;
	z2 = q.z + q.z;

	xx = q.x * x2;
	xy = q.x * y2;
	xz = q.x * z2;

	yy = q.y * y2;
	yz = q.y * z2;
	zz = q.z * z2;

	wx = q.w * x2;
	wy = q.w * y2;
	wz = q.w * z2;

	mat[ 0 ][ 0 ] = 1.0f - ( yy + zz );
	mat[ 0 ][ 1 ] = xy - wz;
	mat[ 0 ][ 2 ] = xz + wy;

	mat[ 1 ][ 0 ] = xy + wz;
	mat[ 1 ][ 1 ] = 1.0f - ( xx + zz );
	mat[ 1 ][ 2 ] = yz - wx;

	mat[ 2 ][ 0 ] = xz - wy;
	mat[ 2 ][ 1 ] = yz + wx;
	mat[ 2 ][ 2 ] = 1.0f - ( xx + yy );

	return mat;
#if 0
	f32 x2 = q.x * q.x;
	f32 y2 = q.y * q.y;
	f32 z2 = q.z * q.z;
	f32 xy = q.x * q.y;
	f32 xz = q.x * q.z;
	f32 yz = q.y * q.z;
	f32 wx = q.w * q.x;
	f32 wy = q.w * q.y;
	f32 wz = q.w * q.z;
	return
	{
		1.0f - 2.0f * (y2 + z2),  2.0f * (xy - wz),         2.0f * (xz + wy),
		2.0f * (xy + wz),         1.0f - 2.0f * (x2 + z2),  2.0f * (yz - wx),
		2.0f * (xz - wy),         2.0f * (yz + wx),         1.0f - 2.0f * (x2 + y2),
	};
#endif
}

EulerAngles ToAngles(Quaternion q)
{
	EulerAngles result;
	auto singularityTest = q.z * q.x - q.w * q.y;
	auto yawY = 2.0f * (q.w * q.z + q.x * q.y);
	auto yawX = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	auto singularityThreshold = 0.4999995f;
	if (singularityTest < -singularityThreshold)
	{
		result.roll = -PI / 2;
		result.yaw = Atan2(yawY, yawX);
		result.pitch = -result.yaw - (2.0f * Atan2(q.x, q.w));
	}
	else if (singularityTest > singularityThreshold)
	{
		result.roll = PI / 2;
		result.yaw = Atan2(yawY, yawX);
		result.pitch = result.yaw - (2.0f * Atan2(q.x, q.w));
	}
	else
	{
		result.roll = Asin(2.0f * singularityTest);
		result.yaw = Atan2(yawY, yawX);
		result.pitch = Atan2(-2.0f * (q.w * q.x + q.y * q.z), 1.0f - 2.0f * (q.x * q.x + q.y * q.y));
	}
	return result;
}

V3 Lerp(V3 start, V3 end, f32 t)
{
	return start + t * (end - start);
}

f32 NormalSquared(Quaternion q)
{
	return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w; 
}

Quaternion Normalize(Quaternion q)
{
	auto inverseNormalSquared = 1.0f / SquareRoot(NormalSquared(q));
	return
	{
		q.x * inverseNormalSquared,
		q.y * inverseNormalSquared,
		q.z * inverseNormalSquared,
		q.w * inverseNormalSquared
	};
}

Quaternion Lerp(Quaternion a, Quaternion b, f32 t)
{
	f32 dotProduct = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	f32 oneMinusT = 1.0f - t;
	Quaternion result;
	if (dotProduct < 0.0f)
	{
		result.x = oneMinusT * a.x + t * -b.x;
		result.y = oneMinusT * a.y + t * -b.y;
		result.z = oneMinusT * a.z + t * -b.z;
		result.w = oneMinusT * a.w + t * -b.w;
	}
	else
	{
		result.x = oneMinusT * a.x + t * b.x;
		result.y = oneMinusT * a.y + t * b.y;
		result.z = oneMinusT * a.z + t * b.z;
		result.w = oneMinusT * a.w + t * b.w;
	}
	Normalize(result);
	return result;
}

V3 CalculateRightVector(const Quaternion &q)
{
	return
	{
		1 - 2 * (q.y * q.y + q.z * q.z),
		2 * (q.x * q.y + q.w * q.z),
		2 * (q.x * q.z - q.w * q.y),
	};
}

V3 CalculateForwardVector(const Quaternion &q)
{
	return
	{
		2 * (q.x * q.y - q.w * q.z),
		1 - 2 * (q.x * q.x + q.z * q.z),
		2 * (q.y * q.z + q.w * q.x),
	};
}

V3 CalculateUpVector(const Quaternion &q)
{
	return
	{
		2 * (q.x * q.z + q.w * q.y),
		2 * (q.y * q.z - q.w * q.x),
		1 - 2 * (q.x * q.x + q.y * q.y),
	};
}

Quaternion ToQuaternion(EulerAngles angles)
{
	auto pitchNoWinding = Fmod(angles.pitch, 2 * PI);
	auto yawNoWinding = Fmod(angles.yaw, 2 * PI);
	auto rollNoWinding = Fmod(angles.roll, 2 * PI);

	f32 sp, sy, sr;
	f32 cp, cy, cr;
	SinCos(pitchNoWinding * 0.5f, &sp, &cp);
	SinCos(yawNoWinding * 0.5f, &sy, &cy);
	SinCos(rollNoWinding * 0.5f, &sr, &cr);

	return
	{
		cp * sr * sy - sp * cr * cy,
		-cp * sr * cy - sp * cr * sy,
		cp * cr * sy - sp * sr * cy,
		cp * cr * cy + sp * sr * sy,
	};
}
