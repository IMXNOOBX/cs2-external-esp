#pragma once
#include "core/engine/types/Game.hpp"
#include "core/engine/types/Player.hpp"
#include "core/engine/types/Globals.hpp"

//constexpr int MAX_ENTITIES = 1024;
//constexpr int MAX_PLAYERS = 64; // Mo need we get this from Globals

using namespace std::chrono;

class Cache {
public:
	Game game;
	Globals globals;
	std::vector<Player> players;
public:
	static Cache& Get()
	{
		static Cache instance{};
		return instance;
	}

	static Game CopyGame();
	static Globals CopyGlobals();
	static std::vector<Player> CopyPlayers();

	static bool Refresh();
private:
	std::mutex mtx;
	milliseconds duration;
	steady_clock::time_point last;
private:
	bool RefreshImpl();
};