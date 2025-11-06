#pragma once
#include "core/engine/types/Player.hpp"
#include "core/engine/types/Globals.hpp"

//constexpr int MAX_ENTITIES = 1024;
//constexpr int MAX_PLAYERS = 64; // Mo need we get this from Globals

class Cache {
public:
	Globals globals;
	std::vector<Player> players;
public:
	static Cache& Get()
	{
		static Cache instance{};
		return instance;
	}

	static bool Refresh();
private:
	bool RefreshImpl();
};