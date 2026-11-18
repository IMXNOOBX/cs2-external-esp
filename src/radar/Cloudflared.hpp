#pragma once

#include <filesystem>

namespace radar {
    std::filesystem::path FindCloudflared(const std::filesystem::path& appDirectory);
}
