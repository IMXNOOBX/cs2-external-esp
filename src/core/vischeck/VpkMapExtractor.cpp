#include "VpkMapExtractor.h"

#include "PathUtil.h"
#include "Math.hpp"

#include <vpkpp/PackFile.h>

#include <fstream>
#include <vector>

namespace {
	struct ProcessResult {
		bool ok{};
		DWORD exitCode{ 1 };
		std::string output;
	};

	ProcessResult RunProcess(const std::wstring& cmd) {
		ProcessResult result{};
		SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
		HANDLE readPipe{}, writePipe{};
		if (!CreatePipe(&readPipe, &writePipe, &sa, 0))
			return result;

		SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFOW si{};
		PROCESS_INFORMATION pi{};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdOutput = writePipe;
		si.hStdError = writePipe;

		std::vector<wchar_t> buf(cmd.begin(), cmd.end());
		buf.push_back(L'\0');

		if (!CreateProcessW(nullptr, buf.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
			CloseHandle(readPipe);
			CloseHandle(writePipe);
			return result;
		}

		CloseHandle(writePipe);
		char chunk[512]{};
		DWORD read = 0;
		while (ReadFile(readPipe, chunk, sizeof(chunk) - 1, &read, nullptr) && read > 0) {
			chunk[read] = '\0';
			result.output += chunk;
		}

		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &result.exitCode);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		CloseHandle(readPipe);

		result.ok = result.exitCode == 0;
		return result;
	}

	std::wstring Quote(const std::filesystem::path& p) {
		return L"\"" + p.wstring() + L"\"";
	}

	bool WriteTemp(const std::filesystem::path& path, const std::string& data) {
		std::error_code ec;
		std::filesystem::create_directories(path.parent_path(), ec);
		std::ofstream out(path, std::ios::binary);
		if (!out)
			return false;
		out.write(data.data(), static_cast<std::streamsize>(data.size()));
		return out.good();
	}

} // namespace

std::optional<std::string> VpkMapExtractor::EnsureTriCache(const std::string& map) {
	const auto cache = PathUtil::MapCachePath(map);
	if (std::filesystem::exists(cache))
		return std::nullopt;

	const auto root = PathUtil::FindCs2Root();
	if (!root) {
		LOGF(WARNING, "VisCheck: CS2 install not found for map '{}'", map);
		return "CS2 install not found (cs2.exe path unavailable)";
	}

	const auto vpkPath = PathUtil::MapVpkPath(*root, map);
	if (!std::filesystem::exists(vpkPath)) {
		LOGF(WARNING, "VisCheck: map VPK missing for '{}': {}", map, vpkPath.string());
		return "map VPK not found: " + vpkPath.string();
	}

	auto vpk = vpkpp::PackFile::open(vpkPath.string());
	if (!vpk) {
		LOGF(WARNING, "VisCheck: failed to open VPK for '{}': {}", map, vpkPath.string());
		return "failed to open VPK: " + vpkPath.string();
	}

	const auto entry = "maps/" + map + "/world_physics.vmdl_c";
	if (!vpk->hasEntry(entry)) {
		LOGF(WARNING, "VisCheck: VPK entry missing for '{}': {}", map, entry);
		return "VPK entry missing: " + entry;
	}

	const auto bytes = vpk->readEntry(entry);
	if (!bytes || bytes->empty()) {
		LOGF(WARNING, "VisCheck: failed to read VPK entry for '{}': {}", map, entry);
		return "failed to read VPK entry: " + entry;
	}

	const auto tempDir = PathUtil::ExeDirectory() / "maps" / ".tmp" / map;
	const auto vmdlPath = tempDir / "world_physics.vmdl_c";
	if (!WriteTemp(vmdlPath, std::string(reinterpret_cast<const char*>(bytes->data()), bytes->size()))) {
		LOGF(WARNING, "VisCheck: failed to write temp vmdl for '{}': {}", map, vmdlPath.string());
		return "failed to write temp vmdl: " + vmdlPath.string();
	}

	const auto vrf = PathUtil::VrfExtractPath();
	if (!std::filesystem::exists(vrf)) {
		LOGF(WARNING, "VisCheck: VrfExtract.exe missing beside exe: {}", vrf.string());
		return "VrfExtract.exe not found: " + vrf.string();
	}

	const auto cmd = Quote(vrf) + L" " + Quote(vmdlPath) + L" " + Quote(cache);
	const auto proc = RunProcess(cmd);
	if (!proc.ok) {
		LOGF(WARNING, "VisCheck: VrfExtract failed for '{}' (exit {}): {}", map, proc.exitCode, proc.output);
		return "VrfExtract failed for " + vmdlPath.filename().string() + " (exit " + std::to_string(proc.exitCode) + ")";
	}

	if (!std::filesystem::exists(cache) || std::filesystem::file_size(cache) == 0) {
		LOGF(WARNING, "VisCheck: VrfExtract produced no tri output for '{}'", map);
		return "VrfExtract produced empty tri: " + cache.string();
	}

	const auto triBytes = std::filesystem::file_size(cache);
	const auto triCount = triBytes / sizeof(TriangleCombined);
	LOGF(INFO, "VisCheck: built tri cache for '{}' ({} triangles, {} bytes)", map, triCount, triBytes);
	return std::nullopt;
}

extern "C" {
	unsigned long s_read_getrandom(unsigned char*, unsigned long) { return 0; }
	unsigned long s_read_arc4random(unsigned char*, unsigned long) { return 0; }
	unsigned long s_read_urandom(unsigned char*, unsigned long) { return 0; }
}
