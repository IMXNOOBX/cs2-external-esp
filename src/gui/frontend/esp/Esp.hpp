#pragma once

#include "core/engine/cache/Cache.hpp"

class Esp {
public:
    ~Esp() = default;
    Esp(const Esp&) = delete;
    Esp(Esp&&) = delete;
    Esp& operator=(const Esp&) = delete;
    Esp& operator=(Esp&&) = delete;

    static void Render();
private:
    Esp() {};

    static Esp& GetInstance()
    {
        static Esp i{};
        return i;
    }

    void RenderImpl();

    view_matrix_t matrix;

    void RenderPlayer(Player player, bool mate = false);
    void RenderPlayerBones(Player player, bool mate = false);
    void RenderPlayerFalgs(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate = false);
    void RenderPlayerTracker(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate = false);
};