#pragma once

struct color_t {
    float r, g, b, a;

    color_t() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    color_t(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    color_t(const ImColor& col) : r(col.Value.x), g(col.Value.y), b(col.Value.z), a(col.Value.w) {}
    color_t(const ImVec4& vec) : r(vec.x), g(vec.y), b(vec.z), a(vec.w) {}

    operator ImColor() const { return ImColor(r, g, b, a); }
    operator ImVec4() const { return ImVec4(r, g, b, a); }

    float* data() { return &r; }               // <--- added
    const float* data() const { return &r; }   // <--- added
};