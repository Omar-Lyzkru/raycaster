#include "raycaster_core.h"
#include <cmath>
#include <iostream>
#include <string>

int failures = 0;
void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}
bool near(float actual, float expected, float tolerance = 0.11f) {
    return std::abs(actual - expected) < tolerance;
}

int main(int argc, char** argv) {
    const std::string group = argc > 1 ? argv[1] : "all";
    // An asymmetric fixture catches swapped row/column indices.
    const std::wstring cells = L"#####"
                               L"#...#"
                               L"#.#.#"
                               L"##..#"
                               L"#####";
    const raycaster::Map map{cells, 5, 5};
    if (group == "all" || group == "coordinates") {
        check(map.isWall(1.5f, 3.5f), "wall lookup uses row-major coordinates");
        check(!map.isWall(3.5f, 1.5f), "open tile is not its transposed wall");
        raycaster::Player player{3.5f, 1.5f, 0.0f};
        raycaster::movePlayer(map, player, 0.0f, 1.0f);
        check(near(player.x, 3.5f) && near(player.y, 2.5f), "movement crosses open cells");
        const auto hit = raycaster::castRay(map, 1.5f, 1.5f, 0.0f, 16.0f);
        check(hit.hit && near(hit.distance, 1.5f), "ray hits the correct row's wall");
    }
    if (group == "all" || group == "bounds") {
        check(map.isWall(-0.1f, 1.5f), "negative fractional coordinate is blocked");
        check(map.isWall(5.0f, 1.0f), "right edge is blocked");
        check(map.isWall(1.0f, 5.0f), "bottom edge is blocked");
        check(map.isWall(-100.0f, -100.0f), "far outside the map is safe");
        const raycaster::Map open{L".........", 3, 3};
        raycaster::Player player{1.5f, 2.9f, 0.0f};
        raycaster::movePlayer(open, player, 0.0f, 1.0f);
        check(player.y < 3.0f, "movement cannot leave an open map edge");
    }
    if (group == "all" || group == "collision") {
        raycaster::Player player{1.5f, 2.5f, 0.0f};
        raycaster::movePlayer(map, player, 2.0f, 0.0f);
        check(player.x < 2.0f, "large movement cannot tunnel through a wall");
        raycaster::movePlayer(map, player, -2.0f, 0.0f);
        check(player.x >= 1.0f, "backward movement cannot tunnel through a wall");
        player = {1.5f, 1.5f, 0.0f};
        raycaster::movePlayer(map, player, -1.0f, 1.0f);
        check(player.x >= 1.0f && player.y > 2.0f, "movement can slide along a wall");
    }
    if (group == "all" || group == "projection") {
        check(near(raycaster::projectedDistance(4.0f, 0.0f, 0.0f), 4.0f, 0.001f), "center ray keeps its depth");
        check(near(raycaster::projectedDistance(5.0f, 0.643501109f, 0.0f), 4.0f, 0.001f), "off-center ray uses perpendicular depth");
        check(raycaster::projectedDistance(0.0f, 0.0f, 0.0f) > 0.0f, "zero depth cannot divide by zero");
    }
    if (group == "all" || group == "rays") {
        const raycaster::Map rectangle{L"#####" L"#...#" L"#####", 5, 3};
        const auto hit = raycaster::castRay(rectangle, 2.5f, 1.5f, 0.0f, 16.0f);
        check(hit.hit && near(hit.distance, 0.5f), "ray supports a non-square map");
        const raycaster::Map open{L".........", 3, 3};
        const auto edge = raycaster::castRay(open, 1.5f, 1.5f, 0.0f, 16.0f);
        check(edge.hit && near(edge.distance, 1.5f), "open map edge renders as a boundary");
        const auto limited = raycaster::castRay(map, 1.5f, 1.5f, 0.0f, 0.4f);
        check(!limited.hit && near(limited.distance, 0.4f, 0.001f), "ray respects its maximum range");
        const auto corner = raycaster::castRay(map, 2.0f, 2.0f, 0.0f, 16.0f);
        check(std::isfinite(corner.distance), "ray on a tile corner stays finite");
    }
    if (failures) return 1;
    std::cout << "PASS: " << group << '\n';
    return 0;
}
