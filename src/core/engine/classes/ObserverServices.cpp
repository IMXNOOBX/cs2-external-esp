#include "ObserverServices.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"

bool ObserverServices::Update() {
	auto p = Engine::GetProcess();

	if (!p)
		return false;

	auto client = Engine::GetClient();

	if (!this->address) return false;

	this->mode = p->read<ObserverMode>(this->address + offsets::observerServices::m_iObserverMode);
	this->target = p->read<int>(this->address + offsets::observerServices::m_hObserverTarget);

	return true;
}

void ObserverServices::SetAddress(DWORD64 address) {
	this->address = address;
}

const char* ObserverServices::ToString() {
	switch (this->mode)
	{
	case ObserverMode::Alive:   return "Self";
	case ObserverMode::Unknown:   return "1";
	case ObserverMode::First: return "First person";
	case ObserverMode::Third: return "Third person";
	case ObserverMode::Free: return "Free Roam";
	default:      return "Unknown";
	}
}