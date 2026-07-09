#pragma once

#include "core/engine/cache/Cache.hpp"

class Overlays {
public:
    ~Overlays() = default;
    Overlays(const Overlays&) = delete;
    Overlays(Overlays&&) = delete;
    Overlays& operator=(const Overlays&) = delete;
    Overlays& operator=(Overlays&&) = delete;

    static bool Init();
    static void Render();

private:
    ImFont* font;
    ImFont* font_alt;
    ImFont* font_icons;

    size_t vel_index = 0;
    std::vector<int> vel_buffer;
    float vel_accumulator = 0.0f;
private:
    Overlays() {};

    static Overlays& GetInstance()
    {
        static Overlays i{};
        return i;
    }

    bool InitImpl();
    void RenderImpl();

    void RenderBomb();
    void RenderRadar();
    void RenderNotice();
    void RenderWatermark();
    void RenderSpeedChart();
    void RenderDebugWindow();
    void RenderSpectatorList();
};