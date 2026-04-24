#pragma once
class Weapon
{
public:
	Weapon(int index) : index(index) {}
	bool Update();

public:
	std::string weapon_name;
	
private:
	const char* ToString() const;

	short item_index;
	int index;
};

