#pragma once
#include "Entity.hpp"

class Player : public Entity {
public:
    Player() : Entity() {}
    Player(int index, uintptr_t le) : Entity(index, le) {}
    Player(const Entity& entity) : Entity(entity) {}

    bool Update();
private:
    uintptr_t pawn;

    int team = 0;
    int health = 0;
    int armor = 0;
    int money = 0;

    bool spotted = false;
    bool defusing = false;

    std::string name;
    std::string weapon;
};