#pragma once

#include "Math.hpp"

#include <memory>
#include <string>
#include <vector>

struct BVHNode {
    AABB bounds;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    std::vector<TriangleCombined> triangles;

    bool IsLeaf() const { return !left && !right; }
};

class VisCheck {
public:
    VisCheck() = default;
    explicit VisCheck(const std::string& triFile);
    explicit VisCheck(std::vector<TriangleCombined> triangles);

    bool IsReady() const { return !bvhNodes.empty(); }
    size_t TriangleCount() const { return triCount_; }

    // Returns true if no geometry blocks the ray from 'from' to 'to'
    bool IsPointVisible(const Vector3& from, const Vector3& to) const;

    // Casts a ray and returns the hit position + normal if hit
    struct HitResult {
        bool hit = false;
        float distance = 0.f;
        Vector3 point;
        Vector3 normal;
    };
    HitResult RayCast(const Vector3& origin, const Vector3& dir, float maxDist) const;

private:
    std::vector<std::unique_ptr<BVHNode>> bvhNodes;
    size_t triCount_{ 0 };

    void BuildFromTriangles(std::vector<TriangleCombined> triangles);
    std::unique_ptr<BVHNode> BuildBVH(const std::vector<TriangleCombined>& tris);
    bool IntersectBVH(const BVHNode* node, const Vector3& origin, const Vector3& dir, float maxDist, float& hitDist) const;
    bool IntersectBVHDetailed(const BVHNode* node, const Vector3& origin, const Vector3& dir, float maxDist, float& hitDist, Vector3& hitNormal) const;
    static bool RayIntersectsTriangle(const Vector3& origin, const Vector3& dir, const TriangleCombined& tri, float& t);
    static bool RayIntersectsTriangleDetailed(const Vector3& origin, const Vector3& dir, const TriangleCombined& tri, float& t, Vector3& normal);
};