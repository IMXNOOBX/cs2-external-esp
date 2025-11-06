#include "Globals.hpp"

#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"

int Globals::max_clients = 0;
long Globals::current_time = 0;
char Globals::map_name[32] = "";
bool Globals::in_match = false;
uintptr_t Globals::address = 0;

bool Globals::Update() {
	auto p = Engine::GetProcess();
	auto client = Engine::GetClient();

	if (!p) 
		return false;

	address = p->read<uintptr_t>(client.base + offsets::globalVars);

	if (!address)
		return false;

	current_time = p->read<long>(address + offsets::global::currentTime);

	max_clients = p->read<int>(address + offsets::global::maxClients);
	in_match = max_clients <= 1; // In the lobby, it seems that it still counts one

	auto map_name_addr = p->read<DWORD64>(address + offsets::global::currentMapName);

	if (!p->read_raw(map_name_addr, map_name, sizeof(map_name)))
		return false;

	return true;
}
