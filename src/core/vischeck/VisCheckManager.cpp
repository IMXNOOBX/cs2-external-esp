#include "VisCheckManager.h"

#include "PathUtil.h"
#include "VpkMapExtractor.h"

#include <algorithm>
#include <cctype>

namespace {
	std::string TrimMapName(const char* mapName) {
		if (!mapName)
			return {};
		std::string map(mapName);
		if (const auto nul = map.find('\0'); nul != std::string::npos)
			map.resize(nul);
		if (map.starts_with("maps/"))
			map.erase(0, 5);
		if (map.size() > 4 && map.ends_with(".vpk"))
			map.resize(map.size() - 4);
		return map;
	}

	bool IsValidMap(const std::string& map) {
		if (map.empty())
			return false;
		if (map.front() == '<')
			return false;
		if (map == "lobby")
			return false;
		for (unsigned char c : map) {
			if (!std::isalnum(c) && c != '_')
				return false;
		}
		return true;
	}

	bool IsValidTriCache(const std::filesystem::path& cache) {
		if (!std::filesystem::exists(cache))
			return false;
		std::error_code ec;
		const auto sz = std::filesystem::file_size(cache, ec);
		return !ec && sz > 0 && sz % sizeof(TriangleCombined) == 0;
	}
} // namespace

VisCheckManager& VisCheckManager::Get() {
	static VisCheckManager inst;
	return inst;
}

void VisCheckManager::OnMapChanged(const char* mapName) {
	const auto map = TrimMapName(mapName);
	if (!IsValidMap(map)) {
		LOGF(VERBOSE, "VisCheck: skipping invalid map '{}'", map.empty() ? "<empty>" : map);
		return;
	}

	Get().LoadAsync(map);
}

bool VisCheckManager::IsReady() {
	auto& self = Get();
	std::lock_guard lock(self.mtx_);
	return self.vis_ && self.vis_->IsReady();
}

bool VisCheckManager::IsVisible(const Vec3_t& from, const Vec3_t& to) {
	auto& self = Get();
	std::lock_guard lock(self.mtx_);
	if (!self.vis_ || !self.vis_->IsReady())
		return false;

	return self.vis_->IsPointVisible(
		{ from.x, from.y, from.z },
		{ to.x, to.y, to.z }
	);
}

void VisCheckManager::SetVisCheck(std::unique_ptr<VisCheck> vis) {
	std::lock_guard lock(mtx_);
	vis_ = std::move(vis);
}

void VisCheckManager::LoadAsync(std::string map) {
	{
		std::lock_guard lock(mtx_);
		if (pendingMap_ == map && (loading_ || (vis_ && vis_->IsReady())))
			return;
		pendingMap_ = map;
	}

	if (worker_.joinable())
		worker_.join();

	loading_ = true;
	worker_ = std::thread([this, map = std::move(map)]() {
		const auto cache = PathUtil::MapCachePath(map);
		bool ok = IsValidTriCache(cache);

		if (!ok) {
			std::error_code ec;
			if (std::filesystem::exists(cache))
				std::filesystem::remove(cache, ec);

			if (const auto err = VpkMapExtractor::EnsureTriCache(map))
				LOGF(WARNING, "VisCheck: {} — spotted ESP disabled", *err);
			ok = IsValidTriCache(cache);
		}

		if (ok) {
			auto vis = std::make_unique<VisCheck>(cache.string());
			if (vis->IsReady()) {
				const auto triCount = vis->TriangleCount();
				SetVisCheck(std::move(vis));
				LOGF(INFO, "VisCheck: loaded map '{}' ({} triangles)", map, triCount);
			}
			else {
				SetVisCheck(nullptr);
				std::error_code ec;
				std::filesystem::remove(cache, ec);
				LOGF(WARNING, "VisCheck: tri cache invalid for '{}'", map);
			}
		}
		else
			SetVisCheck(nullptr);

		loading_ = false;
	});
}
