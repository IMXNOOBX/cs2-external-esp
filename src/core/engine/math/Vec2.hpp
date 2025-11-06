#pragma once
#include <cmath>

class vec2_t {
public:
	vec2_t() = default;
	explicit vec2_t(float a) : x(a), y(a) {}
	vec2_t(float a, float b) : x(a), y(b) {}

	operator float* () {
		return &x;
	}
	bool operator==(const vec2_t& o) const {
		return x == o.x && y == o.y;
	}
	bool operator==(const float o) const {
		return x == y && y == o;
	}
	bool operator!=(const vec2_t& o) const {
		return !(*this == o);
	}
	bool operator!=(const float o) const {
		return!(*this == o);
	}
	bool operator>(const vec2_t& o) const {
		return x > o.x && y > o.y;
	}
	bool operator>(const float o) const {
		return x > o && y > o;
	}
	bool operator<(const vec2_t& o) const {
		return o > *this;
	}
	bool operator<(const float o) const {
		return vec2_t(o) > *this;
	}
	bool operator>=(const vec2_t& o) const {
		return x >= o.x && y >= o.y;
	}
	bool operator>=(const float o) const {
		return x >= o && y >= o;
	}
	bool operator<=(const vec2_t& o) const {
		return o >= *this;
	}
	bool operator<=(const float o) const {
		return vec2_t(o, o) >= *this;
	}
	vec2_t operator+(const vec2_t& o) const {
		return vec2_t(x + o.x, y + o.y);
	}
	vec2_t operator+(const float o) const {
		return vec2_t(x + o, y + o);
	}
	vec2_t operator-() const {
		return vec2_t(-x, -y);
	}
	vec2_t operator-(const vec2_t& o) const {
		return *this + (-o);
	}
	vec2_t operator-(const float o) const {
		return *this + (-o);
	}
	vec2_t operator*(const vec2_t& o) const {
		return vec2_t(x * o.x, y * o.y);
	}
	vec2_t operator*(const float o) const {
		return vec2_t(x * o, y * o);
	}
	vec2_t operator/(const vec2_t& o) const {
		return vec2_t(x / o.x, y / o.y);
	}
	vec2_t operator/(const float o) const {
		return vec2_t(x / o, y / o);
	}
	void operator+=(const vec2_t& o) {
		x += o.x;
		y += o.y;
	}
	void operator+=(const float o) {
		x += o;
		y += o;
	}
	void operator-=(const vec2_t& o) {
		x -= o.x;
		y -= o.y;
	}
	void operator-=(const float o) {
		x -= o;
		y -= o;
	}
	void operator*=(const vec2_t& o) {
		x *= o.x;
		y *= o.y;
	}
	void operator*=(const float o) {
		x *= o;
		y *= o;
	}
	void operator/=(const vec2_t& o) {
		x /= o.x;
		y /= o.y;
	}
	void operator/=(const float o) {
		x /= o;
		y /= o;
	}

	vec2_t floor() const {
		return vec2_t(floorf(x), floorf(y));
	}
	vec2_t ceil() const {
		return vec2_t(ceilf(x), ceilf(y));
	}
	vec2_t round() const {
		return vec2_t(roundf(x), roundf(y));
	}
	vec2_t& floor() {
		x = floorf(x);
		y = floorf(y);
		return *this;
	}
	vec2_t& ceil() {
		x = ceilf(x);
		y = ceilf(y);
		return *this;
	}
	vec2_t& round() {
		x = roundf(x);
		y = roundf(y);
		return *this;
	}
	float len_sqr() const {
		return x * x + y * y;
	}
	float len() const {
		return sqrtf(len_sqr());
	}
	bool is_zero() const {
		return *this == 0.f;
	}

	float dist_to(const vec2_t& pos) {
		return sqrtf(powf(pos.x - x, 2) + powf(pos.y - y, 2));
	}

	float x{}, y{};
};