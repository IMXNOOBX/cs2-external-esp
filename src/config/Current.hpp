#pragma once

namespace cfg {
	inline bool enabled = true;

	namespace esp {
		inline bool team = true;

		inline bool box = true;
		inline bool armor = true;
		inline bool health = true;
		inline bool skeleton = true;
		inline bool head_tracker = true;

		inline bool spotted = false;

		namespace flags {
			inline bool name = true;
			inline bool ping = true;
			inline bool weapon = false;
			inline bool defusing = false;
			inline bool money = false;
			inline bool flashed = false;
		}

		namespace colors {
			inline color_t box_team{ 0.f, 1.f, 0.29f, 0.5f };	
			inline color_t box_enemy{ 1.f, 0.f, 0.f, 0.5f };

			inline color_t skeleton_team{ 0.f, 1.f, 0.f, 0.5f };
			inline color_t skeleton_enemy{ 1.f, 0.f, 0.f, 0.5f };

			inline color_t tracker_team{ 1.f, 1.f, 1.f, 0.3f };
			inline color_t tracker_enemy{ 1.f, 1.f, 1.f, 0.3f };
		}

	}

	namespace settings {
		inline bool watermark = true;
		inline bool streamproof = false;
	}

#ifdef _DEBUG
	// Not stored, just for testing
	namespace dev {
		inline bool console = true;
		inline int open_menu_key = false;
		inline int cache_refresh_rate = 5;
	}
#endif
}