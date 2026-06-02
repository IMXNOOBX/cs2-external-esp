#include "PathUtil.h"

#include "core/engine/Engine.hpp"

namespace {
	std::filesystem::path ModuleDirectory() {
		wchar_t buf[MAX_PATH]{};
		const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
		if (!n)
			return {};
		return std::filesystem::path(buf).parent_path();
	}

	bool HasCs2Maps(const std::filesystem::path& root) {
		return std::filesystem::exists(root / "game" / "csgo" / "maps");
	}
} // namespace

std::filesystem::path PathUtil::ExeDirectory() {
	return ModuleDirectory();
}

std::filesystem::path PathUtil::MapCachePath(const std::string& map) {
	return ExeDirectory() / "maps" / (map + ".tri");
}

std::filesystem::path PathUtil::VrfExtractPath() {
	return ExeDirectory() / "VrfExtract.exe";
}

std::optional<std::filesystem::path> PathUtil::FindCs2Root() {
	const auto proc = Engine::GetProcess();
	if (!proc)
		return std::nullopt;

	const auto exe = proc->GetExePath();
	if (exe.empty())
		return std::nullopt;

	std::filesystem::path p = exe;
	for (int i = 0; i < 12 && !p.empty(); ++i) {
		if (HasCs2Maps(p))
			return p;
		if (!p.has_parent_path() || p == p.parent_path())
			break;
		p = p.parent_path();
	}
	return std::nullopt;
}

std::filesystem::path PathUtil::MapVpkPath(const std::filesystem::path& cs2Root, const std::string& map) {
	return cs2Root / "game" / "csgo" / "maps" / (map + ".vpk");
}
