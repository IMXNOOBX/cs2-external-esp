#include "Bomb.hpp"

#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"

bool Bomb::Update() {

	auto p = Engine::GetProcess();

	if (!p)
		return false;

	auto client = Engine::GetClient();


	this->is_planted = p->read<uintptr_t>(client.base + offsets::plantedC4 - offsets::bomb::m_isPlanted);

	if (!this->is_planted)
		return true;

	this->address = p->read<uintptr_t>(client.base + offsets::plantedC4);
	this->address = p->read<uintptr_t>(this->address);

	if (!this->address)
		return false;

	auto site = p->read<uint32_t>(this->address + offsets::bomb::m_nBombSite);
	this->site = (site == 1) ? BombSite::B : BombSite::A;

	return true;
}
