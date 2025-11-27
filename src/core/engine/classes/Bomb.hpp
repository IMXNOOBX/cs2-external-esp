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
    bool is_planted = false;
    BombSite site = BombSite::Unknown;

private:
    uintptr_t address;
};