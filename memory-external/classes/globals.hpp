#pragma once
#include <wtypes.h>

namespace g {
	inline HDC hdcBuffer = NULL;
	inline HBITMAP hbmBuffer = NULL;

	bool outdated_offsets = false;

	RECT gameBounds;
}
