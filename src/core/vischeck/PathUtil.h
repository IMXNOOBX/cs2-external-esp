#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace PathUtil {
    std::filesystem::path ExeDirectory();
    std::filesystem::path MapCachePath(const std::string& map);
    std::filesystem::path VrfExtractPath();
    std::optional<std::filesystem::path> FindCs2Root();
    std::filesystem::path MapVpkPath(const std::filesystem::path& cs2Root, const std::string& map);
}
