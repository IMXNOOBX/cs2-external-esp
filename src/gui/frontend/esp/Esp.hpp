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

    void RenderPlayer(Player players, view_matrix_t matrix);
    void RenderPlayerBones(Player player, view_matrix_t matrix);
    void RenderPlayerFalgs(Player player, view_matrix_t matrix, std::pair<Vec2_t, Vec2_t> bounds);
    void RenderPlayerTracker(Player player, view_matrix_t matrix, std::pair<Vec2_t, Vec2_t> bounds);
};