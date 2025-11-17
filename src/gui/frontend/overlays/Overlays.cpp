#include "Overlays.hpp"

#include "updater/Updater.hpp"
#include "gui/renderer/Renderer.hpp" // Circular dependency
#include "gui/frontend/menu/Menu.hpp" // Circular dependency

bool Overlays::Init() {
	return GetInstance().InitImpl();
}

void Overlays::Render() {
    return GetInstance().RenderImpl();
}

bool Overlays::InitImpl() {
	auto& io = ImGui::GetIO();

	ImFontConfig cfg{};
	cfg.FontDataOwnedByAtlas = false;

	//this->font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 14.0f, &cfg);
	this->font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 12.0f, &cfg);

	return true;
}

void Overlays::RenderImpl() {
	ImGui::PushFont(this->font);

	RenderWatermark();

	RenderNotice();

#ifdef _DEBUG
	RenderDebugWindow();
#endif

	ImGui::PopFont();
}

void Overlays::RenderWatermark() {
	if (!cfg::settings::watermark)
		return;

	auto& io = ImGui::GetIO();
	auto d = ImGui::GetBackgroundDrawList();

	auto globals = Cache::CopyGlobals();

	static int margin = 10;
	static int padding = 10;
	std::string watermark_string = "cs2-external-esp (PTB)";

	watermark_string += std::format(" | {}fps", (int)io.Framerate);

	if (globals.in_match)
		watermark_string += std::format(" | {}", globals.map_name);

	auto size = ImGui::CalcTextSize(watermark_string.data());

	auto rect_start = ImVec2(io.DisplaySize.x - margin - padding * 2 - size.x, margin);
	auto rect_end = ImVec2(io.DisplaySize.x - margin, margin + size.y + padding);
	auto pos = ImVec2(rect_start.x + padding, rect_start.y + padding * 0.6/* compensate font */);

	d->AddRectFilled(
		rect_start,
		rect_end,
		IM_COL32(0, 0, 0, 200),
		8.f
	);

	d->AddRect(
		rect_start,
		rect_end,
		IM_COL32(100, 100, 100, 200),
		8.f
	);

	d->AddText(
		pos,
		IM_COL32(255, 255, 255, 255),
		watermark_string.data()
	);
}

void Overlays::RenderNotice() {
	static auto status = Updater::GetStatus();

	if (status.notice.empty())
		return;

	if (!Renderer::IsOpen())
		return;

	auto& io = ImGui::GetIO();
	auto d = ImGui::GetBackgroundDrawList();

	static int margin = 10;
	static int padding = 10;
	auto menu_pos = Menu::GetPos();
	auto menu_size = Menu::GetSize();

	auto max_width = menu_size.x - padding * 2;
	
	auto size = ImGui::CalcTextSize(status.notice.data(), nullptr, false, max_width);

	auto rect_start = ImVec2(menu_pos.x, menu_pos.y - margin - padding * 2 - size.y);
	auto rect_end = ImVec2(menu_pos.x + menu_size.x, menu_pos.y - margin);
	auto pos = ImVec2(rect_start.x + padding, rect_start.y + padding);

	d->AddRectFilled(
		rect_start,
		rect_end,
		IM_COL32(0, 0, 0, 200),
		10.f
	);

	d->AddRect(
		rect_start,
		rect_end,
		IM_COL32(100, 100, 100, 200),
		10.f
	);

	d->AddText(
		pos - ImVec2(0, padding + padding * 0.5),
		IM_COL32(255, 200, 0, 255),
		"Notice"
	);

	d->AddText(
		this->font,
		this->font->LegacySize,
		pos,
		IM_COL32(255, 255, 255, 255),
		status.notice.data(),
		nullptr, 
		max_width
	);
}

void Overlays::RenderDebugWindow() {
#ifdef _DEBUG
	auto& io = ImGui::GetIO();
	auto d = ImGui::GetBackgroundDrawList();

	auto globals = Cache::CopyGlobals();
	auto players = Cache::CopyPlayers();

	static int margin = 10;
	static int padding = 10;
	std::string debug_string = "> Game Debug Window\n";

	debug_string += std::format("Map: {}\n", globals.map_name);
	debug_string += std::format("Max Clients: {}\n", globals.max_clients);
	debug_string += std::format("Cache Refresh: {}ms\n", cfg::dev::cache_refresh_rate);

	if (!players.empty())
		debug_string += std::format("Players ({}):\n", players.size());

	for (auto& player : players)
		debug_string += std::format("- {} ({}) {}hp {}ms\n", player.name, player.steam_id, player.health, player.ping);

	auto size = ImGui::CalcTextSize(debug_string.data());

	d->AddRectFilled(
		ImVec2(10 + margin - padding, io.DisplaySize.y - size.y - 20 - margin - padding),
		ImVec2(10 + size.x + margin + padding, io.DisplaySize.y - 20 - margin + padding),
		IM_COL32(0, 0, 0, 200),
		10.f
	);

	d->AddRect(
		ImVec2(10 + margin - padding, io.DisplaySize.y - size.y - 20 - margin - padding),
		ImVec2(10 + size.x + margin + padding, io.DisplaySize.y - 20 - margin + padding),
		IM_COL32(100, 100, 100, 200),
		10.f
	);

	d->AddText(
		ImVec2(10 + margin, io.DisplaySize.y - size.y - 20 - margin),
		IM_COL32(255, 255, 255, 255),
		debug_string.data()
	);
#endif
}