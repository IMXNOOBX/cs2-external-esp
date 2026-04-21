#pragma once
class Weapon
{
public:
	Weapon(int index, uintptr_t le) : index(index), le(le) {}
	bool Update();

public:
	std::string weapon_name;
	
private:
	const char* ToString() const;

	short item_index;
	int index;
	uintptr_t le;
};

