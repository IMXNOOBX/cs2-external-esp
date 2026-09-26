#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

struct Vector3 {
    float x{}, y{}, z{};

    Vector3() = default;
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vector3 operator-(const Vector3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vector3 operator*(float f) const { return { x * f, y * f, z * f }; }
    Vector3 operator/(float f) const { return { x / f, y / f, z / f }; }

    float dot(const Vector3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vector3 cross(const Vector3& o) const {
        return {
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        };
    }

    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3 normalized() const {
        float len = length();
        return len > 0 ? *this / len : Vector3{};
    }
};

struct AABB {
    Vector3 min, max;

    bool RayIntersects(const Vector3& origin, const Vector3& dir) const {
        float t1 = (min.x - origin.x) / (dir.x != 0.0f ? dir.x : 1e-7f);
        float t2 = (max.x - origin.x) / (dir.x != 0.0f ? dir.x : 1e-7f);
        float t3 = (min.y - origin.y) / (dir.y != 0.0f ? dir.y : 1e-7f);
        float t4 = (max.y - origin.y) / (dir.y != 0.0f ? dir.y : 1e-7f);
        float t5 = (min.z - origin.z) / (dir.z != 0.0f ? dir.z : 1e-7f);
        float t6 = (max.z - origin.z) / (dir.z != 0.0f ? dir.z : 1e-7f);

        float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
        float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

        if (tmax < 0) return false;
        if (tmin > tmax) return false;
        return true;
    }
};

struct TriangleCombined {
    Vector3 v0, v1, v2;

    AABB ComputeAABB() const {
        return {
            { std::min({ v0.x, v1.x, v2.x }), std::min({ v0.y, v1.y, v2.y }), std::min({ v0.z, v1.z, v2.z }) },
            { std::max({ v0.x, v1.x, v2.x }), std::max({ v0.y, v1.y, v2.y }), std::max({ v0.z, v1.z, v2.z }) }
        };
    }
};