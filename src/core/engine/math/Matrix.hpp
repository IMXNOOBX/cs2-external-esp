#pragma once

struct view_matrix_t {
    float* operator[ ](int index) {
        return matrix[index];
    }

    float matrix[4][4];
};