#pragma once

#include "core/engine/cache/Cache.hpp"

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
    ImDrawList* d;

    // Temporary storage for ease
    view_matrix_t matrix;
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
    void RenderPlayerBones(Player player, bool mate, bool visible);
    void RenderPlayerBars(Player player, std::pair<Vec2_t, Vec2_t> bounds);
    void RenderPlayerFalgs(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate = false);
    void RenderPlayerTracker(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate, bool visible);
    void RenderPlayerTracers(Player source, Player player, bool mate = false);

	void RenderCrosshair(Player local);
    void RenderBomb(Player local, Bomb bomb);
};