#pragma once
#include "core/engine/classes/Game.hpp"
#include "core/engine/classes/Bomb.hpp"
#include "core/engine/classes/Player.hpp"
#include "core/engine/classes/Globals.hpp"

//constexpr int MAX_ENTITIES = 1024;
//constexpr int MAX_PLAYERS = 64; // Mo need we get this from Globals

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

	static Game CopyGame();
	static Bomb CopyBomb();
	static Globals CopyGlobals();
	static std::vector<Player> CopyPlayers();

	static Snapshot CopySnapshot();

	static bool Refresh();
private:
	std::mutex mtx;
	milliseconds duration;
	steady_clock::time_point last;
private:
	bool RefreshImpl();
};