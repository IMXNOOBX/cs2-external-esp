namespace offsets
{
	// client.dll
	inline DWORD entityList;
	inline DWORD viewMatrix;
	inline DWORD localPlayerController;
	inline DWORD globalVars;
	inline DWORD weaponC4;

	//inline DWORD viewAngles;
	//inline DWORD localPlayerPawn;
	//inline DWORD csgoInput;

	// engine2.dll
	inline DWORD buildNumber;

	namespace controller {
		constexpr std::ptrdiff_t m_hPawn = 0x6B4; // CHandle<C_BasePlayerPawn>
		constexpr std::ptrdiff_t m_steamID = 0x770; // uint64
		constexpr std::ptrdiff_t m_iszPlayerName = 0x6E8; // char[128]

		//constexpr std::ptrdiff_t health = 0x32C;
		//constexpr std::ptrdiff_t teamId = 0x3BF;
		//constexpr std::ptrdiff_t isAlive = 0x7DC;
		//constexpr std::ptrdiff_t playerPawn = 0x5F4;
		//constexpr std::ptrdiff_t iszPlayerName = 0x628;
	}

	namespace pawn {
		constexpr std::ptrdiff_t m_vOldOrigin = 0x15A0; // Vector
		constexpr std::ptrdiff_t m_iHealth = 0x34C; // int32
		constexpr std::ptrdiff_t m_iTeamNum = 0x3EB; // uint8

#if 0
		constexpr std::ptrdiff_t maxHealth = 0x328;
		constexpr std::ptrdiff_t currentHealth = 0x32C;
		constexpr std::ptrdiff_t gameSceneNode = 0x310;
		constexpr std::ptrdiff_t boneArray = 0x1E0;
		constexpr std::ptrdiff_t angEyeAngles = 0x1510;
		constexpr std::ptrdiff_t vecLastClipCameraPos = 0x128C;
		constexpr std::ptrdiff_t pClippingWeapon = 0x12A8;
		constexpr std::ptrdiff_t iShotsFired = 0x1418;
		constexpr std::ptrdiff_t flFlashDuration = 0x1468;
		constexpr std::ptrdiff_t aimPunchAngle = 0x1714;
		constexpr std::ptrdiff_t aimPunchCache = 0x1738;
		constexpr std::ptrdiff_t iIdEntIndex = 0x153C;
		constexpr std::ptrdiff_t iTeamNum = 0x3BF;
		constexpr std::ptrdiff_t cameraServices = 0x10E0;
		constexpr std::ptrdiff_t iFovStart = 0x214;
		constexpr std::ptrdiff_t fFlags = 0x3C8;
		constexpr std::ptrdiff_t bSpottedByMask = 0x1630 + 0xC; // entitySpottedState + bSpottedByMask
#endif
	}

	namespace global {
		constexpr std::ptrdiff_t maxClients = 0x10;
		constexpr std::ptrdiff_t currentMapName = 0x180;
		constexpr std::ptrdiff_t currentTime = 0x2C;
	}

	namespace signatures
	{
		const std::string viewMatrix = "48 8D 0D ?? ?? ?? ?? 48 C1 E0 06";
		const std::string globalVars = "48 89 15 ?? ?? ?? ?? 48 89 42";
		const std::string entityList = "48 8B 0D ?? ?? ?? ?? 48 89 7C 24 ?? 8B FA C1 EB";
		const std::string localPlayerController = "48 8B 05 ?? ?? ?? ?? 41 89 BE";
		const std::string weaponC4 =
			"48 89 05 ?? ?? ?? ?? "
			"F7 C1 ?? ?? ?? ?? "
			"74 ?? "
			"81 E1 ?? ?? ?? ?? "
			"89 0D ?? ?? ?? ?? "
			"8B 05 ?? ?? ?? ?? "
			"89 1D ?? ?? ?? ?? "
			"EB ?? "
			"48 8B 15 ?? ?? ?? ?? "
			"48 8B 5C 24 ?? "
			"FF C0 "
			"89 05 ?? ?? ?? ?? "
			"48 8B C6 48 89 34 EA 80 BE";

	#if 0
		const std::string localPlayerPawn = "48 8D 05 ?? ?? ?? ?? C3 CC CC CC CC CC CC CC CC 48 83 EC ?? 8B 0D";

		const std::string csgoInput = "48 89 05 ?? ?? ?? ?? 0F 57 C0 0F 11 05";
		const std::string viewAngles = "F2 42 0F 10 84 28 ?? ?? ?? ??"; //"48 8B 0D ?? ?? ?? ?? 48 8B 01 48 FF 60 30";
	#endif

		const std::string buildNumber = "89 05 ?? ?? ?? ?? 48 8d 0d ?? ?? ?? ?? ff 15 ?? ?? ?? ?? 48 8b 0d"; //"89 05 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? FF 15 ?? ?? ?? ?? 48 8B 0D";
		
	}
}
