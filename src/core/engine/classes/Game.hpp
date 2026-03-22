#pragma once

class Game {
public:
    Game() {};

    bool Update();
    bool UpdateMatrix();
    bool UpdateEntityList();

public:
    view_matrix_t view_matrix;

    uintptr_t entity_list;
    uintptr_t list_entry;
private:
    uintptr_t address;
};