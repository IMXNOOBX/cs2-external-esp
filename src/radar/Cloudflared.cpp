#include "Cloudflared.hpp"
#include <windows.h>
#include <string>

namespace {
    namespace fs = std::filesystem;

    constexpr wchar_t executableName[] = L"cloudflared.exe";

    fs::path Existing(const fs::path& candidate) {
        std::error_code ignored;
        return fs::is_regular_file(candidate, ignored) ? candidate : fs::path{};
    }

    fs::path Environment(const wchar_t* name) {
        DWORD size = GetEnvironmentVariableW(name, nullptr, 0);
        if (!size) return {};
        std::wstring value(size, L'\0');
        DWORD written = GetEnvironmentVariableW(name, value.data(), size);
        if (!written || written >= size) return {};
        value.resize(written);
        return fs::path(value);
    }

    // Searches PATH by hand instead of using SearchPathW: that helper also looks in the current
    // working directory, which is not ours to trust.
    fs::path OnPath() {
        auto path = Environment(L"PATH").wstring();
        for (size_t begin = 0; begin <= path.size();) {
            auto end = path.find(L';', begin);
            if (end == std::wstring::npos) end = path.size();
            std::wstring entry = path.substr(begin, end - begin);
            begin = end + 1;
            if (entry.size() >= 2 && entry.front() == L'"' && entry.back() == L'"')
                entry = entry.substr(1, entry.size() - 2);
            // An empty entry means the current directory, so it is skipped on purpose.
            if (entry.empty()) continue;
            if (auto found = Existing(fs::path(entry) / executableName); !found.empty()) return found;
        }
        return {};
    }
}

fs::path radar::FindCloudflared(const fs::path& appDirectory) {
    // A copy beside the program is pinned to the release, so prefer it over anything installed.
    for (const auto& candidate : { appDirectory / executableName, appDirectory / L"cloudflare" / executableName })
        if (auto found = Existing(candidate); !found.empty()) return found;

    // The official MSI installs into Program Files (x86), and its PATH entry is known to be
    // malformed, so check the install folders before relying on PATH.
    for (const wchar_t* variable : { L"ProgramFiles(x86)", L"ProgramFiles", L"ProgramW6432" }) {
        auto folder = Environment(variable);
        if (folder.empty()) continue;
        if (auto found = Existing(folder / L"cloudflared" / executableName); !found.empty()) return found;
    }

    // winget used to install cloudflared as a portable package with a link in this folder.
    auto local = Environment(L"LocalAppData");
    if (!local.empty())
        if (auto found = Existing(local / L"Microsoft" / L"WinGet" / L"Links" / executableName); !found.empty())
            return found;

    // Chocolatey, Scoop and manual installs end up here.
    return OnPath();
}
