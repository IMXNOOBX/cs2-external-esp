#pragma once

#include "core/engine/cache/Cache.hpp"

#include <chrono>
#include <unordered_map>

class Esp {
public:
    ~Esp() = default;
    Esp(const Esp&) = delete;
    Esp(Esp&&) = delete;
    Esp& operator=(const Esp&) = delete;
    Esp& operator=(Esp&&) = delete;

    static bool Init();
    static void Render();

private:
    ImGuiIO io;
    ImFont* font;
    ImFont* font_merged_icons;
    ImDrawList* d;

    // Temporary storage for ease
    view_matrix_t matrix;

    struct SoundMarker {
        Vec3_t pos;
        std::chrono::steady_clock::time_point birth;
        bool mate;
        int kind; // 0 = footstep, 1 = gunfire, 2 = reload
    };
    struct HitMarker {
        Vec3_t pos;
        std::chrono::steady_clock::time_point birth;
        bool mate;
        int damage;
    };
    struct PlayerTrack {
        float last_ammo = -1.f;
        int last_health = -1;
        short last_weapon = -1;
        bool was_reloading = false;
        std::chrono::steady_clock::time_point last_footstep{};
        std::chrono::steady_clock::time_point last_seen{};
    };
    std::vector<SoundMarker> sound_markers;
    std::vector<HitMarker> hit_markers;
    std::unordered_map<int, PlayerTrack> tracks;
private:
    Esp() {};

    static Esp& GetInstance()
    {
        static Esp i{};
        return i;
    }

    bool InitImpl();
    void RenderImpl();

    void RenderPlayer(Player player, bool mate = false, bool visible = false);
    void RenderPlayerBones(Player player, bool mate = false, bool use_vis = false);
    void RenderPlayerBars(Player player, std::pair<Vec2_t, Vec2_t> bounds);
    void RenderPlayerFalgs(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate = false);
<<<<<<< Updated upstream
    void RenderBombBox(Bomb bomb);
    void RenderPlayerTracker(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate = false);
    void RenderPlayerTracers(Player source, Player player, bool mate = false);

=======
    void RenderPlayerTracker(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate = false, bool use_vis = false);
    void RenderPlayerTracers(Player source, Player player, bool mate = false);
    void RenderPlayerBox3D(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate = false, bool use_vis = false);
    
    void RenderBombBox(Bomb bomb);
>>>>>>> Stashed changes
	void RenderCrosshair(Player local);
};
