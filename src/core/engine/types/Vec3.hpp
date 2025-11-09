#pragma once
#include <numbers>
#include <cmath>
#include <limits>

class Vec3_t {
public:
	float x, y, z;
public:
	__forceinline Vec3_t() : x{}, y{}, z{} {}
	__forceinline Vec3_t(float x, float y, float z) : x{ x }, y{ y }, z{ z } {}

	__forceinline float& at(const size_t index) {
		return ((float*)this)[index];
	}
	__forceinline float& at(const size_t index) const {
		return ((float*)this)[index];
	}

	__forceinline float& operator( )(const size_t index) {
		return at(index);
	}
	__forceinline const float& operator( )(const size_t index) const {
		return at(index);
	}
	__forceinline float& operator[ ](const size_t index) {
		return at(index);
	}
	__forceinline const float& operator[ ](const size_t index) const {
		return at(index);
	}

	__forceinline bool operator==(const Vec3_t& v) const {
		return v.x == x && v.y == y && v.z == z;
	}
	__forceinline bool operator!=(const Vec3_t& v) const {
		return v.x != x || v.y != y || v.z != z;
	}

	__forceinline Vec3_t& operator=(const Vec3_t& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	__forceinline Vec3_t operator-() const {
		return Vec3_t{ -x, -y, -z };
	}

	__forceinline Vec3_t operator+(const Vec3_t& v) const {
		return {
			x + v.x,
			y + v.y,
			z + v.z
		};
	}

	__forceinline Vec3_t operator-(const Vec3_t& v) const {
		return {
			x - v.x,
			y - v.y,
			z - v.z
		};
	}

	__forceinline Vec3_t operator*(const Vec3_t& v) const {
		return {
			x * v.x,
			y * v.y,
			z * v.z
		};
	}

	__forceinline Vec3_t operator/(const Vec3_t& v) const {
		return {
			x / v.x,
			y / v.y,
			z / v.z
		};
	}

	__forceinline Vec3_t& operator+=(const Vec3_t& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	__forceinline Vec3_t& operator-=(const Vec3_t& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	__forceinline Vec3_t& operator*=(const Vec3_t& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	__forceinline Vec3_t& operator/=(const Vec3_t& v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	__forceinline Vec3_t operator+(float f) const {
		return {
			x + f,
			y + f,
			z + f
		};
	}

	__forceinline Vec3_t operator-(float f) const {
		return {
			x - f,
			y - f,
			z - f
		};
	}

	__forceinline Vec3_t operator*(float f) const {
		return {
			x * f,
			y * f,
			z * f
		};
	}

	__forceinline Vec3_t operator/(float f) const {
		return {
			x / f,
			y / f,
			z / f
		};
	}

	__forceinline Vec3_t& operator+=(float f) {
		x += f;
		y += f;
		z += f;
		return *this;
	}

	__forceinline Vec3_t& operator-=(float f) {
		x -= f;
		y -= f;
		z -= f;
		return *this;
	}

	__forceinline Vec3_t& operator*=(float f) {
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	__forceinline Vec3_t& operator/=(float f) {
		x /= f;
		y /= f;
		z /= f;
		return *this;
	}

	__forceinline void clear() {
		x = y = z = 0.f;
	}

	__forceinline float length_sqr() const {
		return ((x * x) + (y * y) + (z * z));
	}

	__forceinline float length_2d_sqr() const {
		return ((x * x) + (y * y));
	}

	__forceinline float length() const {
		return std::sqrt(length_sqr());
	}

	__forceinline float length_2d() const {
		return std::sqrt(length_2d_sqr());
	}

	__forceinline float dot(const Vec3_t& v) const {
		return (x * v.x + y * v.y + z * v.z);
	}

	__forceinline float dot(float* v) const {
		return (x * v[0] + y * v[1] + z * v[2]);
	}

	constexpr const bool zero() const noexcept {
		return x == 0.f && y == 0.f && z == 0.f;
	}

	__forceinline Vec3_t cross(const Vec3_t& v) const {
		return {
			(y * v.z) - (z * v.y),
			(z * v.x) - (x * v.z),
			(x * v.y) - (y * v.x)
		};
	}

	__forceinline float dist_to(const Vec3_t& vOther) const {
		Vec3_t delta;

		delta.x = x - vOther.x;
		delta.y = y - vOther.y;
		delta.z = z - vOther.z;

		return delta.length_2d();
	}

	__forceinline float dist_to_3d(const Vec3_t& vOther) const {
		Vec3_t delta;

		delta.x = x - vOther.x;
		delta.y = y - vOther.y;
		delta.z = z - vOther.z;

		return delta.length();
	}

	const Vec3_t& ToAngle() const noexcept {
		return Vec3_t{
			std::atan2(-z, std::hypot(x, y)) * (180.0f / std::numbers::pi_v<float>),
			std::atan2(y, x) * (180.0f / std::numbers::pi_v<float>),
			0.0f
		};
	}

	__forceinline float normalize() {
		float len = length();

		(*this) /= (length() + std::numeric_limits<float>::epsilon());

		return len;
	}

	__forceinline Vec3_t normalized() const {
		auto vec = *this;

		vec.normalize();

		return vec;
	}
};