#pragma once

namespace cfg {
	inline bool enabled = true;

	namespace esp {
		inline bool team = true;

		inline bool box = true;
		inline bool box_3d = false;
		inline bool armor = true;
		inline bool health = true;
		inline bool skeleton = true;
		inline bool head_tracker = true;
		inline bool health_number = false;

		inline bool spotted = false;
		inline bool bomb = true;
		inline color_t bomb_color{ 1.f, 0.84f, 0.f, 1.f };

		inline bool los_spotted = false;
		inline bool los_use_visible_colors = true;
		inline bool los_extra_bones = false;

		inline bool tracers = false;

		inline bool sound = false;
		inline bool sound_footsteps = true;
		inline bool sound_gunfire = true;
		inline float sound_fade = 1.5f;

		inline bool hit_markers = false;
		inline float hit_marker_fade = 1.2f;

		namespace flags {
			inline bool name = true;
			inline bool ping = true;
			inline bool weapon = false;
			inline bool ammo = false;
			inline bool reloading = false;
			inline bool defusing = false;
			inline bool money = false;
			inline bool flashed = false;
			inline bool scoped = false;
			inline bool has_c4 = false;
		}

		namespace colors {
			inline color_t box_team{ 0.f, 1.f, 0.29f, 0.5f };
			inline color_t box_enemy{ 1.f, 0.f, 0.f, 0.5f };

			inline color_t skeleton_team{ 0.f, 1.f, 0.f, 0.5f };
			inline color_t skeleton_enemy{ 1.f, 0.f, 0.f, 0.5f };

			inline color_t tracker_team{ 1.f, 1.f, 1.f, 0.3f };
			inline color_t tracker_enemy{ 1.f, 1.f, 1.f, 0.3f };

			inline color_t tracer_team{ 0.f, 1.f, 0.f, 0.5f };
			inline color_t tracer_enemy{ 1.f, 0.f, 0.f, 0.5f };

		namespace sound {
			inline color_t footstep_team{ 0.f, 1.f, 0.6f, 0.5f };
			inline color_t footstep_enemy{ 1.f, 0.62f, 0.f, 0.85f };
			inline color_t gunfire_team{ 1.f, 0.85f, 0.2f, 0.8f };
			inline color_t gunfire_enemy{ 1.f, 0.3f, 0.1f, 0.9f };
			inline color_t reload_team{ 0.4f, 0.7f, 1.f, 0.7f };
			inline color_t reload_enemy{ 0.6f, 0.3f, 1.f, 0.8f };
		}

			inline color_t los_visible_team{ 0.f, 0.8f, 1.f, 0.8f };
			inline color_t los_visible_enemy{ 1.f, 0.84f, 0.f, 0.8f };

			inline color_t hit_marker{ 1.f, 0.2f, 0.2f, 1.f };

			inline color_t bomb{ 1.f, 0.84f, 0.f, 1.f };
			
			namespace flags {
				inline color_t flashed_team{ 1.f, 1.f, 1.f, 0.5f };
				inline color_t flashed_enemy{ 1.f, 1.f, 1.f, 0.8f };

				inline color_t reloading_team{ 1.f, 1.f, 1.f, 0.5f };
				inline color_t reloading_enemy{ 1.f, 1.f, 1.f, 0.8f };

				inline color_t defusing_team{ 1.f, 1.f, 1.f, 0.5f };
				inline color_t defusing_enemy{ 1.f, 1.f, 1.f, 0.8f };

				inline color_t scoped_team{ 1.f, 1.f, 1.f, 0.5f };
				inline color_t scoped_enemy{ 1.f, 1.f, 1.f, 0.8f };

				inline color_t c4_team{ 1.f, 0.84f, 0.f, 1.f };
				inline color_t c4_enemy{ 1.f, 0.84f, 0.f, 1.f };
			}
			
		}

	}

	namespace world {
		namespace spectators {
			inline bool enabled = false;

			inline bool detailed = false;
			inline bool self_only = true;

			inline Vec2_t pos{ 10.f, 100.f };
		}

		namespace bomb {
			inline bool location = true;
			inline bool timer = true;
			inline bool hud = false;
			inline Vec2_t pos{ 10.f, 300.f };
		}

		namespace crosshair {
			inline bool enabled = false;
		}

		namespace radar {
			inline bool enabled = true;
			inline bool no_rotate = false;
			inline float range = 2000.f;
			inline Vec2_t pos{ 10.f, 10.f };
			inline Vec2_t size{ 200.f, 200.f };
		}

		namespace velocity {
			inline bool enabled = false;
			inline int sample_rate = 35;
			inline float sample_length = 5.f;

			inline Vec2_t size{ 400.f, 100.f };
			inline Vec2_t pos{ 10.f, 400.f };
		}
	}

	namespace settings {
		inline bool watermark = true;
		inline bool streamproof = false;
		inline bool vsync = false;
		inline bool free_cpu = true;
	}

	// Not stored, just for testing
	namespace dev {
		inline bool console = true;
		inline int open_menu_key = false;
		inline int cache_refresh_rate = 5;
		inline bool force_show_flags = false;
	}
}