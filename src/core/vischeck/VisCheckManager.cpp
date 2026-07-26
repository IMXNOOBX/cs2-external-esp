#include "VisCheckManager.h"

#include "core/logger/LogHelper.hpp"

#include <cctype>
#include <filesystem>
#include <mutex>
#include <string>

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
        if (map.empty()) return false;
        if (map.front() == '<') return false;
        if (map == "lobby") return false;
        for (unsigned char c : map) {
            if (!std::isalnum(c) && c != '_')
                return false;
        }
        return true;
    }

    std::filesystem::path ExeDir() {
        char buf[MAX_PATH]{};
        GetModuleFileNameA(nullptr, buf, sizeof(buf));
        return std::filesystem::path(buf).parent_path();
    }

    std::filesystem::path MapTriPath(const std::string& map) {
        return ExeDir() / "maps" / std::filesystem::path(map + ".tri");
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

VisCheck::HitResult VisCheckManager::RayCast(const Vec3_t& origin, const Vec3_t& dir, float maxDist) {
    auto& self = Get();
    std::lock_guard lock(self.mtx_);
    if (!self.vis_ || !self.vis_->IsReady())
        return {};

    return self.vis_->RayCast(
        { origin.x, origin.y, origin.z },
        { dir.x, dir.y, dir.z },
        maxDist
    );
}

void VisCheckManager::SetVisCheck(std::unique_ptr<VisCheck> vis) {
    std::lock_guard lock(mtx_);
    vis_ = std::move(vis);
}

void VisCheckManager::LoadAsync(std::string map) {
    {
        std::lock_guard lock(mtx_);
        if (pendingMap_ == map && loading_)
            return;
        pendingMap_ = map;
    }

    if (worker_.joinable())
        worker_.join();

    loading_ = true;
    worker_ = std::thread([this, map = std::move(map)]() {
        const auto triPath = MapTriPath(map);
        if (std::filesystem::exists(triPath)) {
            auto vis = std::make_unique<VisCheck>(triPath.string());
            if (vis->IsReady()) {
                const auto triCount = vis->TriangleCount();
                SetVisCheck(std::move(vis));
                LOGF(INFO, "VisCheck: loaded '{}' ({} triangles)", map, triCount);
            } else {
                SetVisCheck(nullptr);
                LOGF(WARNING, "VisCheck: tri file invalid for '{}'", map);
            }
        } else {
            SetVisCheck(nullptr);
        }
        loading_ = false;
    });
}