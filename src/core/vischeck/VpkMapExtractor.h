#pragma once

#include <optional>
#include <string>

class VpkMapExtractor {
public:
    static std::optional<std::string> EnsureTriCache(const std::string& map);
};
