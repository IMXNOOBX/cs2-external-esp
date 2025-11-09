#pragma once

namespace cfg {
	inline bool enabled = true;

	namespace esp {
		inline bool team = false;

		inline bool box = true;
		inline bool health = true;
		inline bool skeleton = true;
		inline bool head_tracker = true;

		namespace flags {
			inline bool name = true;
			inline bool armor = true;
			inline bool defusing = false;
			inline bool money = false;
			inline bool flashed = false;
		}

		namespace colors {
			inline color_t box_team{ 0.29f, 0.69f, 0.29f };
			inline color_t box_enemy{ 0.88f, 0.29f, 0.29f };

			inline color_t skeleton_team{ 0.29f, 0.69f, 0.29f };
			inline color_t skeleton_enemy{ 0.88f, 0.29f, 0.29f };

			inline color_t tracker_team{ 0.98f, 0.98f, 0.98f };
			inline color_t tracker_enemy{ 0.98f, 0.98f, 0.98f };
		}
	}

	namespace settings {
		inline bool console = true;
		inline bool streamproof = false;
		inline int open_menu_key = false;
	}
}