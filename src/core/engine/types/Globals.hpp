#pragma once

class Globals {
public:
    static bool Update();
public:
    static int max_clients;

    static long current_time;

	static char map_name[32];

	static bool in_match;
private:
    static uintptr_t address;
};