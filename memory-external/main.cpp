#include <iostream>
#include <thread>
#include <chrono>

#include "memory/memory.hpp"
#include "classes/vector.hpp"
#include "classes/OverlayCord.h"

// https://www.unknowncheats.me/forum/3846642-post734.html

#define dwLocalPlayer 0x1713348
#define dwEntityList 0x1679C38
#define dwPawnHealth 0x808
#define dwPlayerPawn 0x5dc
#define dwViewMatrix 0x1713CF0
#define m_iTeamNum 0x3bf
#define m_vecOrigin 0x12AC

int main() {
	std::shared_ptr<pProcess> process = std::make_shared<pProcess>();

	std::cout << "[cs2] Waiting for cs2.exe..." << std::endl;

	while (!process->AttachProcess("cs2.exe"))
		std::this_thread::sleep_for(std::chrono::seconds(1));

	std::cout << "[cs2] Attached to cs2.exe" << std::endl;

	ProcessModule base_module;

	do {
		base_module = process->GetModule("client.dll");
		if (base_module.base == 0) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cout << "[cs2] Failed to find module client.dll" << std::endl;
		}
	} while (base_module.base == 0);

	std::cout << "[overlay] Connecting to the process..." << std::endl;
	OverlayCord::Communication::ConnectedProcessInfo processInfo = { 0 };
	processInfo.ProcessId = process->pid_;

	bool status = false;
	do {
		status = OverlayCord::Communication::ConnectToProcess(processInfo);
		if (!status) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cout << "[overlay] Failed to connect to the process overlay backend" << std::endl;
		}
	} while (!status);

	std::cout << "[overlay] Connected to the process with mapped address 0x" << processInfo.MappedAddress << std::endl;

	OverlayCord::Drawing::Frame mainFrame = OverlayCord::Drawing::CreateFrame(screen_x, screen_y);

	OverlayCord::Drawing::Pixel enemy_color = { 75, 75, 175, 255 }; // BYTE B, G, R, A;
	OverlayCord::Drawing::Pixel team_color = { 75, 175, 75, 255 };

	std::cout << "[cs2] Fully set up, running main loop now" << std::endl;

	while (!(GetAsyncKeyState(VK_END) & 0x8000)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		uintptr_t localPlayer = process->read<uintptr_t>(base_module.base + dwLocalPlayer);

		if (!localPlayer) continue;

		int localTeam = process->read<int>(localPlayer + m_iTeamNum);

		view_matrix_t view_matrix = process->read<view_matrix_t>(base_module.base + dwViewMatrix);

		Vector3 localOrigin = process->read<Vector3>(localPlayer + m_vecOrigin);

		OverlayCord::Drawing::CleanFrame(mainFrame);

		uintptr_t entity_list = process->read<uintptr_t>(base_module.base + dwEntityList);

		for (int i = 1; i < 32; i++) {
			uintptr_t list_entry = process->read<uintptr_t>(entity_list + (8 * (i & 0x7FFF) >> 9) + 16);
			if (!list_entry) continue;
			uintptr_t player = process->read<uintptr_t>(list_entry + 120 * (i & 0x1FF));

			if (!player) continue;

			int playerHealth = process->read<int>(player + dwPawnHealth);
			if (playerHealth < 0 || playerHealth > 100) continue;

			int playerTeam = process->read<int>(player + m_iTeamNum);

			// https://github.com/UnnamedZ03/CS2-external-base/blob/main/source/CSSPlayer.hpp#L132
			std::uint32_t playerpawn = process->read<std::uint32_t>(player + dwPlayerPawn);

			uintptr_t list_entry2 = process->read<uintptr_t>(entity_list + 0x8 * ((playerpawn & 0x7FFF) >> 9) + 16);
			if (!list_entry2) continue;
			uintptr_t pCSPlayerPawn = process->read<uintptr_t>(list_entry2 + 120 * (playerpawn & 0x1FF));

			if (pCSPlayerPawn == localPlayer) continue;

			// https://github.com/UnnamedZ03/CS2-external-base/blob/main/source/CSSPlayer.hpp#L132
			Vector3 origin = process->read<Vector3>(pCSPlayerPawn + m_vecOrigin);

			//if ((localOrigin - origin).length2d() > 1000) continue;

			Vector3 head;
			head.x = origin.x;
			head.y = origin.y;
			head.z = origin.z + 75.f;

			Vector3 screenpos = origin.world_to_screen(view_matrix);
			Vector3 screenhead = head.world_to_screen(view_matrix);

			float height = screenpos.y - screenhead.y;
			float width = height / 2.4f;
			if (screenpos.z >= 0.01f) {
				OverlayCord::Drawing::Pixel health_color = { 75,
					(unsigned char)(55 + playerHealth * 2),
					(unsigned char)(255 - playerHealth),
					255
				};

				OverlayCord::Drawing::DrawRectangle(
					mainFrame,
					screenhead.x - 25.f,
					screenhead.y,
					width,
					height,
					(localTeam == playerTeam ? team_color : enemy_color)
				);

				OverlayCord::Drawing::DrawRectangle(
					mainFrame,
					screenhead.x - 30.f,
					screenhead.y + (height * (100 - playerHealth) / 100),
					2,
					height - (height * (100 - playerHealth) / 100),
					health_color
				);
			}
		}

		for (int i = 0; i < 10; i++)
			OverlayCord::Drawing::DrawCircle(mainFrame, 15, 15, i, enemy_color);

		OverlayCord::Communication::SendFrame(processInfo, mainFrame.Width, mainFrame.Height, mainFrame.Buffer, mainFrame.Size);
	}

	std::cout << "[overlay] Cleaning frame" << std::endl;
	OverlayCord::Drawing::CleanFrame(mainFrame);
	std::cout << "[overlay] Disconnecting from process" << std::endl;
	OverlayCord::Communication::DisconnectFromProcess(processInfo);

	std::cout << "[cs2] Deataching from process" << std::endl;
	process->Close();

	std::cout << "[cs2] Press any key to close" << std::endl;
	std::cin.get();

	return 1;
}
