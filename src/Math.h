#pragma once

#include <cstdint>
#include <cmath>

struct Vec2 {
    float x, y;
};

struct Vec3 {
    float x, y, z;
};

struct ViewMatrix {
    float m[4][4];
};

inline float WorldToScreenW(const Vec3& world, const ViewMatrix& matrix) {
    return matrix.m[3][0] * world.x + matrix.m[3][1] * world.y + matrix.m[3][2] * world.z + matrix.m[3][3];
}

// World to screen using the standard CS2 external row-major projection.
inline bool WorldToScreen(const Vec3& world, Vec2& screen, const ViewMatrix& matrix, float width, float height) {
    float w = WorldToScreenW(world, matrix);
    if (w <= 0.01f) return false;

    float invW = 1.0f / w;
    float x = matrix.m[0][0] * world.x + matrix.m[0][1] * world.y + matrix.m[0][2] * world.z + matrix.m[0][3];
    float y = matrix.m[1][0] * world.x + matrix.m[1][1] * world.y + matrix.m[1][2] * world.z + matrix.m[1][3];
    screen.x = (width * 0.5f) + (0.5f * x * invW * width);
    screen.y = (height * 0.5f) - (0.5f * y * invW * height);

    return std::isfinite(screen.x) && std::isfinite(screen.y);
}
