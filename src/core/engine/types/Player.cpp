#include "Player.hpp"

#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"

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

	this->controller = p->read<DWORD64>(le + (index + 1) * 0x70); // before was 0x78

	return this->controller != 0;
}

bool Player::GetPawn() {
	auto p = Engine::GetProcess();
	auto client = Engine::GetClient();

	DWORD64 entity_pawn_list_entry = 0;
	DWORD64 entity_pawn_address = 0;

	entity_pawn_address = p->read<DWORD64>(controller + offsets::controller::m_hPawn);

	if (!pawn) 
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

	if (!p->read_raw(controller + offsets::controller::m_iszPlayerName, this->name, sizeof(this->name)))
		return false;

	return true;
}

bool Player::UpdatePawn() {
	auto p = Engine::GetProcess();

	this->health = p->read<int>(pawn + offsets::pawn::m_iHealth);
	this->alive = health != 0;

	if (!alive) // No need to continue 
		return true;

	this->pos = p->read<Vec3_t>(pawn + offsets::pawn::m_vOldOrigin);

	if (this->pos.zero())
		return false;

	this->team = p->read<uint8_t>(pawn + offsets::pawn::m_iTeamNum);

	return true;
}