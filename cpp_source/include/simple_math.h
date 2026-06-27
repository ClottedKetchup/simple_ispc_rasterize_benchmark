#ifndef SIMPLE_MATH_H
#define SIMPLE_MATH_H

#define SIMPLE_MATH_PI 3.1415926535897932384626433832795
#define SIMPLE_MATH_FMAX 3.402823466e+38F

#define EMPTY_INDEX_32 0xFFFFFFFF


#if defined (ISPC) || defined(__ISPC__)
	#define key_uniform uniform
	#define key_varying varying
#else
	#define key_uniform
	#define key_varying
#endif

#if defined (ISPC) || defined(__ISPC__)

#else
#include <math.h>
#include <cstdint>
#endif


// note: by test, ispc support these usage:
// 
//	static inline key_uniform int test_qualifier(key_uniform int x, key_uniform int y)
//	{
//		return x + y;
//	}
//
//	static inline key_varying int test_qualifier(key_varying int x, key_varying int y)
//	{
//		return x + y;
//	}
//
//	static key_uniform Float3 test_vec_add(const key_uniform Float3& a, const key_uniform Float3& b)
//	{
//		key_uniform Float3 result;
//		result.v[0] = a.v[0] + b.v[0];
//		result.v[1] = a.v[1] + b.v[1];
//		result.v[2] = a.v[2] + b.v[2];
//
//		return result;
//	}
//
//	static key_varying Float3 test_vec_add(const key_varying Float3& a, const key_varying Float3& b)
//	{
//		key_varying Float3 result;
//		result.v[0] = a.v[0] + b.v[0];
//		result.v[1] = a.v[1] + b.v[1];
//		result.v[2] = a.v[2] + b.v[2];
//
//		return result;
//	}
//
//	static void test_set_vec(key_uniform Float3& color, key_uniform Float3 target)
//	{
//		color.v[0] = target.v[0];
//		color.v[1] = target.v[1];
//		color.v[2] = target.v[2];
//	}
//
//	static void test_set_vec(key_varying Float3& color, key_varying Float3 target)
//	{
//		color.v[0] = target.v[0];
//		color.v[1] = target.v[1];
//		color.v[2] = target.v[2];
//	}

struct Float2
{
	float v[2];
};

static inline key_varying float get_x(const key_varying Float2& v)
{
	return v.v[0];
}
static inline key_varying float get_y(const key_varying Float2& v)
{
	return v.v[1];
}

static inline key_varying Float2 make_Float2(const key_varying float x, const key_varying float y)
{
	key_varying Float2 v;
	v.v[0] = x;
	v.v[1] = y;
	return v;
}

static inline key_varying Float2 make_Float2(const key_varying float f)
{
	key_varying Float2 v;
	v.v[0] = f;
	v.v[1] = f;
	return v;
}

static inline key_varying Float2 operator+(const key_varying Float2& a, const key_varying Float2& b) {
	return make_Float2(a.v[0] + b.v[0], a.v[1] + b.v[1]);
}
static inline key_varying Float2 operator-(const key_varying Float2& a, const key_varying Float2& b) {
	return make_Float2(a.v[0] - b.v[0], a.v[1] - b.v[1]);
}
static inline key_varying Float2 operator*(const key_varying Float2& a, const key_varying Float2& b) {
	return make_Float2(a.v[0] * b.v[0], a.v[1] * b.v[1]);
}
static inline key_varying Float2 operator/(const key_varying Float2& a, const key_varying Float2& b) {
	return make_Float2(a.v[0] / b.v[0], a.v[1] / b.v[1]);
}

static inline key_varying Float2 operator+(const key_varying Float2& v, const key_varying float s) {
	return make_Float2(v.v[0] + s, v.v[1] + s);
}
static inline key_varying Float2 operator+(const key_varying float s, const key_varying Float2& v) {
	return make_Float2(s + v.v[0], s + v.v[1]);
}
static inline key_varying Float2 operator-(const key_varying Float2& v, const key_varying float s) {
	return make_Float2(v.v[0] - s, v.v[1] - s);
}
static inline key_varying Float2 operator-(const key_varying float s, const key_varying Float2& v) {
	return make_Float2(s - v.v[0], s - v.v[1]);
}
static inline key_varying Float2 operator*(const key_varying Float2& v, const key_varying float s) {
	return make_Float2(v.v[0] * s, v.v[1] * s);
}
static inline key_varying Float2 operator*(const key_varying float s, const key_varying Float2& v) {
	return make_Float2(s * v.v[0], s * v.v[1]);
}
static inline key_varying Float2 operator/(const key_varying Float2& v, const key_varying float s) {
	return make_Float2(v.v[0] / s, v.v[1] / s);
}
static inline key_varying Float2 operator/(const key_varying float s, const key_varying Float2& v) {
	return make_Float2(s / v.v[0], s / v.v[1]);
}

static inline key_varying float cross(const key_varying Float2& a, const key_varying Float2& b)
{
	return a.v[0] * b.v[1] - a.v[1] * b.v[0];
}

static inline key_varying Float2 normalize(const key_varying Float2& v)
{
	key_varying float len_sq = v.v[0] * v.v[0] + v.v[1] * v.v[1];
	key_varying float inv_len = (len_sq > 0.0f) ? 1.0f / sqrt(len_sq) : 0.0f;
	
	key_varying Float2 result;
	result.v[0] = v.v[0] * inv_len;
	result.v[1] = v.v[1] * inv_len;

	return result;
}

static inline key_varying float dot(const key_varying Float2& a, const key_varying Float2& b)
{
	return a.v[0] * b.v[0] + a.v[1] * b.v[1];
}

#if defined (ISPC) || defined(__ISPC__)
static inline key_uniform float get_x(const key_uniform Float2& v)
{
	return v.v[0];
}
static inline key_uniform float get_y(const key_uniform Float2& v)
{
	return v.v[1];
}

static inline key_uniform Float2 make_Float2(const key_uniform float x, const key_uniform float y)
{
	key_uniform Float2 v;
	v.v[0] = x;
	v.v[1] = y;
	return v;
}

static inline key_uniform Float2 make_Float2(const key_uniform float f)
{
	key_uniform Float2 v;
	v.v[0] = f;
	v.v[1] = f;
	return v;
}

static inline key_uniform Float2 operator+(const key_uniform Float2& a, const key_uniform Float2& b)
{
	return make_Float2(a.v[0] + b.v[0], a.v[1] + b.v[1]);
}
static inline key_uniform Float2 operator-(const key_uniform Float2& a, const key_uniform Float2& b)
{
	return make_Float2(a.v[0] - b.v[0], a.v[1] - b.v[1]);
}
static inline key_uniform Float2 operator*(const key_uniform Float2& a, const key_uniform Float2& b)
{
	return make_Float2(a.v[0] * b.v[0], a.v[1] * b.v[1]);
}
static inline key_uniform Float2 operator/(const key_uniform Float2& a, const key_uniform Float2& b)
{
	return make_Float2(a.v[0] / b.v[0], a.v[1] / b.v[1]);
}

static inline key_uniform Float2 operator+(const key_uniform Float2& v, const key_uniform float s)
{
	return make_Float2(v.v[0] + s, v.v[1] + s);
}
static inline key_uniform Float2 operator+(const key_uniform float s, const key_uniform Float2& v)
{
	return make_Float2(s + v.v[0], s + v.v[1]);
}
static inline key_uniform Float2 operator-(const key_uniform Float2& v, const key_uniform float s)
{
	return make_Float2(v.v[0] - s, v.v[1] - s);
}
static inline key_uniform Float2 operator-(const key_uniform float s, const key_uniform Float2& v)
{
	return make_Float2(s - v.v[0], s - v.v[1]);
}
static inline key_uniform Float2 operator*(const key_uniform Float2& v, const key_uniform float s)
{
	return make_Float2(v.v[0] * s, v.v[1] * s);
}
static inline key_uniform Float2 operator*(const key_uniform float s, const key_uniform Float2& v)
{
	return make_Float2(s * v.v[0], s * v.v[1]);
}
static inline key_uniform Float2 operator/(const key_uniform Float2& v, const key_uniform float s)
{
	return make_Float2(v.v[0] / s, v.v[1] / s);
}
static inline key_uniform Float2 operator/(const key_uniform float s, const key_uniform Float2& v)
{
	return make_Float2(s / v.v[0], s / v.v[1]);
}

static inline key_uniform float cross(const key_uniform Float2& a, const key_uniform Float2& b)
{
	return a.v[0] * b.v[1] - a.v[1] * b.v[0];
}

static inline key_uniform Float2 normalize(const key_uniform Float2& v)
{
	key_uniform float len_sq = v.v[0] * v.v[0] + v.v[1] * v.v[1];
	key_uniform float inv_len = (len_sq > 0.0f) ? 1.0f / sqrt(len_sq) : 0.0f;

	key_uniform Float2 result;
	result.v[0] = v.v[0] * inv_len;
	result.v[1] = v.v[1] * inv_len;
	return result;
}

static inline key_uniform float dot(const key_uniform Float2& a, const key_uniform Float2& b)
{
	return a.v[0] * b.v[0] + a.v[1] * b.v[1];
}
#endif



struct Float3
{
	float v[3];
};

static inline key_varying float get_x(const key_varying Float3& v)
{
	return v.v[0];
}
static inline key_varying float get_y(const key_varying Float3& v)
{
	return v.v[1];
}
static inline key_varying float get_z(const key_varying Float3& v)
{
	return v.v[2];
}

static inline key_varying Float3 make_Float3(const key_varying float x, const key_varying float y, const key_varying float z)
{
	key_varying Float3 v;
	v.v[0] = x;
	v.v[1] = y;
	v.v[2] = z;
	return v;
}

static inline key_varying Float3 make_Float3(const key_varying float f)
{
	key_varying Float3 v;
	v.v[0] = f;
	v.v[1] = f;
	v.v[2] = f;
	return v;
}

static inline key_varying Float3 operator+(const key_varying Float3& a, const key_varying Float3& b) {
	return make_Float3(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]);
}
static inline key_varying Float3 operator-(const key_varying Float3& a, const key_varying Float3& b) {
	return make_Float3(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2]);
}
static inline key_varying Float3 operator*(const key_varying Float3& a, const key_varying Float3& b) {
	return make_Float3(a.v[0] * b.v[0], a.v[1] * b.v[1], a.v[2] * b.v[2]);
}
static inline key_varying Float3 operator/(const key_varying Float3& a, const key_varying Float3& b) {
	return make_Float3(a.v[0] / b.v[0], a.v[1] / b.v[1], a.v[2] / b.v[2]);
}

static inline key_varying Float3 operator+(const key_varying Float3& v, const key_varying float s)
{
	return make_Float3(v.v[0] + s, v.v[1] + s, v.v[2] + s);
}
static inline key_varying Float3 operator+(const key_varying float s, const key_varying Float3& v)
{
	return make_Float3(s + v.v[0], s + v.v[1], s + v.v[2]);
}
static inline key_varying Float3 operator-(const key_varying Float3& v, const key_varying float s)
{
	return make_Float3(v.v[0] - s, v.v[1] - s, v.v[2] - s);
}
static inline key_varying Float3 operator-(const key_varying float s, const key_varying Float3& v)
{
	return make_Float3(s - v.v[0], s - v.v[1], s - v.v[2]);
}
static inline key_varying Float3 operator*(const key_varying Float3& v, const key_varying float s)
{
	return make_Float3(v.v[0] * s, v.v[1] * s, v.v[2] * s);
}
static inline key_varying Float3 operator*(const key_varying float s, const key_varying Float3& v)
{
	return make_Float3(s * v.v[0], s * v.v[1], s * v.v[2]);
}
static inline key_varying Float3 operator/(const key_varying Float3& v, const key_varying float s)
{
	return make_Float3(v.v[0] / s, v.v[1] / s, v.v[2] / s);
}
static inline key_varying Float3 operator/(const key_varying float s, const key_varying Float3& v)
{
	return make_Float3(s / v.v[0], s / v.v[1], s / v.v[2]);
}

static inline key_varying Float3 cross(const key_varying Float3& a, const key_varying Float3& b)
{
	key_varying Float3 result;
	result.v[0] = a.v[1] * b.v[2] - a.v[2] * b.v[1];
	result.v[1] = a.v[2] * b.v[0] - a.v[0] * b.v[2];
	result.v[2] = a.v[0] * b.v[1] - a.v[1] * b.v[0];
	return result;
}

static inline key_varying Float3 normalize(const key_varying Float3& v)
{
	key_varying float len_sq = v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2];
	key_varying float inv_len = (len_sq > 0.0f) ? 1.0f / sqrt(len_sq) : 0.0f;
	key_varying Float3 result;
	result.v[0] = v.v[0] * inv_len;
	result.v[1] = v.v[1] * inv_len;
	result.v[2] = v.v[2] * inv_len;
	return result;
}

static inline key_varying float length(const key_varying Float3& v)
{
	key_varying float len_sq = v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2];
	return sqrt(len_sq);
}

static inline key_varying float dot(const key_varying Float3& a, const key_varying Float3& b)
{
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

#if defined (ISPC) || defined(__ISPC__)
static inline key_uniform float get_x(const key_uniform Float3& v)
{
	return v.v[0];
}
static inline key_uniform float get_y(const key_uniform Float3& v)
{
	return v.v[1];
}
static inline key_uniform float get_z(const key_uniform Float3& v)
{
	return v.v[2];
}

static inline key_uniform Float3 make_Float3(const key_uniform float x, const key_uniform float y, const key_uniform float z)
{
	key_uniform Float3 v;
	v.v[0] = x;
	v.v[1] = y;
	v.v[2] = z;
	return v;
}

static inline key_uniform Float3 make_Float3(const key_uniform float f)
{
	key_uniform Float3 v;
	v.v[0] = f;
	v.v[1] = f;
	v.v[2] = f;
	return v;
}

static inline key_uniform Float3 operator+(const key_uniform Float3& a, const key_uniform Float3& b)
{
	return make_Float3(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]);
}
static inline key_uniform Float3 operator-(const key_uniform Float3& a, const key_uniform Float3& b)
{
	return make_Float3(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2]);
}
static inline key_uniform Float3 operator*(const key_uniform Float3& a, const key_uniform Float3& b)
{
	return make_Float3(a.v[0] * b.v[0], a.v[1] * b.v[1], a.v[2] * b.v[2]);
}
static inline key_uniform Float3 operator/(const key_uniform Float3& a, const key_uniform Float3& b)
{
	return make_Float3(a.v[0] / b.v[0], a.v[1] / b.v[1], a.v[2] / b.v[2]);
}

static inline key_uniform Float3 operator+(const key_uniform Float3& v, const key_uniform float s)
{
	return make_Float3(v.v[0] + s, v.v[1] + s, v.v[2] + s);
}
static inline key_uniform Float3 operator+(const key_uniform float s, const key_uniform Float3& v)
{
	return make_Float3(s + v.v[0], s + v.v[1], s + v.v[2]);
}
static inline key_uniform Float3 operator-(const key_uniform Float3& v, const key_uniform float s)
{
	return make_Float3(v.v[0] - s, v.v[1] - s, v.v[2] - s);
}
static inline key_uniform Float3 operator-(const key_uniform float s, const key_uniform Float3& v)
{
	return make_Float3(s - v.v[0], s - v.v[1], s - v.v[2]);
}
static inline key_uniform Float3 operator*(const key_uniform Float3& v, const key_uniform float s)
{
	return make_Float3(v.v[0] * s, v.v[1] * s, v.v[2] * s);
}
static inline key_uniform Float3 operator*(const key_uniform float s, const key_uniform Float3& v)
{
	return make_Float3(s * v.v[0], s * v.v[1], s * v.v[2]);
}
static inline key_uniform Float3 operator/(const key_uniform Float3& v, const key_uniform float s)
{
	return make_Float3(v.v[0] / s, v.v[1] / s, v.v[2] / s);
}
static inline key_uniform Float3 operator/(const key_uniform float s, const key_uniform Float3& v)
{
	return make_Float3(s / v.v[0], s / v.v[1], s / v.v[2]);
}

static inline key_uniform Float3 cross(const key_uniform Float3& a, const key_uniform Float3& b)
{
	key_uniform Float3 result;
	result.v[0] = a.v[1] * b.v[2] - a.v[2] * b.v[1];
	result.v[1] = a.v[2] * b.v[0] - a.v[0] * b.v[2];
	result.v[2] = a.v[0] * b.v[1] - a.v[1] * b.v[0];
	return result;
}

static inline key_uniform Float3 normalize(const key_uniform Float3& v)
{
	key_uniform float len_sq = v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2];
	key_uniform float inv_len = (len_sq > 0.0f) ? 1.0f / sqrt(len_sq) : 0.0f;
	key_uniform Float3 result;
	result.v[0] = v.v[0] * inv_len;
	result.v[1] = v.v[1] * inv_len;
	result.v[2] = v.v[2] * inv_len;
	return result;
}

static inline key_uniform float length(const key_uniform Float3& v)
{
	key_uniform float len_sq = v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2];
	return sqrt(len_sq);
}

static inline key_uniform float dot(const key_uniform Float3& a, const key_uniform Float3& b)
{
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}
#endif



struct Float4 
{
	float v[4];
};

static inline key_varying float get_x(const key_varying Float4& v)
{
	return v.v[0];
}
static inline key_varying float get_y(const key_varying Float4& v)
{
	return v.v[1];
}
static inline key_varying float get_z(const key_varying Float4& v)
{
	return v.v[2];
}
static inline key_varying float get_w(const key_varying Float4& v)
{
	return v.v[3];
}
static inline key_varying Float2 get_xy(const key_varying Float4& v)
{
	return make_Float2(v.v[0], v.v[1]);
}

static inline key_varying Float4 make_Float4(
	const key_varying float x,
	const key_varying float y,
	const key_varying float z,
	const key_varying float w)
{
	key_varying Float4 v;
	v.v[0] = x;
	v.v[1] = y;
	v.v[2] = z;
	v.v[3] = w;
	return v;
}

static inline key_varying Float4 make_Float4(const key_varying float f)
{
	key_varying Float4 v;
	v.v[0] = f;
	v.v[1] = f;
	v.v[2] = f;
	v.v[3] = f;
	return v;
}

static inline key_varying Float4 make_Float4(const key_varying Float3& v3, const key_varying float w)
{
	key_varying Float4 v4;
	v4.v[0] = v3.v[0];
	v4.v[1] = v3.v[1];
	v4.v[2] = v3.v[2];
	v4.v[3] = w;
	return v4;
}

static inline key_varying Float4 operator+(const key_varying Float4& a, const key_varying Float4& b)
{
	return make_Float4(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2], a.v[3] + b.v[3]);
}
static inline key_varying Float4 operator-(const key_varying Float4& a, const key_varying Float4& b)
{
	return make_Float4(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2], a.v[3] - b.v[3]);
}
static inline key_varying Float4 operator*(const key_varying Float4& a, const key_varying Float4& b)
{
	return make_Float4(a.v[0] * b.v[0], a.v[1] * b.v[1], a.v[2] * b.v[2], a.v[3] * b.v[3]);
}
static inline key_varying Float4 operator/(const key_varying Float4& a, const key_varying Float4& b)
{
	return make_Float4(a.v[0] / b.v[0], a.v[1] / b.v[1], a.v[2] / b.v[2], a.v[3] / b.v[3]);
}

static inline key_varying Float4 operator+(const key_varying Float4& v, const key_varying float s)
{
	return make_Float4(v.v[0] + s, v.v[1] + s, v.v[2] + s, v.v[3] + s);
}
static inline key_varying Float4 operator+(const key_varying float s, const key_varying Float4& v)
{
	return make_Float4(s + v.v[0], s + v.v[1], s + v.v[2], s + v.v[3]);
}
static inline key_varying Float4 operator-(const key_varying Float4& v, const key_varying float s)
{
	return make_Float4(v.v[0] - s, v.v[1] - s, v.v[2] - s, v.v[3] - s);
}
static inline key_varying Float4 operator-(const key_varying float s, const key_varying Float4& v)
{
	return make_Float4(s - v.v[0], s - v.v[1], s - v.v[2], s - v.v[3]);
}
static inline key_varying Float4 operator*(const key_varying Float4& v, const key_varying float s)
{
	return make_Float4(v.v[0] * s, v.v[1] * s, v.v[2] * s, v.v[3] * s);
}
static inline key_varying Float4 operator*(const key_varying float s, const key_varying Float4& v)
{
	return make_Float4(s * v.v[0], s * v.v[1], s * v.v[2], s * v.v[3]);
}
static inline key_varying Float4 operator/(const key_varying Float4& v, const key_varying float s)
{
	return make_Float4(v.v[0] / s, v.v[1] / s, v.v[2] / s, v.v[3] / s);
}
static inline key_varying Float4 operator/(const key_varying float s, const key_varying Float4& v)
{
	return make_Float4(s / v.v[0], s / v.v[1], s / v.v[2], s / v.v[3]);
}

static inline key_varying Float4 cross(const key_varying Float4& a, const key_varying Float4& b)
{
	key_varying Float4 result;
	result.v[0] = a.v[1] * b.v[2] - a.v[2] * b.v[1];
	result.v[1] = a.v[2] * b.v[0] - a.v[0] * b.v[2];
	result.v[2] = a.v[0] * b.v[1] - a.v[1] * b.v[0];
	result.v[3] = 0.0f;
	return result;
}

static inline key_varying Float4 normalize(const key_varying Float4& v)
{
	key_varying float len_sq = v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2] + v.v[3] * v.v[3];
	key_varying float inv_len = (len_sq > 0.0f) ? 1.0f / sqrt(len_sq) : 0.0f;
	key_varying Float4 result;
	result.v[0] = v.v[0] * inv_len;
	result.v[1] = v.v[1] * inv_len;
	result.v[2] = v.v[2] * inv_len;
	result.v[3] = v.v[3] * inv_len;
	return result;
}

static inline key_varying float dot(const key_varying Float4& a, const key_varying Float4& b)
{
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2] + a.v[3] * b.v[3];
}

#if defined (ISPC) || defined(__ISPC__)
static inline key_uniform float get_x(const key_uniform Float4& v)
{
	return v.v[0];
}
static inline key_uniform float get_y(const key_uniform Float4& v)
{
	return v.v[1];
}
static inline key_uniform float get_z(const key_uniform Float4& v)
{
	return v.v[2];
}
static inline key_uniform float get_w(const key_uniform Float4& v)
{
	return v.v[3];
}
static inline key_uniform Float2 get_xy(const key_uniform Float4& v)
{
	return make_Float2(v.v[0], v.v[1]);
}

static inline key_uniform Float4 make_Float4(
	const key_uniform float x,
	const key_uniform float y,
	const key_uniform float z,
	const key_uniform float w)
{
	key_uniform Float4 v;
	v.v[0] = x;
	v.v[1] = y;
	v.v[2] = z;
	v.v[3] = w;
	return v;
}

static inline key_uniform Float4 make_Float4(const key_uniform float f)
{
	key_uniform Float4 v;
	v.v[0] = f;
	v.v[1] = f;
	v.v[2] = f;
	v.v[3] = f;
	return v;
}

static inline key_uniform Float4 make_Float4(const key_uniform Float3& v3, const key_uniform float w)
{
	key_uniform Float4 v4;
	v4.v[0] = v3.v[0];
	v4.v[1] = v3.v[1];
	v4.v[2] = v3.v[2];
	v4.v[3] = w;
	return v4;
}

static inline key_uniform Float4 operator+(const key_uniform Float4& a, const key_uniform Float4& b)
{
	return make_Float4(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2], a.v[3] + b.v[3]);
}
static inline key_uniform Float4 operator-(const key_uniform Float4& a, const key_uniform Float4& b)
{
	return make_Float4(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2], a.v[3] - b.v[3]);
}
static inline key_uniform Float4 operator*(const key_uniform Float4& a, const key_uniform Float4& b)
{
	return make_Float4(a.v[0] * b.v[0], a.v[1] * b.v[1], a.v[2] * b.v[2], a.v[3] * b.v[3]);
}
static inline key_uniform Float4 operator/(const key_uniform Float4& a, const key_uniform Float4& b)
{
	return make_Float4(a.v[0] / b.v[0], a.v[1] / b.v[1], a.v[2] / b.v[2], a.v[3] / b.v[3]);
}

static inline key_uniform Float4 operator+(const key_uniform Float4& v, const key_uniform float s)
{
	return make_Float4(v.v[0] + s, v.v[1] + s, v.v[2] + s, v.v[3] + s);
}
static inline key_uniform Float4 operator+(const key_uniform float s, const key_uniform Float4& v)
{
	return make_Float4(s + v.v[0], s + v.v[1], s + v.v[2], s + v.v[3]);
}
static inline key_uniform Float4 operator-(const key_uniform Float4& v, const key_uniform float s)
{
	return make_Float4(v.v[0] - s, v.v[1] - s, v.v[2] - s, v.v[3] - s);
}
static inline key_uniform Float4 operator-(const key_uniform float s, const key_uniform Float4& v)
{
	return make_Float4(s - v.v[0], s - v.v[1], s - v.v[2], s - v.v[3]);
}
static inline key_uniform Float4 operator*(const key_uniform Float4& v, const key_uniform float s)
{
	return make_Float4(v.v[0] * s, v.v[1] * s, v.v[2] * s, v.v[3] * s);
}
static inline key_uniform Float4 operator*(const key_uniform float s, const key_uniform Float4& v)
{
	return make_Float4(s * v.v[0], s * v.v[1], s * v.v[2], s * v.v[3]);
}
static inline key_uniform Float4 operator/(const key_uniform Float4& v, const key_uniform float s)
{
	return make_Float4(v.v[0] / s, v.v[1] / s, v.v[2] / s, v.v[3] / s);
}
static inline key_uniform Float4 operator/(const key_uniform float s, const key_uniform Float4& v)
{
	return make_Float4(s / v.v[0], s / v.v[1], s / v.v[2], s / v.v[3]);
}

static inline key_uniform Float4 cross(const key_uniform Float4& a, const key_uniform Float4& b)
{
	key_uniform Float4 result;
	result.v[0] = a.v[1] * b.v[2] - a.v[2] * b.v[1];
	result.v[1] = a.v[2] * b.v[0] - a.v[0] * b.v[2];
	result.v[2] = a.v[0] * b.v[1] - a.v[1] * b.v[0];
	result.v[3] = 0.0f;
	return result;
}

static inline key_uniform Float4 normalize(const key_uniform Float4& v)
{
	key_uniform float len_sq = v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2] + v.v[3] * v.v[3];
	key_uniform float inv_len = (len_sq > 0.0f) ? 1.0f / sqrt(len_sq) : 0.0f;
	key_uniform Float4 result;
	result.v[0] = v.v[0] * inv_len;
	result.v[1] = v.v[1] * inv_len;
	result.v[2] = v.v[2] * inv_len;
	result.v[3] = v.v[3] * inv_len;
	return result;
}

static inline key_uniform float dot(const key_uniform Float4& a, const key_uniform Float4& b)
{
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2] + a.v[3] * b.v[3];
}
#endif



struct Matrix2x2
{
	float m[2][2];
};

static inline key_varying Matrix2x2 matrix2x2_zero_varying()
{
	key_varying Matrix2x2 m;
	m.m[0][0] = 0.0f; m.m[0][1] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 0.0f;
	return m;
}

static inline key_varying Matrix2x2 matrix2x2_identity_varying()
{
	key_varying Matrix2x2 m;
	m.m[0][0] = 1.0f; m.m[0][1] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 1.0f;
	return m;
}

static inline key_varying Matrix2x2 operator*(const key_varying Matrix2x2& a, const key_varying Matrix2x2& b)
{
	key_varying Matrix2x2 r;
	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
	r.m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
	r.m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
	r.m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
	return r;
}

static inline key_varying float matrix2x2_det(const key_varying Matrix2x2& m)
{
	return m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
}

static inline key_varying Matrix2x2 matrix2x2_inverse(const key_varying Matrix2x2& m)
{
	key_varying float det = matrix2x2_det(m);
	key_varying Matrix2x2 r;
	if (det == 0.0f)
		return matrix2x2_identity_varying();

	key_varying float inv = 1.0f / det;
	r.m[0][0] = m.m[1][1] * inv;
	r.m[0][1] = -m.m[0][1] * inv;
	r.m[1][0] = -m.m[1][0] * inv;
	r.m[1][1] = m.m[0][0] * inv;
	return r;
}

static inline key_varying Float2 operator*(const key_varying Matrix2x2& m, const key_varying Float2& v)
{
	key_varying Float2 result;
	result.v[0] = m.m[0][0] * v.v[0] + m.m[0][1] * v.v[1];
	result.v[1] = m.m[1][0] * v.v[0] + m.m[1][1] * v.v[1];
	return result;
}

#if defined (ISPC) || defined(__ISPC__)
static inline key_uniform Matrix2x2 matrix2x2_zero_uniform()
{
	key_uniform Matrix2x2 m;
	m.m[0][0] = 0.0f; m.m[0][1] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 0.0f;
	return m;
}

static inline key_uniform Matrix2x2 matrix2x2_identity_uniform()
{
	key_uniform Matrix2x2 m;
	m.m[0][0] = 1.0f; m.m[0][1] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 1.0f;
	return m;
}

static inline key_uniform Matrix2x2 operator*(const key_uniform Matrix2x2& a, const key_uniform Matrix2x2& b)
{
	key_uniform Matrix2x2 r;
	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
	r.m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
	r.m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
	r.m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
	return r;
}

static inline key_uniform float matrix2x2_det(const key_uniform Matrix2x2& m)
{
	return m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
}

static inline key_uniform Matrix2x2 matrix2x2_inverse(const key_uniform Matrix2x2& m)
{
	key_uniform float det = matrix2x2_det(m);
	key_uniform Matrix2x2 r;
	if (det == 0.0f)
		return matrix2x2_identity_uniform();

	key_uniform float inv = 1.0f / det;
	r.m[0][0] = m.m[1][1] * inv;
	r.m[0][1] = -m.m[0][1] * inv;
	r.m[1][0] = -m.m[1][0] * inv;
	r.m[1][1] = m.m[0][0] * inv;
	return r;
}

static inline key_uniform Float2 operator*(const key_uniform Matrix2x2& m, const key_uniform Float2& v)
{
	key_uniform Float2 result;
	result.v[0] = m.m[0][0] * v.v[0] + m.m[0][1] * v.v[1];
	result.v[1] = m.m[1][0] * v.v[0] + m.m[1][1] * v.v[1];
	return result;
}
#endif



struct Matrix3x3
{
	float m[3][3];
};

static inline key_varying Matrix3x3 matrix3x3_zero_varying()
{
	key_varying Matrix3x3 m;
	m.m[0][0] = 0.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 0.0f; m.m[1][2] = 0.0f;
	m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 0.0f;
	return m;
}

static inline key_varying Matrix3x3 matrix3x3_identity_varying()
{
	key_varying Matrix3x3 m;
	m.m[0][0] = 1.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 1.0f; m.m[1][2] = 0.0f;
	m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 1.0f;
	return m;
}

static inline key_varying Matrix3x3 operator*(const key_varying Matrix3x3& a, const key_varying Matrix3x3& b)
{
	key_varying Matrix3x3 r;
	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0];
	r.m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1];
	r.m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2];

	r.m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0];
	r.m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1];
	r.m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2];

	r.m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0];
	r.m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1];
	r.m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2];
	return r;
}

static inline key_varying float matrix3x3_det(const key_varying Matrix3x3& m)
{
	key_varying float a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2];
	key_varying float a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2];
	key_varying float a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2];
	return a00 * (a11 * a22 - a12 * a21)
		- a01 * (a10 * a22 - a12 * a20)
		+ a02 * (a10 * a21 - a11 * a20);
}

static inline key_varying Matrix3x3 matrix3x3_inverse(const key_varying Matrix3x3& m)
{
	key_varying float det = matrix3x3_det(m);
	key_varying Matrix3x3 r;
	if (det == 0.0f)
		return matrix3x3_identity_varying();

	key_varying float inv = 1.0f / det;
	key_varying float a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2];
	key_varying float a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2];
	key_varying float a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2];

	r.m[0][0] = (a11 * a22 - a12 * a21) * inv;
	r.m[0][1] = (a02 * a21 - a01 * a22) * inv;
	r.m[0][2] = (a01 * a12 - a02 * a11) * inv;
	r.m[1][0] = (a12 * a20 - a10 * a22) * inv;
	r.m[1][1] = (a00 * a22 - a02 * a20) * inv;
	r.m[1][2] = (a02 * a10 - a00 * a12) * inv;
	r.m[2][0] = (a10 * a21 - a11 * a20) * inv;
	r.m[2][1] = (a01 * a20 - a00 * a21) * inv;
	r.m[2][2] = (a00 * a11 - a01 * a10) * inv;
	return r;
}

static inline key_varying Float3 operator*(const key_varying Matrix3x3& m, const key_varying Float3& v)
{
	key_varying Float3 result;
	result.v[0] = m.m[0][0] * v.v[0] + m.m[0][1] * v.v[1] + m.m[0][2] * v.v[2];
	result.v[1] = m.m[1][0] * v.v[0] + m.m[1][1] * v.v[1] + m.m[1][2] * v.v[2];
	result.v[2] = m.m[2][0] * v.v[0] + m.m[2][1] * v.v[1] + m.m[2][2] * v.v[2];
	return result;
}

#if defined (ISPC) || defined(__ISPC__)
static inline key_uniform Matrix3x3 matrix3x3_zero_uniform()
{
	key_uniform Matrix3x3 m;
	m.m[0][0] = 0.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 0.0f; m.m[1][2] = 0.0f;
	m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 0.0f;
	return m;
}

static inline key_uniform Matrix3x3 matrix3x3_identity_uniform()
{
	key_uniform Matrix3x3 m;
	m.m[0][0] = 1.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 1.0f; m.m[1][2] = 0.0f;
	m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 1.0f;
	return m;
}

static inline key_uniform Matrix3x3 operator*(const key_uniform Matrix3x3& a, const key_uniform Matrix3x3& b)
{
	key_uniform Matrix3x3 r;
	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0];
	r.m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1];
	r.m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2];

	r.m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0];
	r.m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1];
	r.m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2];

	r.m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0];
	r.m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1];
	r.m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2];
	return r;
}

static inline key_uniform float matrix3x3_det(const key_uniform Matrix3x3& m)
{
	key_uniform float a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2];
	key_uniform float a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2];
	key_uniform float a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2];
	return a00 * (a11 * a22 - a12 * a21)
		- a01 * (a10 * a22 - a12 * a20)
		+ a02 * (a10 * a21 - a11 * a20);
}

static inline key_uniform Matrix3x3 matrix3x3_inverse(const key_uniform Matrix3x3& m)
{
	key_uniform float det = matrix3x3_det(m);
	key_uniform Matrix3x3 r;
	if (det == 0.0f)
		return matrix3x3_identity_uniform();

	key_uniform float inv = 1.0f / det;
	key_uniform float a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2];
	key_uniform float a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2];
	key_uniform float a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2];

	r.m[0][0] = (a11 * a22 - a12 * a21) * inv;
	r.m[0][1] = (a02 * a21 - a01 * a22) * inv;
	r.m[0][2] = (a01 * a12 - a02 * a11) * inv;
	r.m[1][0] = (a12 * a20 - a10 * a22) * inv;
	r.m[1][1] = (a00 * a22 - a02 * a20) * inv;
	r.m[1][2] = (a02 * a10 - a00 * a12) * inv;
	r.m[2][0] = (a10 * a21 - a11 * a20) * inv;
	r.m[2][1] = (a01 * a20 - a00 * a21) * inv;
	r.m[2][2] = (a00 * a11 - a01 * a10) * inv;
	return r;
}

static inline key_uniform Float3 operator*(const key_uniform Matrix3x3& m, const key_uniform Float3& v)
{
	key_uniform Float3 result;
	result.v[0] = m.m[0][0] * v.v[0] + m.m[0][1] * v.v[1] + m.m[0][2] * v.v[2];
	result.v[1] = m.m[1][0] * v.v[0] + m.m[1][1] * v.v[1] + m.m[1][2] * v.v[2];
	result.v[2] = m.m[2][0] * v.v[0] + m.m[2][1] * v.v[1] + m.m[2][2] * v.v[2];
	return result;
}
#endif	

struct Matrix4x4
{
	float m[4][4];
};

static inline key_varying Matrix4x4 matrix4x4_zero_varying()
{
	key_varying Matrix4x4 m;
	m.m[0][0] = 0.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f; m.m[0][3] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 0.0f; m.m[1][2] = 0.0f; m.m[1][3] = 0.0f;
	m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 0.0f; m.m[2][3] = 0.0f;
	m.m[3][0] = 0.0f; m.m[3][1] = 0.0f; m.m[3][2] = 0.0f; m.m[3][3] = 0.0f;
	return m;
}

static inline key_varying Matrix4x4 matrix4x4_identity_varying()
{
	key_varying Matrix4x4 m;
	m.m[0][0] = 1.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f; m.m[0][3] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 1.0f; m.m[1][2] = 0.0f; m.m[1][3] = 0.0f;
	m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 1.0f; m.m[2][3] = 0.0f;
	m.m[3][0] = 0.0f; m.m[3][1] = 0.0f; m.m[3][2] = 0.0f; m.m[3][3] = 1.0f;
	return m;
}

static inline key_varying Matrix4x4 operator*(const key_varying Matrix4x4& a, const key_varying Matrix4x4& b)
{
	key_varying Matrix4x4 r;
	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0] + a.m[0][3] * b.m[3][0];
	r.m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1] + a.m[0][3] * b.m[3][1];
	r.m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2] + a.m[0][3] * b.m[3][2];
	r.m[0][3] = a.m[0][0] * b.m[0][3] + a.m[0][1] * b.m[1][3] + a.m[0][2] * b.m[2][3] + a.m[0][3] * b.m[3][3];

	r.m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0] + a.m[1][3] * b.m[3][0];
	r.m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1] + a.m[1][3] * b.m[3][1];
	r.m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2] + a.m[1][3] * b.m[3][2];
	r.m[1][3] = a.m[1][0] * b.m[0][3] + a.m[1][1] * b.m[1][3] + a.m[1][2] * b.m[2][3] + a.m[1][3] * b.m[3][3];

	r.m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0] + a.m[2][3] * b.m[3][0];
	r.m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1] + a.m[2][3] * b.m[3][1];
	r.m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2] + a.m[2][3] * b.m[3][2];
	r.m[2][3] = a.m[2][0] * b.m[0][3] + a.m[2][1] * b.m[1][3] + a.m[2][2] * b.m[2][3] + a.m[2][3] * b.m[3][3];

	r.m[3][0] = a.m[3][0] * b.m[0][0] + a.m[3][1] * b.m[1][0] + a.m[3][2] * b.m[2][0] + a.m[3][3] * b.m[3][0];
	r.m[3][1] = a.m[3][0] * b.m[0][1] + a.m[3][1] * b.m[1][1] + a.m[3][2] * b.m[2][1] + a.m[3][3] * b.m[3][1];
	r.m[3][2] = a.m[3][0] * b.m[0][2] + a.m[3][1] * b.m[1][2] + a.m[3][2] * b.m[2][2] + a.m[3][3] * b.m[3][2];
	r.m[3][3] = a.m[3][0] * b.m[0][3] + a.m[3][1] * b.m[1][3] + a.m[3][2] * b.m[2][3] + a.m[3][3] * b.m[3][3];
	return r;
}

static inline key_varying float matrix4x4_det(const key_varying Matrix4x4& m)
{
	key_varying float a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2], a03 = m.m[0][3];
	key_varying float a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2], a13 = m.m[1][3];
	key_varying float a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2], a23 = m.m[2][3];
	key_varying float a30 = m.m[3][0], a31 = m.m[3][1], a32 = m.m[3][2], a33 = m.m[3][3];
	key_varying float det = 0.0f;
	det += a00 * (a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) + a13 * (a21 * a32 - a22 * a31));
	det -= a01 * (a10 * (a22 * a33 - a23 * a32) - a12 * (a20 * a33 - a23 * a30) + a13 * (a20 * a32 - a22 * a30));
	det += a02 * (a10 * (a21 * a33 - a23 * a31) - a11 * (a20 * a33 - a23 * a30) + a13 * (a20 * a31 - a21 * a30));
	det -= a03 * (a10 * (a21 * a32 - a22 * a31) - a11 * (a20 * a32 - a22 * a30) + a12 * (a20 * a31 - a21 * a30));
	return det;
}

static inline key_varying Matrix4x4 matrix4x4_inverse(const key_varying Matrix4x4& m)
{
	key_varying float det = matrix4x4_det(m);
	if (det == 0.0f)
		return matrix4x4_identity_varying();

	key_varying float inv = 1.0f / det;
	key_varying float a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2], a03 = m.m[0][3];
	key_varying float a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2], a13 = m.m[1][3];
	key_varying float a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2], a23 = m.m[2][3];
	key_varying float a30 = m.m[3][0], a31 = m.m[3][1], a32 = m.m[3][2], a33 = m.m[3][3];

	key_varying Matrix4x4 r;

	r.m[0][0] = (a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) + a13 * (a21 * a32 - a22 * a31)) * inv;
	r.m[0][1] = (a02 * (a21 * a33 - a23 * a31) - a01 * (a22 * a33 - a23 * a32) - a03 * (a21 * a32 - a22 * a31)) * inv;
	r.m[0][2] = (a01 * (a12 * a33 - a13 * a32) - a02 * (a11 * a33 - a13 * a31) + a03 * (a11 * a32 - a12 * a31)) * inv;
	r.m[0][3] = (a02 * (a11 * a23 - a13 * a21) - a01 * (a12 * a23 - a13 * a22) - a03 * (a11 * a22 - a12 * a21)) * inv;

	r.m[1][0] = (a12 * (a20 * a33 - a23 * a30) - a10 * (a22 * a33 - a23 * a32) - a13 * (a20 * a32 - a22 * a30)) * inv;
	r.m[1][1] = (a00 * (a22 * a33 - a23 * a32) - a02 * (a20 * a33 - a23 * a30) + a03 * (a20 * a32 - a22 * a30)) * inv;
	r.m[1][2] = (a02 * (a10 * a33 - a13 * a30) - a00 * (a12 * a33 - a13 * a32) - a03 * (a10 * a32 - a12 * a30)) * inv;
	r.m[1][3] = (a00 * (a12 * a23 - a13 * a22) - a02 * (a10 * a23 - a13 * a20) + a03 * (a10 * a22 - a12 * a20)) * inv;

	r.m[2][0] = (a10 * (a21 * a33 - a23 * a31) - a11 * (a20 * a33 - a23 * a30) + a13 * (a20 * a31 - a21 * a30)) * inv;
	r.m[2][1] = (a01 * (a20 * a33 - a23 * a30) - a00 * (a21 * a33 - a23 * a31) - a03 * (a20 * a31 - a21 * a30)) * inv;
	r.m[2][2] = (a00 * (a11 * a33 - a13 * a31) - a01 * (a10 * a33 - a13 * a30) + a03 * (a10 * a31 - a11 * a30)) * inv;
	r.m[2][3] = (a01 * (a10 * a23 - a13 * a20) - a00 * (a11 * a23 - a13 * a21) - a03 * (a10 * a21 - a11 * a20)) * inv;

	r.m[3][0] = (a11 * (a20 * a32 - a22 * a30) - a10 * (a21 * a32 - a22 * a31) - a12 * (a20 * a31 - a21 * a30)) * inv;
	r.m[3][1] = (a00 * (a21 * a32 - a22 * a31) - a01 * (a20 * a32 - a22 * a30) + a02 * (a20 * a31 - a21 * a30)) * inv;
	r.m[3][2] = (a01 * (a10 * a32 - a12 * a30) - a00 * (a11 * a32 - a12 * a31) - a02 * (a10 * a31 - a11 * a30)) * inv;
	r.m[3][3] = (a00 * (a11 * a22 - a12 * a21) - a01 * (a10 * a22 - a12 * a20) + a02 * (a10 * a21 - a11 * a20)) * inv;
	return r;
}

static inline key_varying Float4 operator*(const key_varying Matrix4x4& m, const key_varying Float4& v)
{
	key_varying Float4 result;
	result.v[0] = m.m[0][0] * v.v[0] + m.m[0][1] * v.v[1] + m.m[0][2] * v.v[2] + m.m[0][3] * v.v[3];
	result.v[1] = m.m[1][0] * v.v[0] + m.m[1][1] * v.v[1] + m.m[1][2] * v.v[2] + m.m[1][3] * v.v[3];
	result.v[2] = m.m[2][0] * v.v[0] + m.m[2][1] * v.v[1] + m.m[2][2] * v.v[2] + m.m[2][3] * v.v[3];
	result.v[3] = m.m[3][0] * v.v[0] + m.m[3][1] * v.v[1] + m.m[3][2] * v.v[2] + m.m[3][3] * v.v[3];
	return result;
}

#if defined (ISPC) || defined(__ISPC__)
static inline key_uniform Matrix4x4 matrix4x4_zero_uniform()
{
	key_uniform Matrix4x4 m;
	m.m[0][0] = 0.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f; m.m[0][3] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 0.0f; m.m[1][2] = 0.0f; m.m[1][3] = 0.0f;
	m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 0.0f; m.m[2][3] = 0.0f;
	m.m[3][0] = 0.0f; m.m[3][1] = 0.0f; m.m[3][2] = 0.0f; m.m[3][3] = 0.0f;
	return m;
}

static inline key_uniform Matrix4x4 matrix4x4_identity_uniform()
{
	key_uniform Matrix4x4 m;
	m.m[0][0] = 1.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f; m.m[0][3] = 0.0f;
	m.m[1][0] = 0.0f; m.m[1][1] = 1.0f; m.m[1][2] = 0.0f; m.m[1][3] = 0.0f;
	m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 1.0f; m.m[2][3] = 0.0f;
	m.m[3][0] = 0.0f; m.m[3][1] = 0.0f; m.m[3][2] = 0.0f; m.m[3][3] = 1.0f;
	return m;
}

static inline key_uniform Matrix4x4 operator*(const key_uniform Matrix4x4& a, const key_uniform Matrix4x4& b)
{
	key_uniform Matrix4x4 r;
	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0] + a.m[0][3] * b.m[3][0];
	r.m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1] + a.m[0][3] * b.m[3][1];
	r.m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2] + a.m[0][3] * b.m[3][2];
	r.m[0][3] = a.m[0][0] * b.m[0][3] + a.m[0][1] * b.m[1][3] + a.m[0][2] * b.m[2][3] + a.m[0][3] * b.m[3][3];

	r.m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0] + a.m[1][3] * b.m[3][0];
	r.m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1] + a.m[1][3] * b.m[3][1];
	r.m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2] + a.m[1][3] * b.m[3][2];
	r.m[1][3] = a.m[1][0] * b.m[0][3] + a.m[1][1] * b.m[1][3] + a.m[1][2] * b.m[2][3] + a.m[1][3] * b.m[3][3];

	r.m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0] + a.m[2][3] * b.m[3][0];
	r.m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1] + a.m[2][3] * b.m[3][1];
	r.m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2] + a.m[2][3] * b.m[3][2];
	r.m[2][3] = a.m[2][0] * b.m[0][3] + a.m[2][1] * b.m[1][3] + a.m[2][2] * b.m[2][3] + a.m[2][3] * b.m[3][3];

	r.m[3][0] = a.m[3][0] * b.m[0][0] + a.m[3][1] * b.m[1][0] + a.m[3][2] * b.m[2][0] + a.m[3][3] * b.m[3][0];
	r.m[3][1] = a.m[3][0] * b.m[0][1] + a.m[3][1] * b.m[1][1] + a.m[3][2] * b.m[2][1] + a.m[3][3] * b.m[3][1];
	r.m[3][2] = a.m[3][0] * b.m[0][2] + a.m[3][1] * b.m[1][2] + a.m[3][2] * b.m[2][2] + a.m[3][3] * b.m[3][2];
	r.m[3][3] = a.m[3][0] * b.m[0][3] + a.m[3][1] * b.m[1][3] + a.m[3][2] * b.m[2][3] + a.m[3][3] * b.m[3][3];
	return r;
}

static inline key_uniform float matrix4x4_det(const key_uniform Matrix4x4& m)
{
	key_uniform float a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2], a03 = m.m[0][3];
	key_uniform float a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2], a13 = m.m[1][3];
	key_uniform float a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2], a23 = m.m[2][3];
	key_uniform float a30 = m.m[3][0], a31 = m.m[3][1], a32 = m.m[3][2], a33 = m.m[3][3];
	key_uniform float det = 0.0f;
	det += a00 * (a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) + a13 * (a21 * a32 - a22 * a31));
	det -= a01 * (a10 * (a22 * a33 - a23 * a32) - a12 * (a20 * a33 - a23 * a30) + a13 * (a20 * a32 - a22 * a30));
	det += a02 * (a10 * (a21 * a33 - a23 * a31) - a11 * (a20 * a33 - a23 * a30) + a13 * (a20 * a31 - a21 * a30));
	det -= a03 * (a10 * (a21 * a32 - a22 * a31) - a11 * (a20 * a32 - a22 * a30) + a12 * (a20 * a31 - a21 * a30));
	return det;
}

static inline key_uniform Matrix4x4 matrix4x4_inverse(const key_uniform Matrix4x4& m)
{
	key_uniform float det = matrix4x4_det(m);
	if (det == 0.0f)
		return matrix4x4_identity_uniform();

	key_uniform float inv = 1.0f / det;
	key_uniform float a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2], a03 = m.m[0][3];
	key_uniform float a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2], a13 = m.m[1][3];
	key_uniform float a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2], a23 = m.m[2][3];
	key_uniform float a30 = m.m[3][0], a31 = m.m[3][1], a32 = m.m[3][2], a33 = m.m[3][3];

	key_uniform Matrix4x4 r;

	r.m[0][0] = (a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) + a13 * (a21 * a32 - a22 * a31)) * inv;
	r.m[0][1] = (a02 * (a21 * a33 - a23 * a31) - a01 * (a22 * a33 - a23 * a32) - a03 * (a21 * a32 - a22 * a31)) * inv;
	r.m[0][2] = (a01 * (a12 * a33 - a13 * a32) - a02 * (a11 * a33 - a13 * a31) + a03 * (a11 * a32 - a12 * a31)) * inv;
	r.m[0][3] = (a02 * (a11 * a23 - a13 * a21) - a01 * (a12 * a23 - a13 * a22) - a03 * (a11 * a22 - a12 * a21)) * inv;

	r.m[1][0] = (a12 * (a20 * a33 - a23 * a30) - a10 * (a22 * a33 - a23 * a32) - a13 * (a20 * a32 - a22 * a30)) * inv;
	r.m[1][1] = (a00 * (a22 * a33 - a23 * a32) - a02 * (a20 * a33 - a23 * a30) + a03 * (a20 * a32 - a22 * a30)) * inv;
	r.m[1][2] = (a02 * (a10 * a33 - a13 * a30) - a00 * (a12 * a33 - a13 * a32) - a03 * (a10 * a32 - a12 * a30)) * inv;
	r.m[1][3] = (a00 * (a12 * a23 - a13 * a22) - a02 * (a10 * a23 - a13 * a20) + a03 * (a10 * a22 - a12 * a20)) * inv;

	r.m[2][0] = (a10 * (a21 * a33 - a23 * a31) - a11 * (a20 * a33 - a23 * a30) + a13 * (a20 * a31 - a21 * a30)) * inv;
	r.m[2][1] = (a01 * (a20 * a33 - a23 * a30) - a00 * (a21 * a33 - a23 * a31) - a03 * (a20 * a31 - a21 * a30)) * inv;
	r.m[2][2] = (a00 * (a11 * a33 - a13 * a31) - a01 * (a10 * a33 - a13 * a30) + a03 * (a10 * a31 - a11 * a30)) * inv;
	r.m[2][3] = (a01 * (a10 * a23 - a13 * a20) - a00 * (a11 * a23 - a13 * a21) - a03 * (a10 * a21 - a11 * a20)) * inv;

	r.m[3][0] = (a11 * (a20 * a32 - a22 * a30) - a10 * (a21 * a32 - a22 * a31) - a12 * (a20 * a31 - a21 * a30)) * inv;
	r.m[3][1] = (a00 * (a21 * a32 - a22 * a31) - a01 * (a20 * a32 - a22 * a30) + a02 * (a20 * a31 - a21 * a30)) * inv;
	r.m[3][2] = (a01 * (a10 * a32 - a12 * a30) - a00 * (a11 * a32 - a12 * a31) - a02 * (a10 * a31 - a11 * a30)) * inv;
	r.m[3][3] = (a00 * (a11 * a22 - a12 * a21) - a01 * (a10 * a22 - a12 * a20) + a02 * (a10 * a21 - a11 * a20)) * inv;
	return r;
}

static inline key_uniform Float4 operator*(const key_uniform Matrix4x4& m, const key_uniform Float4& v)
{
	key_uniform Float4 result;
	result.v[0] = m.m[0][0] * v.v[0] + m.m[0][1] * v.v[1] + m.m[0][2] * v.v[2] + m.m[0][3] * v.v[3];
	result.v[1] = m.m[1][0] * v.v[0] + m.m[1][1] * v.v[1] + m.m[1][2] * v.v[2] + m.m[1][3] * v.v[3];
	result.v[2] = m.m[2][0] * v.v[0] + m.m[2][1] * v.v[1] + m.m[2][2] * v.v[2] + m.m[2][3] * v.v[3];
	result.v[3] = m.m[3][0] * v.v[0] + m.m[3][1] * v.v[1] + m.m[3][2] * v.v[2] + m.m[3][3] * v.v[3];
	return result;
}
#endif



struct BoundingBox2 
{
	float x_min, x_max;
	float y_min, y_max;
};

static inline key_varying BoundingBox2 make_bounding_box2_varying() 
{
	key_varying BoundingBox2 box;
	box.x_min = SIMPLE_MATH_FMAX;
	box.x_max = -SIMPLE_MATH_FMAX;
	box.y_min = SIMPLE_MATH_FMAX;
	box.y_max = -SIMPLE_MATH_FMAX;
	return box;
}

static inline key_varying BoundingBox2 extend_bounding_box2(const key_varying BoundingBox2& box, const key_varying Float2& point)
{
	key_varying float point_x = get_x(point);
	key_varying float point_y = get_y(point);

	key_varying BoundingBox2 extended_box = box;
	extended_box.x_min = point_x < box.x_min ? point_x : box.x_min;
	extended_box.x_max = point_x > box.x_max ? point_x : box.x_max;
	extended_box.y_min = point_y < box.y_min ? point_y : box.y_min;
	extended_box.y_max = point_y > box.y_max ? point_y : box.y_max;
	return extended_box;
}

#if defined (ISPC) || defined(__ISPC__)
static inline key_uniform BoundingBox2 make_bounding_box2_uniform()
{
	key_uniform BoundingBox2 box;
	box.x_min = SIMPLE_MATH_FMAX;
	box.x_max = -SIMPLE_MATH_FMAX;
	box.y_min = SIMPLE_MATH_FMAX;
	box.y_max = -SIMPLE_MATH_FMAX;
	return box;
}

static inline key_uniform BoundingBox2 extend_bounding_box2(const key_uniform BoundingBox2& box, const key_uniform Float2& point)
{
	key_uniform float point_x = get_x(point);
	key_uniform float point_y = get_y(point);

	key_uniform BoundingBox2 extended_box = box;
	extended_box.x_min = point_x < box.x_min ? point_x : box.x_min;
	extended_box.x_max = point_x > box.x_max ? point_x : box.x_max;
	extended_box.y_min = point_y < box.y_min ? point_y : box.y_min;
	extended_box.y_max = point_y > box.y_max ? point_y : box.y_max;
	return extended_box;
}
#endif

struct Camera
{
	Float3 position;
	Float3 front; // target - position.
	Float3 right;
	Float3 up;

	float fov;
	float aspect;
	float near;
	float far;
};

static inline void camera_look_at(key_varying Camera& camera, const key_varying Float3 target)
{
	// note: right hand coordinate.
	const key_varying Float3 world_up = make_Float3(0.0f, 1.0f, 0.0f);
	const key_varying Float3 new_front = normalize(target - camera.position);
	if (abs(dot(world_up, new_front)) > 0.999f)
		return;

	camera.front = new_front; // z
	camera.right = normalize(cross(world_up, new_front)); // x = y x z;
	camera.up = cross(new_front, camera.right); // y = z x x;
}

static inline key_varying Camera camera_init_varying()
{
	key_varying Camera camera;

	// note: initially camera look at +z.
	camera.position = make_Float3(0.0f, 0.0f, 0.0f);

	camera_look_at(camera, make_Float3(0.0f, 0.0f, 1.0f));

	camera.fov = 40.0f; // dgree;
	camera.aspect = 1.0f;
	camera.near = 0.1f;
	camera.far = 1000.0f;

	return camera;
}

static inline key_varying Matrix4x4 get_camera_to_world_matrix(const key_varying Camera& camera)
{
	key_varying Matrix4x4 matrix = matrix4x4_identity_varying();

	matrix.m[0][0] = camera.right.v[0];
	matrix.m[1][0] = camera.right.v[1];
	matrix.m[2][0] = camera.right.v[2];

	matrix.m[0][1] = camera.up.v[0];
	matrix.m[1][1] = camera.up.v[1];
	matrix.m[2][1] = camera.up.v[2];

	matrix.m[0][2] = camera.front.v[0];
	matrix.m[1][2] = camera.front.v[1];
	matrix.m[2][2] = camera.front.v[2];

	matrix.m[0][3] = camera.position.v[0];
	matrix.m[1][3] = camera.position.v[1];
	matrix.m[2][3] = camera.position.v[2];

	return matrix;
}

// note: fov in degrees. near and far greater than zero, and far greater than near.
static inline key_varying Matrix4x4 get_perspective_matrix(const key_varying float fov, 
	const key_varying float aspect, 
	const key_varying float near, 
	const key_varying float far)
{
	key_varying float fov_radians = fov / 180.0f * SIMPLE_MATH_PI;
	key_varying float height = 2.0f * near * tan(fov_radians * 0.5f);
	key_varying float width = height * aspect;

	if (near - far == 0.0f || height == 0.0f || width == 0.0f)
		return matrix4x4_identity_varying();

	key_varying Matrix4x4 matrix = matrix4x4_zero_varying();
	matrix.m[0][0] = 2.0f * near / width;
	matrix.m[1][1] = 2.0f * near / height;
	matrix.m[2][2] = far / (far - near);
	matrix.m[2][3] = - (near * far) / (far - near);
	matrix.m[3][2] = 1.0f;
	return matrix;
}

#if defined (ISPC) || defined(__ISPC__)
static inline void camera_look_at(key_uniform Camera& camera, const key_uniform Float3 target)
{
	// note: right hand coordinate.
	const key_uniform Float3 world_up = make_Float3(0.0f, 1.0f, 0.0f);
	const key_uniform Float3 new_front = normalize(target - camera.position);
	if (abs(dot(world_up, new_front)) > 0.999f)
		return;

	camera.front = new_front; // z
	camera.right = normalize(cross(world_up, new_front)); // x = y x z;
	camera.up = cross(new_front, camera.right); // y = z x x;
}

static inline key_uniform Camera camera_init_uniform()
{
	key_uniform Camera camera;

	// note: initially camera look at +z.
	camera.position = make_Float3(0.0f, 0.0f, 0.0f);

	camera_look_at(camera, make_Float3(0.0f, 0.0f, 1.0f));

	camera.fov = 40.0f; // degree;
	camera.aspect = 1.0f;
	camera.near = 0.1f;
	camera.far = 1000.0f;

	return camera;
}

static inline key_uniform Matrix4x4 get_camera_to_world_matrix(const key_uniform Camera& camera)
{
	key_uniform Matrix4x4 matrix = matrix4x4_identity_uniform();

	matrix.m[0][0] = camera.right.v[0];
	matrix.m[1][0] = camera.right.v[1];
	matrix.m[2][0] = camera.right.v[2];

	matrix.m[0][1] = camera.up.v[0];
	matrix.m[1][1] = camera.up.v[1];
	matrix.m[2][1] = camera.up.v[2];

	matrix.m[0][2] = camera.front.v[0];
	matrix.m[1][2] = camera.front.v[1];
	matrix.m[2][2] = camera.front.v[2];

	matrix.m[0][3] = camera.position.v[0];
	matrix.m[1][3] = camera.position.v[1];
	matrix.m[2][3] = camera.position.v[2];

	return matrix;
}

// note: fov in degrees. near and far greater than zero, and far greater than near.
static inline key_uniform Matrix4x4 get_perspective_matrix(
	const key_uniform float fov,
	const key_uniform float aspect,
	const key_uniform float near,
	const key_uniform float far)
{
	key_uniform float fov_radians = fov / 180.0f * SIMPLE_MATH_PI;
	key_uniform float height = 2.0f * near * tan(fov_radians * 0.5f);
	key_uniform float width = height * aspect;

	if (near - far == 0.0f || height == 0.0f || width == 0.0f)
		return matrix4x4_identity_uniform();

	key_uniform Matrix4x4 matrix = matrix4x4_zero_uniform();
	matrix.m[0][0] = 2.0f * near / width;
	matrix.m[1][1] = 2.0f * near / height;
	matrix.m[2][2] = far / (far - near);
	matrix.m[2][3] = -(near * far) / (far - near);
	matrix.m[3][2] = 1.0f;
	return matrix;
}
#endif

struct Triangle
{
	unsigned int index_0;
	unsigned int index_1;
	unsigned int index_2;

	unsigned int mesh_index;
};

struct Int2 
{
	int x, y;
};


#if defined (ISPC) || defined(__ISPC__)
#else 
using uint32 = uint32_t;
using uint64 = uint64_t;
#endif
struct Mesh 
{
	uint64 triangles_offset;
	uint64 vertex_positions_offset;
	uint64 vertex_colors_offset;
};

struct Pixel 
{
	float screen_x;
	float screen_y;
	unsigned int triangle_index;

	float b0_world;
	float b1_world;
	float b2_world;
};


static inline key_varying Float2 NDC_to_screen(
	const key_varying float x,
	const key_varying float y,
	const key_varying int width,
	const key_varying int height)
{
	key_varying float screen_x = (x * 0.5f + 0.5f) * ((key_varying float)width);
	key_varying float screen_y = (0.5f - y * 0.5f) * ((key_varying float)height);
	return make_Float2(screen_x, screen_y);
}

static inline key_varying Float2 screen_to_NDC(
	const key_varying float screen_x,
	const key_varying float screen_y,
	const key_varying int width,
	const key_varying int height) 
{
	key_varying float x = (screen_x / ((key_varying float)width) - 0.5f) * 2.0f;
	key_varying float y = (0.5f - screen_y / ((key_varying float)height)) * 2.0f;
	return make_Float2(x, y);
}

static inline key_varying bool point_inside_triangle(
	const key_varying Float2& p,
	const key_varying Float2& p0,
	const key_varying Float2& p1,
	const key_varying Float2& p2)
{
	key_varying float cross_p0p_p0p1 = cross(p - p0, p1 - p0);
	key_varying float cross_p1p_p1p2 = cross(p - p1, p2 - p1);
	key_varying float cross_p2p_p0p2 = cross(p - p2, p0 - p2);

	return (cross_p0p_p0p1 > 0.0f && cross_p1p_p1p2 > 0.0f && cross_p2p_p0p2 > 0.0f) ||
		(cross_p0p_p0p1 <= 0.0f && cross_p1p_p1p2 <= 0.0f && cross_p2p_p0p2 <= 0.0f);
}

#if defined (ISPC) || defined(__ISPC__)
static inline key_uniform Float2 NDC_to_screen(
	const key_uniform float x,
	const key_uniform float y,
	const key_uniform int width,
	const key_uniform int height)
{
	key_uniform float screen_x = (x * 0.5f + 0.5f) * ((key_uniform float)width);
	key_uniform float screen_y = (0.5f - y * 0.5f) * ((key_uniform float)height);
	return make_Float2(screen_x, screen_y);
}

static inline key_uniform Float2 screen_to_NDC(
	const key_uniform float screen_x,
	const key_uniform float screen_y,
	const key_uniform int width,
	const key_uniform int height)
{
	key_uniform float x = (screen_x / ((key_uniform float)width) - 0.5f) * 2.0f;
	key_uniform float y = (0.5f - screen_y / ((key_uniform float)height)) * 2.0f;
	return make_Float2(x, y);
}

static inline key_uniform bool point_inside_triangle(
	const key_uniform Float2& p,
	const key_uniform Float2& p0,
	const key_uniform Float2& p1,
	const key_uniform Float2& p2)
{
	key_uniform float cross_p0p_p0p1 = cross(p - p0, p1 - p0);
	key_uniform float cross_p1p_p1p2 = cross(p - p1, p2 - p1);
	key_uniform float cross_p2p_p0p2 = cross(p - p2, p0 - p2);

	return (cross_p0p_p0p1 > 0.0f && cross_p1p_p1p2 > 0.0f && cross_p2p_p0p2 > 0.0f) ||
		(cross_p0p_p0p1 <= 0.0f && cross_p1p_p1p2 <= 0.0f && cross_p2p_p0p2 <= 0.0f);
}
#endif

#define MAX_POLYGON_VERTICES 4
static inline void clip_triangle_near_plane(key_varying const Float4 p_src[MAX_POLYGON_VERTICES],
	key_varying const int count_src, 
	key_varying Float4 p_dst[MAX_POLYGON_VERTICES], 
	key_varying int count_dst[])
{
	const key_varying float threshold = 1E-5f;

	*count_dst = 0;
	for (key_varying int p_index = 0; p_index < count_src; ++p_index) 
	{
		key_varying int p_next_index = (p_index + 1) % count_src; 

		const key_varying Float4 p0 = p_src[p_index];
		const key_varying Float4 p1 = p_src[p_next_index];

		const key_varying bool p0_in_range = !(get_z(p0) < (threshold * get_w(p0)));
		if (p0_in_range) 
		{
			p_dst[(*count_dst)++] = p_src[p_index];
		}

		const key_varying bool p1_in_range = !(get_z(p1) < (threshold * get_w(p1)));
		if (p1_in_range != p0_in_range)
		{
			key_varying float t = (get_z(p0) - threshold * get_w(p0)) / (threshold * (get_w(p1) - get_w(p0)) - (get_z(p1) - get_z(p0)));
			key_varying Float4 p_intersect = p0 + t * (p1 - p0);
			p_dst[(*count_dst)++] = p_intersect;
		}
	}
}

#if defined (ISPC) || defined(__ISPC__)
static inline void clip_triangle_near_plane(const key_uniform Float4 p_src[MAX_POLYGON_VERTICES],
	const key_uniform int count_src,
	key_uniform Float4 p_dst[MAX_POLYGON_VERTICES],
	key_uniform int count_dst[])
{
	const key_uniform float threshold = 1e-5f;

	*count_dst = 0;
	for (key_uniform int p_index = 0; p_index < count_src; ++p_index)
	{
		key_uniform int p_next_index = (p_index + 1) % count_src;

		const key_uniform Float4 p0 = p_src[p_index];
		const key_uniform Float4 p1 = p_src[p_next_index];

		const key_uniform bool p0_in_range = !(get_z(p0) < (threshold * get_w(p0)));
		if (p0_in_range)
		{
			p_dst[(*count_dst)++] = p_src[p_index];
		}

		const key_uniform bool p1_in_range = !(get_z(p1) < (threshold * get_w(p1)));
		if (p1_in_range != p0_in_range)
		{
			key_uniform float t = (get_z(p0) - threshold * get_w(p0)) /
				(threshold * (get_w(p1) - get_w(p0)) - (get_z(p1) - get_z(p0)));
			key_uniform Float4 p_intersect = p0 + t * (p1 - p0);
			p_dst[(*count_dst)++] = p_intersect;
		}
	}
}
#endif

static inline float uint_to_float(unsigned int u)
{
#if defined (ISPC) || defined(__ISPC__)
	return floatbits(u);
#else
	float f;
	memcpy(&f, &u, sizeof(f));
	return f;
#endif
}

static inline unsigned int float_to_uint(float f)
{
#if defined (ISPC) || defined(__ISPC__)
	return intbits(f);
#else
	unsigned int u;
	memcpy(&u, &f, sizeof(u));
	return u;
#endif
}

#endif // !SIMPLE_MATH_H

