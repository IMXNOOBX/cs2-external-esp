#pragma once
class Weapon
{
public:
	Weapon(int index) : index(index) {}
	bool Update();

public:
	std::string weapon_name;
	int32_t ammo;
	bool is_reloading;

private:
	const char* ToString() const;

	short item_index;
	int index;
};

