#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <utility>

namespace raycaster {

struct Map {
    std::wstring cells;
    int width;
    int height;

    bool contains(float x, float y) const {
        return std::isfinite(x) && std::isfinite(y) &&
               x >= 0.0f && y >= 0.0f && x < width && y < height;
    }

    bool isWall(float x, float y) const {
        // Out-of-bounds cells are solid, including the original map's open edges.
        if (!contains(x, y)) return true;
        return cells.at(static_cast<std::size_t>(static_cast<int>(y) * width +
                                                static_cast<int>(x))) == L'#';
    }
};

struct Player {
    float x;
    float y;
    float angle;
};

inline void movePlayer(const Map& map, Player& player, float dx, float dy) {
    // Small steps prevent crossing an entire wall during a long frame.
    const int steps = std::max(1, static_cast<int>(
        std::ceil(std::max(std::abs(dx), std::abs(dy)) / 0.1f)));
    const float stepX = dx / steps;
    const float stepY = dy / steps;
    for (int step = 0; step < steps; ++step) {
        if (!map.isWall(player.x + stepX, player.y)) player.x += stepX;
        if (!map.isWall(player.x, player.y + stepY)) player.y += stepY;
    }
}

struct RayHit {
    float distance;
    bool hit;
    bool boundary;
};

inline RayHit castRay(const Map& map, float x, float y, float angle, float depth) {
    const float eyeX = std::sin(angle);
    const float eyeY = std::cos(angle);
    float distance = 0.0f;
    while (distance < depth) {
        distance = std::min(distance + 0.1f, depth);
        const float testX = x + eyeX * distance;
        const float testY = y + eyeY * distance;
        if (!map.contains(testX, testY)) return {distance, true, false};
        if (!map.isWall(testX, testY)) continue;

        const int tileX = static_cast<int>(testX);
        const int tileY = static_cast<int>(testY);
        std::array<std::pair<float, float>, 4> corners{};
        int index = 0;
        for (int cx = 0; cx < 2; ++cx) {
            for (int cy = 0; cy < 2; ++cy) {
                const float vx = tileX + cx - x;
                const float vy = tileY + cy - y;
                const float length = std::sqrt(vx * vx + vy * vy);
                const float dot = length > 0.0001f
                    ? (eyeX * vx + eyeY * vy) / length : 0.0f;
                corners[index++] = {length, dot};
            }
        }
        std::sort(corners.begin(), corners.end());
        bool boundary = false;
        for (int corner = 0; corner < 3; ++corner) {
            // Rounding can push the dot product just outside acos's domain.
            const float dot = std::clamp(corners[corner].second, -1.0f, 1.0f);
            if (std::acos(dot) < 0.01f) boundary = true;
        }
        return {distance, true, boundary};
    }
    return {depth, false, false};
}

inline float projectedDistance(float distance, float rayAngle, float playerAngle) {
    // Perpendicular depth keeps a straight wall straight across the view.
    return std::max(0.1f, distance * std::cos(rayAngle - playerAngle));
}

} // namespace raycaster
