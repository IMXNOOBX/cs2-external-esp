#pragma once

class Globals {
public:
    Globals() {};

    bool Update();
public:
    int max_clients;

    long current_time;

	char map_name[32];

	bool in_match;
private:
    uintptr_t address;
};