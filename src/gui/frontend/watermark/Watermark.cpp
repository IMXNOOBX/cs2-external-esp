#include "Watermark.hpp"

bool Watermark::Init() {
	return GetInstance().InitImpl();
}

void Watermark::Render() {
    return GetInstance().RenderImpl();
}

bool Watermark::InitImpl() {
	auto& io = ImGui::GetIO();

	ImFontConfig cfg{};
	cfg.FontDataOwnedByAtlas = false;

	this->font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 14.0f, &cfg);

	return true;
}

void Watermark::RenderImpl() {
	if (!cfg::settings::watermark)
		return;

	ImGui::PushFont(this->font);

	auto& io = ImGui::GetIO();
	auto d = ImGui::GetBackgroundDrawList();

	auto globals = Cache::CopyGlobals();

	static int margin = 10;
	static int padding = 10;
	std::string watermark_string = "cs2-external-esp";

	watermark_string += std::format(" | {}fps", (int)io.Framerate);
	watermark_string += std::format(" | {}", globals.map_name);

	auto size = ImGui::CalcTextSize(watermark_string.data());

	auto pos = ImVec2(io.DisplaySize.x - margin - padding - size.x, margin + padding / 2);
	auto rect_start = ImVec2(io.DisplaySize.x - margin - padding * 2 - size.x, margin);
	auto rect_end = ImVec2(io.DisplaySize.x - margin, margin + size.y + padding);

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
		pos,
		IM_COL32(255, 255, 255, 255),
		watermark_string.data()
	);

	ImGui::PopFont();
}