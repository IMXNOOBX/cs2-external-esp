#pragma once

namespace cfg {
	inline bool enabled = true;

	namespace esp {
		inline bool team = false;

		inline bool box = true;
		inline bool skeleton = true;
		inline bool head_tracker = true;

		namespace flags {
			inline bool name = true;
			inline bool health = true;
			inline bool armor = true;
			inline bool defusing = false;
			inline bool money = false;
			inline bool flashed = false;
		}
	}

	namespace utils {
		inline bool console = true;
		inline bool streamproof = false;
		inline int open_menu_key = false;
	}
}