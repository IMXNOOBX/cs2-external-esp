#pragma once
#include "core/engine/classes/Bones.hpp"
#include "core/engine/classes/Weapon.hpp"
#include "core/engine/classes/ObserverServices.hpp"

class Player {
public:
    Player() {}
    Player(int index, uintptr_t el, uintptr_t le) 
        : index(index), entity_list(el), list_entry(le){}

    bool Update();
    bool GetBounds(view_matrix_t matrix, Vec2_t size, std::pair<Vec2_t, Vec2_t>& bounds);
public:
    Vec3_t pos;

    int ping = 0;
    int team = 0;
    int health = 0;
    int armor = 0;
    int money = 0;

    bool bot = true;
    bool alive = false;
    bool scoped = false;
    bool flashed = false;
    bool spotted = false;
    bool defusing = false;
    bool localplayer = false;

    char name[32];
    //std::string name;
    uint64_t steam_id{};

    Weapon weapon;
    int32_t ammo;
    bool is_reloading;

    std::vector<bone_pos> bone_list;

    int pawn_controller_addr;
    ObserverServices observer_services;
private:
    int index;

    uintptr_t list_entry;
    uintptr_t entity_list;

    uintptr_t pawn;
    uintptr_t controller;
    
    bone_data bones[30]{};
private:
    bool GetPawn();
    bool GetController();
    bool GetObserverServices();

    bool UpdatePawn();
    bool UpdateWeapon();
    bool UpdateSkeleton();
    bool UpdateController();
    bool UpdateObserverServices();
    bool UpdateObserverServices2();
};