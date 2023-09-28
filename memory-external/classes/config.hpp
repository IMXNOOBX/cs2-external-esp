#pragma once
#include <fstream>
#include "json.hpp"

namespace config {
	extern bool read();
	extern void save();

	inline std::ptrdiff_t dwLocalPlayer = 0x17DB108;
	inline std::ptrdiff_t dwEntityList = 0x178C888;
	inline std::ptrdiff_t dwViewMatrix = 0x187A6E0;

	inline std::ptrdiff_t dwPawnHealth = 0x808;
	inline std::ptrdiff_t dwPlayerPawn = 0x7FC;
	inline std::ptrdiff_t dwSanitizedName = 0x720;
	inline std::ptrdiff_t m_iTeamNum = 0x3bf;
	inline std::ptrdiff_t m_vecOrigin = 0x1204;
}