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

	if (!p->read_raw(controller + offsets::controller::m_iszPlayerName, this->name, sizeof(this->name)))
		return false;

	this->localplayer = p->read<bool>(controller + offsets::controller::m_bIsLocalPlayerController);


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

bool Player::GetBounds(view_matrix_t matrix, Vec2_t size, std::pair<Vec2_t, Vec2_t>& bounds) {
	Vec2_t origin;
	bool pt1 = matrix.wts(this->pos, size, origin);

	Vec2_t head; // Use head bone whenever its implemented
	bool pt2 = matrix.wts(this->pos + Vec3_t(0, 0, 75.f), size, head);

	float height = origin.y - head.y;
	float width = height / 2.4f;

	head.x -= width / 2;
	origin.x += width / 2;

	bounds = { origin, head };

	return pt1/* || pt2*/;
}
