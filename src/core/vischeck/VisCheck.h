#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Math.hpp"

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
    bool IsPointVisible(const Vector3& from, const Vector3& to) const;

private:
    std::vector<std::unique_ptr<BVHNode>> bvhNodes;
    size_t triCount_{ 0 };

    void BuildFromTriangles(std::vector<TriangleCombined> triangles);
    std::unique_ptr<BVHNode> BuildBVH(const std::vector<TriangleCombined>& tris);
    bool IntersectBVH(const BVHNode* node, const Vector3& origin, const Vector3& dir, float maxDist, float& hitDist) const;
    static bool RayIntersectsTriangle(const Vector3& origin, const Vector3& dir, const TriangleCombined& tri, float& t);
};
