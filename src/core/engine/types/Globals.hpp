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

    uintptr_t localplayer;

private:
    uintptr_t address;
};