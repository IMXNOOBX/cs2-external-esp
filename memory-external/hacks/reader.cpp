#include "reader.hpp"
#include <thread>
#include <map>
#include <cmath>
#include "../classes/auto_updater.hpp"
#include "../classes/config.hpp"

std::map<std::string, int> boneMap = {
	{"head", 6},
	{"neck_0", 5},
	{"spine_1", 4},
	{"spine_2", 2},
	{"pelvis", 0},
	{"arm_upper_L", 8},
	{"arm_lower_L", 9},
	{"hand_L", 10},
	{"arm_upper_R", 13},
	{"arm_lower_R", 14},
	{"hand_R", 15},
	{"leg_upper_L", 22},
	{"leg_lower_L", 23},
	{"ankle_L", 24},
	{"leg_upper_R", 25},
	{"leg_lower_R", 26},
	{"ankle_R", 27}
};

// CC4
uintptr_t CC4::get_planted() {
	return g_game.process->read<uintptr_t>(g_game.process->read<uintptr_t>(g_game.base_client.base + updater::offsets::dwPlantedC4));
}

uintptr_t CC4::get_node() {
	return g_game.process->read<uintptr_t>(get_planted() + updater::offsets::m_pGameSceneNode);
}

Vector3 CC4::get_origin() {
	return g_game.process->read<Vector3>(get_node() + updater::offsets::m_vecAbsOrigin);
}

// CGame
void CGame::init() {
	std::cout << "[cs2] Waiting for cs2.exe..." << std::endl;

	process = std::make_shared<pProcess>();
	while (!process->AttachProcessHj("cs2.exe"))
		std::this_thread::sleep_for(std::chrono::seconds(1));

	std::cout << "[cs2] Attached to cs2.exe\n" << std::endl;

	do {
		base_client = process->GetModule("client.dll");
		base_engine = process->GetModule("engine2.dll");
		if (base_client.base == 0 || base_engine.base == 0) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cout << "[cs2] Failed to find module client.dll/engine2.dll, waiting for the game to load it..." << std::endl;
		}
	} while (base_client.base == 0 || base_engine.base == 0);

	GetClientRect(process->hwnd_, &game_bounds);

	buildNumber = process->read<uintptr_t>(base_engine.base + updater::offsets::dwBuildNumber);
}

void CGame::close() {
	std::cout << "[cs2] Deatachig from process" << std::endl;
	process->Close();
}

void CGame::loop() {
	inGame = false;
	isC4Planted = false;

	localPlayer = process->read<uintptr_t>(base_client.base + updater::offsets::dwLocalPlayerController);
	if (!localPlayer) return;

	localPlayerPawn = process->read<std::uint32_t>(localPlayer + updater::offsets::m_hPlayerPawn);
	if (!localPlayerPawn) return;

	entity_list = process->read<uintptr_t>(base_client.base + updater::offsets::dwEntityList);

	localList_entry2 = process->read<uintptr_t>(entity_list + 0x8 * ((localPlayerPawn & 0x7FFF) >> 9) + 16);
	localpCSPlayerPawn = process->read<uintptr_t>(localList_entry2 + 120 * (localPlayerPawn & 0x1FF));
	if (!localpCSPlayerPawn) return;

	view_matrix = process->read<view_matrix_t>(base_client.base + updater::offsets::dwViewMatrix);

	localTeam = process->read<int>(localPlayer + updater::offsets::m_iTeamNum);
	localOrigin = process->read<Vector3>(localpCSPlayerPawn + updater::offsets::m_vOldOrigin);
	isC4Planted = process->read<bool>(base_client.base + updater::offsets::dwPlantedC4 - 0x8);

	inGame = true;
	int playerIndex = 0;
	std::vector<CPlayer> list;
	CPlayer player;
	uintptr_t list_entry, list_entry2, playerPawn, playerMoneyServices, clippingWeapon, weaponData, playerNameData;

	while (true) {
		playerIndex++;
		list_entry = process->read<uintptr_t>(entity_list + (8 * (playerIndex & 0x7FFF) >> 9) + 16);
		if (!list_entry) break;

		player.entity = process->read<uintptr_t>(list_entry + 120 * (playerIndex & 0x1FF));
		if (!player.entity) continue;

		/**
		* Skip rendering your own character and teammates
		*
		* If you really want you can exclude your own character from the check but
		* since you are in the same team as yourself it will be excluded anyway
		**/
		player.team = process->read<int>(player.entity + updater::offsets::m_iTeamNum);
		if (config::team_esp && (player.team == localTeam)) continue;

		playerPawn = process->read<std::uint32_t>(player.entity + updater::offsets::m_hPlayerPawn);

		list_entry2 = process->read<uintptr_t>(entity_list + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);
		if (!list_entry2) continue;

		player.pCSPlayerPawn = process->read<uintptr_t>(list_entry2 + 120 * (playerPawn & 0x1FF));
		if (!player.pCSPlayerPawn) continue;

		player.health = process->read<int>(player.pCSPlayerPawn + updater::offsets::m_iHealth);
		player.armor = process->read<int>(player.pCSPlayerPawn + updater::offsets::m_ArmorValue);
		if (player.health <= 0 || player.health > 100) continue;

		if (config::team_esp && (player.pCSPlayerPawn == localPlayer)) continue;

		/*
		* Unused for now, but for a vis check
		*
		* player.spottedState = process->read<uintptr_t>(player.pCSPlayerPawn + 0x1630);
		* player.is_spotted = process->read<DWORD_PTR>(player.spottedState + 0xC); // bSpottedByMask
		* player.is_spotted = process->read<bool>(player.spottedState + 0x8); // bSpotted
		*/

		playerNameData = process->read<uintptr_t>(player.entity + updater::offsets::m_sSanitizedPlayerName);
		char buffer[256];
		process->read_raw(playerNameData, buffer, sizeof(buffer));
		std::string name = std::string(buffer);
		player.name = name;

		player.gameSceneNode = process->read<uintptr_t>(player.pCSPlayerPawn + updater::offsets::m_pGameSceneNode);
		player.origin = process->read<Vector3>(player.pCSPlayerPawn + updater::offsets::m_vOldOrigin);
		player.head = { player.origin.x, player.origin.y, player.origin.z + 75.f };

		if (player.origin.x == localOrigin.x && player.origin.y == localOrigin.y && player.origin.z == localOrigin.z)
			continue;

		if (config::render_distance != -1 && (localOrigin - player.origin).length2d() > config::render_distance) continue;
		if (player.origin.x == 0 && player.origin.y == 0) continue;

		if (config::show_skeleton_esp) {
			player.gameSceneNode = process->read<uint64_t>(player.pCSPlayerPawn + 0x310);
			player.boneArray = process->read<uint64_t>(player.gameSceneNode + 0x160 + 0x80);
			player.ReadBones();
		}

		if (config::show_head_tracker && !config::show_skeleton_esp) {
			player.gameSceneNode = process->read<uint64_t>(player.pCSPlayerPawn + 0x310);
			player.boneArray = process->read<uint64_t>(player.gameSceneNode + 0x160 + 0x80);
			player.ReadHead();
		}

		if (config::show_extra_flags) {
			/*
			* Reading values for extra flags is now seperated from the other reads
			* This removes unnecessary memory reads, improving performance when not showing extra flags
			*/
			player.is_defusing = process->read<bool>(player.pCSPlayerPawn + updater::offsets::m_bIsDefusing);

			playerMoneyServices = process->read<uintptr_t>(player.entity + updater::offsets::m_pInGameMoneyServices);
			player.money = process->read<int32_t>(playerMoneyServices + updater::offsets::m_iAccount);

			player.flashAlpha = process->read<float>(player.pCSPlayerPawn + updater::offsets::m_flFlashOverlayAlpha);

			clippingWeapon = process->read<std::uint64_t>(player.pCSPlayerPawn + updater::offsets::m_pClippingWeapon);
			weaponData = process->read<std::uint64_t>(clippingWeapon + 0x360);
			weaponData = process->read<std::uint64_t>(weaponData + updater::offsets::m_szName);
			char buffer[MAX_PATH];
			process->read_raw(weaponData, buffer, sizeof(buffer));
			std::string weaponName = std::string(buffer);
			if (weaponName.compare(0, 7, "weapon_") == 0)
				player.weapon = weaponName.substr(7, weaponName.length()); // Remove weapon_ prefix
			else
				player.weapon = "Invalid Weapon Name";
		}

		list.push_back(player);
	}

	players.clear();
	players.assign(list.begin(), list.end());
}

uintptr_t boneAddress;
Vector3 bonePosition;
int boneIndex;
void CPlayer::ReadHead() {
	boneAddress = boneArray + 6 * 32;
	bonePosition = g_game.process->read<Vector3>(boneAddress);
	bones.bonePositions["head"] = g_game.world_to_screen(&bonePosition);

}

void CPlayer::ReadBones() {
	for (const auto& entry : boneMap) {
		const std::string& boneName = entry.first;
		boneIndex = entry.second;
		boneAddress = boneArray + boneIndex * 32;
		bonePosition = g_game.process->read<Vector3>(boneAddress);
		bones.bonePositions[boneName] = g_game.world_to_screen(&bonePosition);
	}
}

Vector3 CGame::world_to_screen(Vector3* v) {
	float _x = view_matrix[0][0] * v->x + view_matrix[0][1] * v->y + view_matrix[0][2] * v->z + view_matrix[0][3];
	float _y = view_matrix[1][0] * v->x + view_matrix[1][1] * v->y + view_matrix[1][2] * v->z + view_matrix[1][3];

	float w = view_matrix[3][0] * v->x + view_matrix[3][1] * v->y + view_matrix[3][2] * v->z + view_matrix[3][3];

	float inv_w = 1.f / w;
	_x *= inv_w;
	_y *= inv_w;

	float x = game_bounds.right * .5f;
	float y = game_bounds.bottom * .5f;

	x += 0.5f * _x * game_bounds.right + 0.5f;
	y -= 0.5f * _y * game_bounds.bottom + 0.5f;

	return { x, y, w };
}
