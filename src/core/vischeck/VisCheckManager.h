#pragma once

#include "VisCheck.h"
#include "core/engine/types/Vec3.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class VisCheckManager {
public:
    static void OnMapChanged(const char* mapName);
    static bool IsReady();
    static bool IsVisible(const Vec3_t& from, const Vec3_t& to);

private:
    static VisCheckManager& Get();

    void LoadAsync(std::string map);
    void SetVisCheck(std::unique_ptr<VisCheck> vis);

    std::mutex mtx_;
    std::unique_ptr<VisCheck> vis_;
    std::atomic<bool> loading_{ false };
    std::thread worker_;
    std::string pendingMap_;
};
