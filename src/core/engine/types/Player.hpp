#pragma once

class Player {
public:
    Player(int index, uintptr_t le) : index(index), le(le){}

    bool Update();
    bool GetBounds(view_matrix_t matrix, Vec2_t size, std::pair<Vec2_t, Vec2_t>& bounds);
public:
    Vec3_t pos;

    int team = 0;
    int health = 0;
    int armor = 0;
    int money = 0;

    bool bot = true;
    bool alive = false;
    bool spotted = false;
    bool defusing = false;
    bool localplayer = false;

    char name[32];
    //std::string name;
    uint64_t steam_id{};

    char weapon[32];
    //std::string weapon;

private:
    int index; 
    uintptr_t le; // List Entry

    uintptr_t pawn;
    uintptr_t controller;
private:
    bool GetPawn();
    bool GetController();

    bool UpdatePawn();
    bool UpdateController();
};