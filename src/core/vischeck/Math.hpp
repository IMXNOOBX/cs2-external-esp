#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vector3 operator-(const Vector3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    float dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 cross(const Vector3& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }
};

struct AABB {
    Vector3 min;
    Vector3 max;

    bool RayIntersects(const Vector3& rayOrigin, const Vector3& rayDir) const;
};

struct TriangleCombined {
    Vector3 v0, v1, v2;

    AABB ComputeAABB() const;
};

inline AABB TriangleCombined::ComputeAABB() const {
    Vector3 mn, mx;
    mn.x = std::min({ v0.x, v1.x, v2.x });
    mn.y = std::min({ v0.y, v1.y, v2.y });
    mn.z = std::min({ v0.z, v1.z, v2.z });
    mx.x = std::max({ v0.x, v1.x, v2.x });
    mx.y = std::max({ v0.y, v1.y, v2.y });
    mx.z = std::max({ v0.z, v1.z, v2.z });
    return { mn, mx };
}

inline bool AABB::RayIntersects(const Vector3& rayOrigin, const Vector3& rayDir) const {
    float tmin = std::numeric_limits<float>::lowest();
    float tmax = std::numeric_limits<float>::max();
    const float* o = &rayOrigin.x;
    const float* d = &rayDir.x;
    const float* mn = &min.x;
    const float* mx = &max.x;

    for (int i = 0; i < 3; ++i) {
        float inv = 1.f / d[i];
        float t0 = (mn[i] - o[i]) * inv;
        float t1 = (mx[i] - o[i]) * inv;
        if (inv < 0.f) std::swap(t0, t1);
        tmin = std::max(tmin, t0);
        tmax = std::min(tmax, t1);
    }
    return tmax >= tmin && tmax >= 0;
}
