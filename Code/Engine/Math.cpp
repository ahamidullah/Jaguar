// @TODO: Move things into methods.
// operators
// Normal
// Abs
// Conversions
// Cleanup result.

#include "Math.h"
#include "Basic/Log.h"

void PrintM4Actual(String name, M4 m)
{
	ConsolePrint("%k:\n", name);
	for (auto i = 0; i < 4; i += 1)
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
	return sqrtf(x);
}

f32 Tan(f32 x)
{
	return tanf(x);
}

f32 Atan2(f32 x, f32 y)
{
	return atan2f(x, y);
}

f32 Sin(f32 x)
{
	return sinf(x);
}

f32 Asin(f32 x)
{
	return asinf(x);
}

f32 Cos(f32 x)
{
	return cosf(x);
}

f32 Acos(f32 x)
{
	return acosf(x);
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

V2 operator-(V2 a, V2 b)
{
	return
	{
		a.x - b.x,
		a.y - b.y,
	};
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

bool operator==(V3 a, V3 b)
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

bool V3::NotNAN()
{
	return (this->x != NAN) && (this->y != NAN) && (this->z != NAN);
}

bool V3::NotZero()
{
	return (this->x != 0.0f) || (this->y != 0.0f) || (this->z != 0.0f);
}

f32 V3::LengthSquared()
{
	return this->x * this->x + this->y * this->y + this->z * this->z;
}

f32 V3::Length()
{
	return SquareRoot(this->x * this->x + this->y * this->y + this->z * this->z);
}

V3 V3::Normal()
{
	auto r = (1 / this->Length()) * *this;
	Assert(r.NotNAN());
	return r;
}

V3 V3::Abs()
{
	return
	{
		::Abs(this->x),
		::Abs(this->y),
		::Abs(this->z),
	};
}

V4 V3::ToV4(f32 w)
{
	return
	{
		this->x,
		this->y,
		this->z,
		w,
	};
}

void UprightOrthonormalBasis(V3 forward, V3 *right, V3 *up)
{
	if (forward.Abs() == WorldUpVector)
	{
		*right = WorldRightVector;
	}
	else
	{
		*right = VectorCrossProduct(forward, WorldUpVector).Normal();
	}
	*up = VectorCrossProduct(*right, forward).Normal();
}

f32 VectorDotProduct(V3 a, V3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

V3 VectorCrossProduct(V3 a, V3 b)
{
	auto r = V3
	{
		a.y * b.z - b.y * a.z,
		a.z * b.x - b.z * a.x,
		a.x * b.y - b.x * a.y,
	};
	Assert(r.NotZero());
	return r;
}

V3 VectorLerp(V3 start, V3 end, f32 t)
{
	return start + t * (end - start);
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

V3 operator*(M3 m, V3 v)
{
	return
	{
		v.x*m[0][0] + v.y*m[0][1] + v.z*m[0][2],
		v.x*m[1][0] + v.y*m[1][1] + v.z*m[1][2],
		v.x*m[2][0] + v.y*m[2][1] + v.z*m[2][2],
	};
}

M3 NewMatrix(V3 col1, V3 col2, V3 col3)
{
	return
	{
		col1.x, col2.x, col3.x,
		col1.y, col2.y, col3.y,
		col1.z, col2.z, col3.z,
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


V4 &M4::operator[](int i)
{
	return *(V4 *)m[i];
}

M4 operator*(M4 a, M4 b)
{
	auto result = M4{};
	for (auto i = 0; i < 4; i += 1)
	{
		for (auto j = 0; j < 4; j += 1)
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

void M4::SetScale(V3 s)
{
	for (auto i = 0; i < 4; i += 1)
	{
		(*this)[i][0] *= s.x;
		(*this)[i][1] *= s.y;
		(*this)[i][2] *= s.z;
	}
}

void M4::SetRotation(M3 r)
{
	(*this)[0][0] = r[0][0];
	(*this)[0][1] = r[0][1];
	(*this)[0][2] = r[0][2];
	(*this)[1][0] = r[1][0];
	(*this)[1][1] = r[1][1];
	(*this)[1][2] = r[1][2];
	(*this)[2][0] = r[2][0];
	(*this)[2][1] = r[2][1];
	(*this)[2][2] = r[2][2];
}

void M4::SetTranslation(V3 t)
{
	(*this)[0][3] = t.x;
	(*this)[1][3] = t.y;
	(*this)[2][3] = t.z;
}

M4 M4::Transpose()
{
	return
	{
		(*this)[0][0], (*this)[1][0], (*this)[2][0], (*this)[3][0],
		(*this)[0][1], (*this)[1][1], (*this)[2][1], (*this)[3][1],
		(*this)[0][2], (*this)[1][2], (*this)[2][2], (*this)[3][2],
		(*this)[0][3], (*this)[1][3], (*this)[2][3], (*this)[3][3],
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

M4 InfinitePerspectiveProjectionMatrix(f32 near, f32 verticalFOV, f32 aspectRatio)
{
	// Assumes camera space is looking down positive z.
	#ifdef VulkanBuild
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
	#else
		Abort("Math", "Unknown render API.");
		return M4{};
	#endif
}

M4 OrthographicProjectionMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
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
	forward = forward.Normal();
	V3 right, up;
	UprightOrthonormalBasis(forward, &right, &up);
	return
	{
		right.x,      right.y,      right.z,      -VectorDotProduct(right, position),
		up.x,         up.y,         up.z,         -VectorDotProduct(up, position),
		forward.x,    forward.y,    forward.z,    -VectorDotProduct(forward, position),
		0,            0,            0,            1.0f,
	};
}

Quaternion M3::ToQuaternion()
{
	auto trace = (*this)[0][0] + (*this)[1][1] + (*this)[2][2];
	if (trace > 0)
	{
		auto s = 0.5f / SquareRoot(trace + 1.0f);
		return
		{
			.x = ((*this)[2][1] - (*this)[1][2]) * s,
			.y = ((*this)[0][2] - (*this)[2][0]) * s,
			.z = ((*this)[1][0] - (*this)[0][1]) * s,
			.w = 0.25f / s,
		};
	}
	if ((*this)[0][0] > (*this)[1][1] && (*this)[0][0] > (*this)[2][2])
	{
		auto s = 2.0f * SquareRoot(1.0f + (*this)[0][0] - (*this)[1][1] - (*this)[2][2]);
		return
		{
			.x = 0.25f * s,
			.y = ((*this)[0][1] + (*this)[1][0] ) / s,
			.z = ((*this)[0][2] + (*this)[2][0] ) / s,
			.w = ((*this)[2][1] - (*this)[1][2] ) / s,
		};
	}
	if ((*this)[1][1] > (*this)[2][2])
	{
		auto s = 2.0f * SquareRoot(1.0f + (*this)[1][1] - (*this)[0][0] - (*this)[2][2]);
		return
		{
			.x = ((*this)[0][1] + (*this)[1][0] ) / s,
			.y = 0.25f * s,
			.z = ((*this)[1][2] + (*this)[2][1] ) / s,
			.w = ((*this)[0][2] - (*this)[2][0] ) / s,
		};
	}
	auto s = 2.0f * SquareRoot(1.0f + (*this)[2][2] - (*this)[0][0] - (*this)[1][1]);
	return
	{
		.x = ((*this)[0][2] + (*this)[2][0] ) / s,
		.y = ((*this)[1][2] + (*this)[2][1] ) / s,
		.z = 0.25f * s,
		.w = ((*this)[1][0] - (*this)[0][1] ) / s,
	};
}

EulerAngles M3::ToAngles()
{
	auto s = SquareRoot((*this)[0][0] * (*this)[0][0] + (*this)[0][1] * (*this)[0][1]);
	if (s > FloatEpsilon)
	{
		return
		{
			.pitch = Atan2((*this)[1][2], (*this)[2][2]),
			.yaw = Atan2((*this)[0][1], (*this)[0][0]),
			.roll = -Atan2((*this)[0][2], s),
		};
	}
	auto r = EulerAngles
	{
		.pitch = 0.0f,
		.yaw = -Atan2((*this)[1][0], (*this)[1][1]),
	};
	if ((*this)[0][2] < 0.0f)
	{
		r.roll = 90.0f;
	}
	else
	{
		r.roll = -90.0f;
	}
	return r;
}

Quaternion NewQuaternion(V3 forward)
{
	forward = forward.Normal();
	auto right = V3{};
	auto up = V3{};
	UprightOrthonormalBasis(forward, &right, &up);
	return NewMatrix(right, forward, up).ToQuaternion().Normal();
}

Quaternion NewQuaternionFromEuler(EulerAngles ea)
{
	return Quaternion{};
}

Quaternion NewQuaternionFromAxisAngle(AxisAngle aa)
{
	auto imaginary = Sin(aa.angle / 2.0f) * aa.axis;
	return
	{
		.x = imaginary.x,
		.y = imaginary.y,
		.z = imaginary.z,
		.w = Cos(aa.angle / 2.0f),
	};
}

Quaternion operator*(Quaternion a, Quaternion b)
{
	return
	{
        .x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        .y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        .z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        .w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
	};
}

Quaternion operator*(f32 s, Quaternion q)
{
	return
	{
		.x = s * q.x,
		.y = s * q.y,
		.z = s * q.z,
		.w = s * q.w,
	};
}

Quaternion Quaternion::Conjugate()
{
	return
	{
		.x = -this->x,
		.y = -this->y,
		.z = -this->z,
		.w = this->w
	};
}

V3 Quaternion::RotateVector(V3 v)
{
	auto r = (*this) * Quaternion{v.x, v.y, v.z, 0.0} * this->Conjugate();
	return
	{
		.x = r.x,
		.y = r.y,
		.z = r.z,
	};
}

M3 Quaternion::Matrix()
{
	auto x2 = this->x + this->x;
	auto y2 = this->y + this->y;
	auto z2 = this->z + this->z;
	auto xx = this->x * x2;
	auto xy = this->x * y2;
	auto xz = this->x * z2;
	auto yy = this->y * y2;
	auto yz = this->y * z2;
	auto zz = this->z * z2;
	auto wx = this->w * x2;
	auto wy = this->w * y2;
	auto wz = this->w * z2;
	return 
	{
		1.0f - ( yy + zz ),  xy - wz,             xz + wy,
		xy + wz,             1.0f - ( xx + zz ),  yz - wx,
		xz - wy,             yz + wx,             1.0f - ( xx + yy ),
	};
#if 0
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
#endif
}

EulerAngles Quaternion::Euler()
{
	auto singularityTest = this->z * this->x - this->w * this->y;
	auto yawY = 2.0f * (this->w * this->z + this->x * this->y);
	auto yawX = 1.0f - 2.0f * (this->y * this->y + this->z * this->z);
	auto yaw = Atan2(yawY, yawX);
	auto singularityThreshold = 0.4999995f;
	if (singularityTest < -singularityThreshold)
	{
		return
		{
			.pitch = -yaw - (2.0f * Atan2(this->x, this->w)),
			.yaw = yaw,
			.roll = -Pi / 2,
		};
	}
	if (singularityTest > singularityThreshold)
	{
		return
		{
			.pitch = yaw - (2.0f * Atan2(this->x, this->w)),
			.yaw = yaw,
			.roll = Pi / 2,
		};
	}
	return
	{
		.pitch = Atan2(-2.0f * (this->w * this->x + this->y * this->z), 1.0f - 2.0f * (this->x * this->x + this->y * this->y)),
		.yaw = yaw,
		.roll = Asin(2.0f * singularityTest),
	};
}

f32 Quaternion::NormalSquared()
{
	return this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w; 
}

Quaternion Quaternion::Normal()
{
	auto inverseNormalSquared = 1.0f / SquareRoot(this->NormalSquared());
	return
	{
		this->x * inverseNormalSquared,
		this->y * inverseNormalSquared,
		this->z * inverseNormalSquared,
		this->w * inverseNormalSquared
	};
}

Quaternion QuaternionLerp(Quaternion a, Quaternion b, f32 t)
{
	auto dotProduct = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
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
	return result.Normal();
}

V3 Quaternion::Right()
{
	return
	{
		1 - 2 * (this->y * this->y + this->z * this->z),
		2 * (this->x * this->y + this->w * this->z),
		2 * (this->x * this->z - this->w * this->y),
	};
}

V3 Quaternion::Forward()
{
	return
	{
		2 * (this->x * this->y - this->w * this->z),
		1 - 2 * (this->x * this->x + this->z * this->z),
		2 * (this->y * this->z + this->w * this->x),
	};
}

V3 Quaternion::Up()
{
	return
	{
		2 * (this->x * this->z + this->w * this->y),
		2 * (this->y * this->z - this->w * this->x),
		1 - 2 * (this->x * this->x + this->y * this->y),
	};
}

Quaternion EulerAngles::ToQuaternion()
{
	auto pitchNoWinding = Fmod(this->pitch, 2 * Pi);
	auto yawNoWinding = Fmod(this->yaw, 2 * Pi);
	auto rollNoWinding = Fmod(this->roll, 2 * Pi);
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
