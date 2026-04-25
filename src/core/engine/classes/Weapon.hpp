#pragma once
#include "core/engine/types/Weapons.hpp"

class Weapon
{
public:
	Weapon(uintptr_t entity_list, int slot_index)
		: entity_list(entity_list), slot_index(slot_index) {}
	Weapon() 
		: item_index(-1), name("Invalid"), slot_index(0), entity_list(0){}

	bool Update();
public:
	short item_index;
	std::string name;
	int32_t ammo;
	
private:
	const char* ToString() const;

	int slot_index;
	uintptr_t entity_list;
};

