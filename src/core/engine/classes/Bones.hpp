enum bone_index : DWORD {
    head = 6,
    neck_0 = 5,
    spine_1 = 4,
    spine_2 = 2,
    pelvis = 0,
    arm_upper_L = 8,
    arm_lower_L = 9,
    hand_L = 10,
    arm_upper_R = 13,
    arm_lower_R = 14,
    hand_R = 15,
    leg_upper_L = 22,
    leg_lower_L = 23,
    ankle_L = 24,
    leg_upper_R = 25,
    leg_lower_R = 26,
    ankle_R = 27,
};

inline const int connections[15][2] = {
    { neck_0, spine_1 },
    { spine_1, spine_2 },
    { spine_2, pelvis },
    { neck_0, arm_upper_L },
    { arm_upper_L, arm_lower_L },
    { arm_lower_L, hand_L },
    { neck_0, arm_upper_R },
    { arm_upper_R, arm_lower_R },
    { arm_lower_R, hand_R },
    { pelvis, leg_upper_L },
    { leg_upper_L, leg_lower_L },
    { leg_lower_L, ankle_L },
    { pelvis, leg_upper_R },
    { leg_upper_R, leg_lower_R },
    { leg_lower_R, ankle_R },
};

struct bone_data {
    Vec3_t pos;
    uint8_t pad[0x14];
};

struct bone_pos {
    Vec3_t pos;
};