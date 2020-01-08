#define MUL_DEGREES_TO_RADIANS (M_PI / 180.0)
#define MUL_RADIANS_TO_DEGREES (180.0 / M_PI)

#define DEGREES_TO_RADIANS(deg) ((deg) * MUL_DEGREES_TO_RADIANS)
#define RADIANS_TO_DEGREES(rad) ((rad) * MUL_RADIANS_TO_DEGREES)

u32 divide_and_round_up(u32 a, u32 b) {
	return (a + (b - 1)) / b;
}

u8 not_nan(V3 v) {
	return v.x != NAN && v.y != NAN && v.z != NAN;
}

u8 not_zero(V3 v) {
	return v.x != 0.0f || v.y != 0.0f || v.z != 0.0f;
}

V4 v3_to_v4(V3 v3, f32 w) {
	return (V4){
		v3.x,
		v3.y,
		v3.z,
		w,
	};
}

/*
V2 operator-(V2 a, V2 b) {
	return {
		a.x - b.x,
		a.y - b.y,
	};
}

u8 operator==(const V3 &a, const V3 &b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

V3 operator-(V3 v) {
	return {
		-v.x,
		-v.y,
		-v.z,
	};
}

V3 operator*(f32 scalar, V3 v) {
	return {
		scalar * v.x,
		scalar * v.y,
		scalar * v.z,
	};
}

V3 operator/(V3 v, f32 scalar) {
	scalar = 1.0f / scalar;
	return {
		scalar * v.x,
		scalar * v.y,
		scalar * v.z,
	};
}

V3 operator+(V3 a, V3 b) {
	return {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
	};
}

V3 operator-(V3 a, V3 b) {
	return {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
	};
}

V3 &operator+=(V3 &a, V3 b) {
	a = a + b;
	return a;
}

V3 &operator-=(V3 &a, V3 b) {
	a = a - b;
	return a;
}

// @TODO: USE A SWITCH!
f32 V3::operator[](int i) const {
	ASSERT(i >= 0 && i <= 2);
	if (i == 0)  return x;
	if (i == 1)  return y;
	return z;
}

// @TODO: USE A SWITCH!
f32 &V3::operator[](int i) {
	ASSERT(i >= 0 && i <= 2);
	if (i == 0)  return x;
	if (i == 1)  return y;
	return z;
}

// @TODO: USE A SWITCH!
f32 &V2::operator[](int i) {
	ASSERT(i >= 0 && i <= 1);
	if (i == 0)  return x;
	return y;
}

V4 operator/(V4 v, f32 s) {
	return {v.x / s, v.y / s, v.z / s, v.w / s};
}

V4 operator+(V4 a, V4 b) {
	return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

V4 &operator/=(V4 &v, f32 s) {
	v = v / s;
	return v;
}

V4 operator*(f32 s, V4 v) {
	return { v.x * s, v.y * s, v.z * s, v.w * s };
}

// @TODO: USE A SWITCH!
f32 V4::operator[](int i) const {
	ASSERT(i >= 0 && i <= 3);
	if (i == 0)  return x;
	if (i == 1)  return y;
	if (i == 2)  return z;
	return w;
}

// @TODO: USE A SWITCH!
f32 &V4::operator[](int i) {
	ASSERT(i >= 0 && i <= 3);
	if (i == 0)  return x;
	if (i == 1)  return y;
	if (i == 2)  return z;
	return w;
}

V3 &M3::operator[](int i) {
	return *(V3 *)m[i];
}

V4 &M4::operator[](int i) {
	return *(V4 *)m[i];
}
*/

M4 m4_identity() {
	return (M4){{
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f},
	}};
}

/*
M4 operator*(M4 a, M4 b) {
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

M4 operator*(f32 s, M4 m) {
	return {
		s * m.m[0][0], s * m.m[0][1], s * m.m[0][2], s * m.m[0][3],
		s * m.m[1][0], s * m.m[1][1], s * m.m[1][2], s * m.m[1][3],
		s * m.m[2][0], s * m.m[2][1], s * m.m[2][2], s * m.m[2][3],
		s * m.m[3][0], s * m.m[3][1], s * m.m[3][2], s * m.m[3][3]
	};
}

V4 operator*(M4 m, V4 v) {
	return {
		v.x*m[0][0] + v.y*m[0][1] + v.z*m[0][2]  + v.w*m[0][3],
		v.x*m[1][0] + v.y*m[1][1] + v.z*m[1][2]  + v.w*m[1][3],
		v.x*m[2][0] + v.y*m[2][1] + v.z*m[2][2]  + v.w*m[2][3],
		v.x*m[3][0] + v.y*m[3][1] + v.z*m[3][2]  + v.w*m[3][3],
	};
}
*/

f32 length_squared(V3 v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

f32 vector_length(V3 v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

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

V3 normalize(V3 v) {
	V3 result = scale_v3(1 / vector_length(v), v);
	Assert(not_nan(result));
	return result;
}

f32 dot_product(V3 a, V3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

V3 cross_product(V3 a, V3 b) {
	V3 result = (V3){
		a.y*b.z - b.y*a.z,
		a.z*b.x - b.z*a.x,
		a.x*b.y - b.x*a.y,
	};
	Assert(not_zero(result));
	return result;
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

void set_matrix_rotation(M4 *m, M3 rotation) {
	m->m[0][0] = rotation.m[0][0];
	m->m[0][1] = rotation.m[0][1];
	m->m[0][2] = rotation.m[0][2];
	m->m[1][0] = rotation.m[1][0];
	m->m[1][1] = rotation.m[1][1];
	m->m[1][2] = rotation.m[1][2];
	m->m[2][0] = rotation.m[2][0];
	m->m[2][1] = rotation.m[2][1];
	m->m[2][2] = rotation.m[2][2];
}

void set_matrix_translation(M4 *m, V3 translation) {
	m->m[0][3] = translation.x;
	m->m[1][3] = translation.y;
	m->m[2][3] = translation.z;
}

void set_matrix_scale(M4 *m, V3 scale) {
	for (s32 i = 0; i < 4; i++) {
		m->m[i][0] *= scale.x;
		m->m[i][1] *= scale.y;
		m->m[i][2] *= scale.z;
	}
}

V3 get_matrix_translation(M4 m) {
	return (V3){m.m[0][3], m.m[1][3], m.m[2][3]};
}

M4 transpose_matrix(M4 m) {
	return (M4){{
		{m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0]},
		{m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1]},
		{m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2]},
		{m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]},
	}};
}

M4 perspective_projection(f32 fovy, f32 aspect_ratio, f32 near, f32 far) {
	f32 focal_length = 1.0f / tanf(fovy / 2.0f);
	return (M4){{
		{focal_length / aspect_ratio,   0.0f,            0.0f,                 0.0f},
		{0.0f,                          -focal_length,   0.0f,                 0.0f},
		{0.0f,                          0.0f,            far / (near - far),   -(far * near) / (far - near)},
		{0.0f,                          0.0f,            -1.0f,                0.0f},
	}};
}

// Assumes near is 0.01f and far is infinity.
M4 infinite_perspective_projection(f32 fovy, f32 aspect_ratio) {
	const f32 near = 0.01f;
	f32 focal_length = 1 / tan(fovy / 2);
	return (M4){{
		{focal_length,   0.0f,                           0.0f,    0.0f},
		{0.0f,           -focal_length * aspect_ratio,   0.0f,    0.0f},
		{0.0f,           0.0f,                           -1.0f,   -2.0f * near},
		{0.0f,           0.0f,                           -1.0f,   0.0f},
	}};
}

// Assumes near is -1.0f and far is 1.0f.
/*
M4 orthographic_projection(f32 left, f32 right, f32 bottom, f32 top) {
	return (M4){{
		{2.0f / (right - left),   0.0f,                     0.0f,    -(right + left) / (right - left)},
		{0.0f,                    -2.0f / (top - bottom),   0.0f,    -(top + bottom) / (top - bottom)},
		{0.0f,                    0.0f,                     -1.0f,   0.0f},
		{0.0f,                    0.0f,                     0.0f,    1.0f},
	}};
}
*/

M4 orthographic_projection(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
	return (M4){{
		{2.0f / (right - left),   0.0f,                     0.0f,                   -(right + left) / (right - left)},
	    {0.0f,                    -2.0f / (top - bottom),   0.0f,                   -(top + bottom) / (top - bottom)},
	    {0.0f,                    0.0f,                     -1.0f / (far - near),   -near / (far - near)},
	    {0.0f,                    0.0f,                     0.0f,                   1.0f},
	}};
}

M4 view_matrix(V3 position, V3 forward, V3 side, V3 up) {
	return (M4){{
		{side.x,       side.y,       side.z,       -dot_product(side, position)},
		{up.x,         up.y,         up.z,         -dot_product(up, position)},
		{-forward.x,   -forward.y,   -forward.z,   dot_product(forward, position)},
		{0,            0,            0,            1.0f},
	}};
}

M4 look_at(V3 position, V3 target, V3 world_up) {
	// @TODO: Move this out to a create basis function.
	V3 forward = normalize(subtract_v3(target, position));
	V3 side = normalize(cross_product(forward, world_up));
	V3 up = normalize(cross_product(side, forward));

	return view_matrix(position, forward, side, up);
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
