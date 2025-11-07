#pragma once

struct view_matrix_t {
    float* operator[ ](int index) {
        return matrix[index];
    }

    float matrix[4][4];

	// World To Screen
	bool wts(const Vec3_t& pos, const Vec2_t& screen, Vec2_t& out) {
		auto bounds = RECT(
			0, 0,
			screen.x,
			screen.y
		);

		// A bit artificial, but its made by botikes 
		static int margin = 250;

		float view = 0.f;
		float SightX = (bounds.right / 2), SightY = (bounds.bottom / 2);

		view = matrix[3][0] * pos.x + matrix[3][1] * pos.y + matrix[3][2] * pos.z + matrix[3][3];

		if (view <= 0.01)
			return false;

		out.x = SightX + (matrix[0][0] * pos.x + matrix[0][1] * pos.y + matrix[0][2] * pos.z + matrix[0][3]) / view * SightX;
		out.y = SightY - (matrix[1][0] * pos.x + matrix[1][1] * pos.y + matrix[1][2] * pos.z + matrix[1][3]) / view * SightY;

		if (
			out.x < bounds.left - margin || out.x > bounds.right + margin ||
			out.y < bounds.top - margin || out.y > bounds.bottom + margin
			)
			return false;

		return true;
	}
};