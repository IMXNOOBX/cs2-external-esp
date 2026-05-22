#include "VisCheck.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>

namespace {
	constexpr size_t kLeafThreshold = 4;

	bool LoadTriFile(const std::string& path, std::vector<TriangleCombined>& out) {
		std::ifstream in(path, std::ios::binary);
		if (!in)
			return false;

		in.seekg(0, std::ios::end);
		const auto size = in.tellg();
		in.seekg(0, std::ios::beg);
		if (size <= 0 || size % sizeof(TriangleCombined) != 0)
			return false;

		const auto count = static_cast<size_t>(size) / sizeof(TriangleCombined);
		out.resize(count);
		in.read(reinterpret_cast<char*>(out.data()), size);
		return in.good();
	}
} // namespace

VisCheck::VisCheck(const std::string& triFile) {
	std::vector<TriangleCombined> tris;
	if (LoadTriFile(triFile, tris))
		BuildFromTriangles(std::move(tris));
}

VisCheck::VisCheck(std::vector<TriangleCombined> triangles) {
	BuildFromTriangles(std::move(triangles));
}

void VisCheck::BuildFromTriangles(std::vector<TriangleCombined> triangles) {
	if (triangles.empty())
		return;
	triCount_ = triangles.size();
	bvhNodes.push_back(BuildBVH(triangles));
}

std::unique_ptr<BVHNode> VisCheck::BuildBVH(const std::vector<TriangleCombined>& tris) {
	auto node = std::make_unique<BVHNode>();
	if (tris.empty())
		return node;

	AABB bounds = tris[0].ComputeAABB();
	for (size_t i = 1; i < tris.size(); ++i) {
		const auto a = tris[i].ComputeAABB();
		bounds.min.x = std::min(bounds.min.x, a.min.x);
		bounds.min.y = std::min(bounds.min.y, a.min.y);
		bounds.min.z = std::min(bounds.min.z, a.min.z);
		bounds.max.x = std::max(bounds.max.x, a.max.x);
		bounds.max.y = std::max(bounds.max.y, a.max.y);
		bounds.max.z = std::max(bounds.max.z, a.max.z);
	}
	node->bounds = bounds;

	if (tris.size() <= kLeafThreshold) {
		node->triangles = tris;
		return node;
	}

	const Vector3 diff = bounds.max - bounds.min;
	const int axis = (diff.x > diff.y && diff.x > diff.z) ? 0 : (diff.y > diff.z ? 1 : 2);
	auto sorted = tris;
	std::sort(sorted.begin(), sorted.end(), [axis](const TriangleCombined& a, const TriangleCombined& b) {
		const auto aa = a.ComputeAABB(), ab = b.ComputeAABB();
		const float ca = axis == 0 ? (aa.min.x + aa.max.x) : axis == 1 ? (aa.min.y + aa.max.y) : (aa.min.z + aa.max.z);
		const float cb = axis == 0 ? (ab.min.x + ab.max.x) : axis == 1 ? (ab.min.y + ab.max.y) : (ab.min.z + ab.max.z);
		return ca < cb;
	});

	const size_t mid = sorted.size() / 2;
	node->left = BuildBVH({ sorted.begin(), sorted.begin() + static_cast<std::ptrdiff_t>(mid) });
	node->right = BuildBVH({ sorted.begin() + static_cast<std::ptrdiff_t>(mid), sorted.end() });
	return node;
}

bool VisCheck::IntersectBVH(const BVHNode* node, const Vector3& origin, const Vector3& dir, float maxDist, float& hitDist) const {
	if (!node || !node->bounds.RayIntersects(origin, dir))
		return false;

	bool hit = false;
	if (node->IsLeaf()) {
		for (const auto& tri : node->triangles) {
			float t;
			if (RayIntersectsTriangle(origin, dir, tri, t) && t < maxDist && t < hitDist) {
				hitDist = t;
				hit = true;
			}
		}
		return hit;
	}

	if (node->left)
		hit |= IntersectBVH(node->left.get(), origin, dir, maxDist, hitDist);
	if (node->right)
		hit |= IntersectBVH(node->right.get(), origin, dir, maxDist, hitDist);
	return hit;
}

bool VisCheck::IsPointVisible(const Vector3& from, const Vector3& to) const {
	if (!IsReady())
		return false;

	Vector3 delta = to - from;
	const float distance = std::sqrt(delta.dot(delta));
	if (distance < 1e-4f)
		return true;

	const Vector3 direction = { delta.x / distance, delta.y / distance, delta.z / distance };
	constexpr float originPadding = 32.f;
	if (distance <= originPadding)
		return true;

	const Vector3 rayOrigin = {
		from.x + direction.x * originPadding,
		from.y + direction.y * originPadding,
		from.z + direction.z * originPadding
	};
	const float maxTraceDistance = distance - originPadding;
	float hitDistance = std::numeric_limits<float>::max();

	for (const auto& root : bvhNodes) {
		if (IntersectBVH(root.get(), rayOrigin, direction, maxTraceDistance, hitDistance)
			&& hitDistance < maxTraceDistance - 1.f)
			return false;
	}
	return true;
}

bool VisCheck::RayIntersectsTriangle(const Vector3& origin, const Vector3& dir, const TriangleCombined& tri, float& t) {
	constexpr float kEps = 1e-7f;
	const Vector3 e1 = tri.v1 - tri.v0;
	const Vector3 e2 = tri.v2 - tri.v0;
	const Vector3 h = dir.cross(e2);
	const float a = e1.dot(h);
	if (a > -kEps && a < kEps)
		return false;

	const float f = 1.f / a;
	const Vector3 s = origin - tri.v0;
	const float u = f * s.dot(h);
	if (u < 0.f || u > 1.f)
		return false;

	const Vector3 q = s.cross(e1);
	const float v = f * dir.dot(q);
	if (v < 0.f || u + v > 1.f)
		return false;

	t = f * e2.dot(q);
	return t > kEps;
}
