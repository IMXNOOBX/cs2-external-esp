#pragma once
#include "../memory/memory.hpp"
#include "../classes/vector.hpp"

struct view_matrix_t {
	float* operator[ ](int index) {
		return matrix[index];
	}

	float matrix[4][4];
};

class CC4 {
public:
	uintptr_t get_planted();
	uintptr_t get_node();
	Vector3 get_origin();
};

class CPlayer {
public:
	uintptr_t entity;
	int team;
	uintptr_t pCSPlayerPawn;
	int health;
	int armor;
	std::string name;
	Vector3 origin;
	Vector3 head;
	Vector3 skull;
	bool is_defusing;
	int32_t money;
	float flashAlpha;
	std::string weapon;
};

class CGame
{
public:
	std::shared_ptr<pProcess> process;
	ProcessModule base_client;
	ProcessModule base_engine;
	RECT game_bounds;
	uintptr_t buildNumber;
	bool inGame;
	Vector3 localOrigin;
	bool isC4Planted;
	int localTeam;
	CC4 c4;
	std::vector<CPlayer> players = {};
	void init();
	void loop();
	void close();
	Vector3 world_to_screen(Vector3* v);
private:
	view_matrix_t view_matrix;
	uintptr_t entity_list;
	uintptr_t localPlayer;
	std::uint32_t localPlayerPawn;
	uintptr_t localList_entry2;
	uintptr_t localpCSPlayerPawn;
	std::string read_string(uintptr_t addr);
};

inline CGame g_game;