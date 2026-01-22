#pragma once

enum class BombSite {
    Unknown = -1,
    A = 0,
    B = 1
};

class Bomb {
public:
    Bomb() {};

    bool Update();
public:
    Vec3_t pos;
    float time_left = 0.f;
    bool is_planted = false;
    BombSite site = BombSite::Unknown;
private:
    uintptr_t address;
    static bool prev_is_planted;
    static std::time_t plant_time;
};