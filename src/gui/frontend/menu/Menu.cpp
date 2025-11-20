#include "Menu.hpp"

#include "core/engine/cache/Cache.hpp"
#include "gui/renderer/Renderer.hpp" // Circular dependency
#include "gui/renderer/window/Window.hpp" // Circular dependency


bool Menu::Init() {
    return GetInstance().InitImpl();
}

void Menu::Render() {
    return GetInstance().RenderImpl();
}

void Menu::RenderStartupHelp() {
	return GetInstance().RenderStartupHelpImpl();
}

ImVec2 Menu::GetPos() {
	return GetInstance().pos;
}

ImVec2 Menu::GetSize() {
	return GetInstance().size;
}

bool Menu::InitImpl() {
	SetupStyles();

    LOGF(INFO, "Successfully initialized menu...");
    return true;
}

void Menu::RenderImpl() {
	if (!isSetup)
		return;

	static auto io = ImGui::GetIO();
	static auto screen = io.DisplaySize;
	static auto color_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None;

#ifdef _DEBUG
	static auto title = "github.com/IMXNOOBX/cs2-external-esp (recode) [DEV]";
#else
	static auto title = "cs2-external-esp | recode (PTB)";
#endif

	ImGui::SetNextWindowSize(ImVec2(600, 350), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(screen.x/2 - 300, screen.y/2 - 150), ImGuiCond_FirstUseEver);

	ImGui::GetWindowPos();
	if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
		this->pos = ImGui::GetWindowPos();
		this->size = ImGui::GetWindowSize();

		ImGui::Checkbox("Enable", &cfg::enabled);
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(340, 0));
		ImGui::SameLine();
		ImGui::TextLinkOpenURL("Source/Support", "https://github.com/IMXNOOBX/cs2-external-esp");
		ImGui::SameLine();
		ImGui::TextLinkOpenURL("Discord", "https://discord.gg/pRew8ZDkyp");

		static int y_space_left;
		auto space = ImGui::GetContentRegionAvail();

		ImGui::BeginDisabled(!cfg::enabled);
		if (ImGui::BeginChild("##top", ImVec2(0, space.y / 2 + y_space_left)))
		{
			if (ImGui::BeginChild("##leftside", ImVec2(space.x / 2 - 10, 0)))
			{
				ImGui::Text("Visuals");
				ImGui::Separator();

				ImGui::BeginGroup();
				{
					ImGui::Checkbox("Box", &cfg::esp::box);
					ImGui::BeginDisabled(!cfg::esp::box);
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("Team box color", cfg::esp::colors::box_team.data(), color_flags);
						ImGui::SetItemTooltip("Team box color");

						ImGui::SameLine();
						ImGui::ColorEdit4("Enemy box color", cfg::esp::colors::box_enemy.data(), color_flags);
						ImGui::SetItemTooltip("Enemy box color");
					}
					ImGui::EndDisabled();
					
					ImGui::Checkbox("Skeleton", &cfg::esp::skeleton);
					ImGui::BeginDisabled(!cfg::esp::skeleton);
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("Team skeleton color", cfg::esp::colors::skeleton_team.data(), color_flags);
						ImGui::SetItemTooltip("Team skeleton color");

						ImGui::SameLine();
						ImGui::ColorEdit4("Enemy skeleton color", cfg::esp::colors::skeleton_enemy.data(), color_flags);
						ImGui::SetItemTooltip("Enemy skeleton color");
					}
					ImGui::EndDisabled();

					ImGui::Checkbox("Head Tracker", &cfg::esp::head_tracker);
					ImGui::BeginDisabled(!cfg::esp::head_tracker);
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("Team head tracker color", cfg::esp::colors::tracker_team.data(), color_flags);
						ImGui::SetItemTooltip("Team head tracker color");

						ImGui::SameLine();
						ImGui::ColorEdit4("Enemy head tracker color", cfg::esp::colors::tracker_enemy.data(), color_flags);
						ImGui::SetItemTooltip("Enemy head tracker color");
					}
					ImGui::EndDisabled();

					ImGui::Checkbox("Show Team", &cfg::esp::team);
				}
				ImGui::EndGroup();

				ImGui::SameLine();

				ImGui::BeginGroup();
				{
					ImGui::Checkbox("Health", &cfg::esp::health);
					ImGui::Checkbox("Armor", &cfg::esp::armor);
					ImGui::Checkbox("Spotted", &cfg::esp::spotted);
					ImGui::SetItemTooltip("Esp will only be visible if the player has been spotted by you");
				}
				ImGui::EndGroup();
			}
			ImGui::EndChild();

			ImGui::SameLine();

			if (ImGui::BeginChild("##rightside")) 
			{
				ImGui::Text("Flags");
				ImGui::Separator();

				ImGui::BeginGroup();
				{
					ImGui::Checkbox("Name", &cfg::esp::flags::name);
					ImGui::Checkbox("Weapon", &cfg::esp::flags::weapon);
					ImGui::Checkbox("Defusing", &cfg::esp::flags::defusing);
				}
				ImGui::EndGroup();
				
				ImGui::SameLine();

				ImGui::BeginGroup();
				{
					ImGui::Checkbox("Money", &cfg::esp::flags::money);
					ImGui::Checkbox("Flashed", &cfg::esp::flags::flashed);
					ImGui::Checkbox("Ping", &cfg::esp::flags::ping);
				}
				ImGui::EndGroup();
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
		ImGui::EndDisabled();

		if (ImGui::BeginChild("##bottom"))
		{
			ImGui::Text("Settings");
			ImGui::Separator();

			if (ImGui::Checkbox("Streamproof", &cfg::settings::streamproof)) {
				Window::SetAffinity(
					Window::hwnd, 
					cfg::settings::streamproof ? WindowAffinity::Invisible : WindowAffinity::Disabled
				);
			}

			ImGui::Checkbox("Watermark", &cfg::settings::watermark);

			if (ImGui::Checkbox("VSync", &cfg::settings::vsync))
				Window::vsync = cfg::settings::vsync;
			ImGui::SetItemTooltip("VSync, matches render speed with screen refresh rate, improving performance");

#ifdef _DEBUG
			ImGui::Text("Dev");
			ImGui::Separator();

			if (ImGui::Checkbox("Console", &cfg::dev::console)) 
				if (!cfg::dev::console) LogHelper::Free();
			
			static int key_out;
			if (ImGui::Button("Open Menu Key")) {
				for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++) {
					if (ImGui::IsKeyPressed((ImGuiKey)i)) {
						key_out = i;
						LOGF(VERBOSE, "Changed the open menu key to {}", key_out);
						break;
					}
				}
			}

			ImGui::SliderInt("Cache Refresh Rate", &cfg::dev::cache_refresh_rate, 0, 100, "%dms");
#endif
		}
		ImGui::EndChild();

		y_space_left = ImGui::GetContentRegionAvail().y;
	}

	ImGui::End();
}

void Menu::SetupStyles() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

	style.Colors[ImGuiCol_WindowBg] = ImColor(10, 10, 10);
	style.Colors[ImGuiCol_ChildBg] = ImColor(10, 10, 10);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImColor(50, 50, 50);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	style.Colors[ImGuiCol_FrameBg] = ImColor(75, 75, 75);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);

	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);

	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);

	style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.29f, 0.30f, 0.31f, 0.95f);

	style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	style.FrameBorderSize = 1.0f;

	// Window & Frame
	style.WindowRounding = 12.f;
	style.ChildRounding = 10.f;

	style.FrameRounding = 5.f;
	style.PopupRounding = 5.f;

	style.GrabRounding = 3.f;

    auto& io = ImGui::GetIO();

    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f);
}

void Menu::RenderStartupHelpImpl() {
	static bool has_opened_menu = false;

	if (has_opened_menu)
		return;

	auto& io = ImGui::GetIO();
	auto screen = io.DisplaySize;
	auto d = ImGui::GetBackgroundDrawList();

	if (Renderer::IsOpen())
		has_opened_menu = true;

	auto help = "To OPEN the menu, Use Insert or Right Shift keys"
		"\n\t\t\t\tTo CLOSE, press the End key";
	auto size = ImGui::CalcTextSize(help);

	d->AddText(
		ImVec2(screen.x / 2 - size.x / 2, 80),
		IM_COL32(255, 255, 255, 255),
		help
	);
}