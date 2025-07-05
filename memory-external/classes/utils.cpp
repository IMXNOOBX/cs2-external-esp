#include "utils.h"

void Utils::update_console_title() {
	std::string title = "cs2-external-esp";

    config::show_box_esp ? title += " | Box: [O]" : title += " | Box: [X]";
    config::show_skeleton_esp ? title += " | Skeleton: [O]" : title += " | Skeleton: [X]";
    config::show_head_tracker ? title += " | Head Tracker: [O]" : title += " | Head Tracker: [X]";
    config::team_esp ? title += " | Team: [O]" : title += " | Team: [X]";
    config::automatic_update ? title += " | Auto Update: [O]" : title += " | Auto Update: [X]";
    config::show_extra_flags ? title += " | Flags: [O]" : title += " | Flags: [X]";

	SetConsoleTitle(title.c_str());
}


bool Utils::is_in_bounds(const Vector3& pos, int width, int height) {
    return pos.x >= 0 && pos.x <= width && pos.y >= 0 && pos.y <= height;
}