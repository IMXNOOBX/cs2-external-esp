#pragma once

enum class ObserverMode : uint8_t {
	Alive,
	Unknown,
	First,
	Third,
	Free,
};

class ObserverServices {

public:
	ObserverServices() = default;
	ObserverServices(DWORD64 addr) { address = addr; };

	bool Update();
	void SetAddress(DWORD64 address);
	const char* ToString() const;
public:
	int target = 0;
	ObserverMode mode = ObserverMode::Alive;
	
private:
	DWORD64 address = 0;
};