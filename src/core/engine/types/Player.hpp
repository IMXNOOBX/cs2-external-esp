#pragma once
#include "core/engine/types/Bones.hpp"

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

    std::vector<bone_pos> bone_list;
private:
    int index; 
    uintptr_t le; // List Entry

    uintptr_t pawn;
    uintptr_t controller;
    
    bone_data bones[30]{};
private:
    bool GetPawn();
    bool GetController();

    bool UpdatePawn();
    bool UpdateSkeleton();
    bool UpdateController();
};