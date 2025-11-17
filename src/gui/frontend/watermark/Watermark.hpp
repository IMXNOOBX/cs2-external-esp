#pragma once

#include "core/engine/cache/Cache.hpp"

class Watermark {
public:
    ~Watermark() = default;
    Watermark(const Watermark&) = delete;
    Watermark(Watermark&&) = delete;
    Watermark& operator=(const Watermark&) = delete;
    Watermark& operator=(Watermark&&) = delete;

    static bool Init();
    static void Render();
private:
    Watermark() {};

    static Watermark& GetInstance()
    {
        static Watermark i{};
        return i;
    }

    bool InitImpl();
    void RenderImpl();

    ImFont* font;
};