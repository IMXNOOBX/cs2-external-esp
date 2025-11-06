#pragma once


class Entity {
public:
    Entity() : index(0), le(0) {
        this->address = 0;
        this->instance = 0;
    }
    
    Entity(int index, uintptr_t le) : index(index), le(le) {
        this->address = 0;
        this->instance = 0;
    }
    ~Entity() = default;
    

    bool Update();
private:
    int index;
    uintptr_t le; // List Entry
    
    uintptr_t address = 0;
    uintptr_t instance = 0;

    Vec3_t origin;
};