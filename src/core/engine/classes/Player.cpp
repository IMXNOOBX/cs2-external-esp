#include "Player.hpp"

#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"
#include "Weapon.hpp"

bool Player::Update() {
	if (!Engine::GetProcess())
		return false;

	if (!GetController()) {
		//LOGF(WARNING, "Failed to GET controller for entity index({})", index);
		return false;
	}

	if (!GetPawn()) {
		//LOGF(WARNING, "Failed to GET pawn for entity index({})", index);
		return false;
	}	

	if (!UpdateController()) {
		//LOGF(WARNING, "Failed to UPDATE controller for entity index({})", index);
		return false;
	}

	if (!UpdatePawn()) {
		//LOGF(WARNING, "Failed to UPDATE pawn for entity index({})", index);
		return false;
	}

	return true;
}

bool Player::GetController() {
	auto p = Engine::GetProcess();
	auto client = Engine::GetClient();

	this->controller = p->read<DWORD64>(list_entry + (index + 1) * 0x70); // before was 0x78

	return this->controller != 0;
}

bool Player::GetPawn() {
	auto p = Engine::GetProcess();
	auto client = Engine::GetClient();

	DWORD64 entity_pawn_list_entry = 0;
	DWORD64 entity_pawn_address = 0;

	entity_pawn_address = p->read<DWORD64>(controller + offsets::controller::m_hPawn);

	if (!entity_pawn_address)
		return false;

	entity_pawn_list_entry = p->read<DWORD64>(client.base + offsets::entityList);

	if (!entity_pawn_list_entry) 
		return false;

	entity_pawn_list_entry = p->read<uintptr_t>(entity_pawn_list_entry + 0x10 + 0x8 * ((entity_pawn_address & 0x7FFF) >> 9));

	if (!entity_pawn_list_entry)
		return false;

	this->pawn = p->read<uintptr_t>(entity_pawn_list_entry + 0x70 * (entity_pawn_address & 0x1FF)); /*0x78*/

	return this->pawn != 0;
}

bool Player::UpdateController() {
	auto p = Engine::GetProcess();

	this->steam_id = p->read<uint64_t>(controller + offsets::controller::m_steamID);
	this->bot = this->steam_id == 0;

	// expensive
	if (!p->read_raw(controller + offsets::controller::m_iszPlayerName, this->name, sizeof(this->name)))
		return false;

	this->localplayer = p->read<bool>(controller + offsets::controller::m_bIsLocalPlayerController);
	this->ping = p->read<int>(controller + offsets::controller::m_iPing);

	auto money_services = p->read<uintptr_t>(controller + offsets::controller::m_pInGameMoneyServices);

	if (money_services)
		this->money = p->read<int>(money_services + offsets::controller::m_iAccount);

	return true;
}

bool Player::UpdatePawn() {
	auto p = Engine::GetProcess();

	this->health = p->read<int>(pawn + offsets::pawn::m_iHealth);
	this->alive = health != 0;

	if (this->health > 255 || this->health < 0)
		LOGF(FATAL, 
			"Health seems to have a random value (over 100 or under 0) with a value of ({}). Game has probably updated pawn structure", 
			this->health
		);

	if (!alive) // No need to continue 
		return true;

	this->pos = p->read<Vec3_t>(pawn + offsets::pawn::m_vOldOrigin);

	if (this->pos.zero())
		return false;

	this->team = p->read<uint8_t>(pawn + offsets::pawn::m_iTeamNum);

	this->armor = p->read<int>(pawn + offsets::pawn::m_ArmorValue);
	this->defusing = p->read<bool>(pawn + offsets::pawn::m_bIsDefusing);
	this->spotted = p->read<bool>(pawn + offsets::pawn::m_entitySpottedState + offsets::pawn::m_bSpottedByMask);
	this->flashed = p->read<float>(pawn + offsets::pawn::m_flFlashOverlayAlpha) > 0;
	this->scoped = p->read<bool>(pawn + offsets::pawn::m_bIsScoped);

	if (!UpdateSkeleton()) {
		LOGF(FATAL, "Failed to update skeleton");
		return false;
	}

	// Shows errors when player just respawned
	if (!UpdateWeapon()) {
		//LOGF(FATAL, "Failed to update weapon"); // too verbose
		return false;
	}

	return true;
}

bool Player::UpdateSkeleton() {
	auto p = Engine::GetProcess();

	auto game_scene = p->read<DWORD64>(this->pawn + offsets::pawn::m_pGameSceneNode);

	if (!game_scene)
		return false;

	auto bone_array = p->read<DWORD64>(game_scene + (offsets::bone::m_modelState + 0x80));

	if (!bone_array)
		return false;

	if (!p->read_raw(bone_array, bones, sizeof(bones)))
		return false;

	for (int i = 0; i < 30; i++) 
		this->bone_list.push_back({ bones[i].pos });

	return true;
}

bool Player::UpdateWeapon() {
	auto p = Engine::GetProcess();

	auto weapon_services = p->read<uintptr_t>(this->pawn + offsets::pawn::m_pWeaponServices);

	if (!weapon_services)
		return false;

	auto active_weapon_index = p->read<int>(weapon_services + offsets::pawn::m_hActiveWeapon);

	if (!active_weapon_index)
		return false;

	auto weapon = Weapon(entity_list, active_weapon_index);

	if (!weapon.Update())
		return false;

	this->weapon = weapon;

	//this->clean_weapon = weapon.weapon_name;

	return true;
}

bool Player::GetBounds(view_matrix_t matrix, Vec2_t size, std::pair<Vec2_t, Vec2_t>& bounds) {
	Vec2_t origin;
	bool pt1 = matrix.wts(this->pos, size, origin);


	Vec3_t pos_top;
	if (this->bone_list.empty())
		pos_top = this->pos + Vec3_t(0, 0, 65.f); // 75.f
	else
		pos_top = this->bone_list[bone_index::head].pos;

	//auto head_bone = this->bone_list[bone_index::head];
	//head_bone.pos.z *= 1.09; // little offset to cover the entire head
	//bone_pos head_bone = origin + ImVec3

	Vec2_t top;
	bool pt2 = matrix.wts(pos_top, size, top);

	float height = origin.y - top.y;
	float width = height / 2.4f;

	top.x -= width / 2;
	origin.x += width / 2;

	top.y -= width / 4;

	// Top to bottom
	bounds = { top, origin };

	return pt1 || pt2;
}