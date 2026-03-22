#pragma once
#include "core/engine/classes/Game.hpp"
#include "core/engine/classes/Bomb.hpp"
#include "core/engine/classes/Player.hpp"
#include "core/engine/classes/Globals.hpp"

//constexpr int MAX_ENTITIES = 1024;
//constexpr int MAX_PLAYERS = 64; // No need we get this from Globals

using namespace std::chrono;

struct Snapshot {
	Game game;
	Bomb bomb;
	Globals globals;
	std::vector<Player> players;
};

class Cache {
public:
	Game game;
	Bomb bomb;
	Globals globals;
	std::vector<Player> players;
public:
	static Cache& Get()
	{
		static Cache instance{};
		return instance;
	}

	static Snapshot CopySnapshot();

	static bool Refresh();
private:
	std::mutex mtx;
	milliseconds duration{1};
	steady_clock::time_point last{};
private:
	bool RefreshImpl();
};