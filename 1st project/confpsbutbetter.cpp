#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "raycaster_core.h"
#include <chrono>
#include <cwchar>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {
constexpr int screenWidth = 120;
constexpr int screenHeight = 40;
constexpr float pi = 3.14159265358979323846f;
constexpr float fieldOfView = pi / 4.0f;
constexpr float renderDepth = 16.0f;
constexpr float walkingSpeed = 5.0f;
constexpr float turningSpeed = walkingSpeed * 0.75f;

const raycaster::Map world{
    L"#########......."
    L"#..............."
    L"#.......########"
    L"#..............#"
    L"#......##......#"
    L"#......##......#"
    L"#..............#"
    L"###............#"
    L"##.............#"
    L"#......####..###"
    L"#......#.......#"
    L"#......#.......#"
    L"#..............#"
    L"#......#########"
    L"#..............#"
    L"################",
    16, 16
};

void requireConsole(BOOL success, const char* operation) {
    if (!success) {
        throw std::runtime_error(std::string(operation) + " failed (Windows error " +
                                 std::to_string(GetLastError()) + ")");
    }
}

// Restore the previous console buffer on Escape, errors, and normal return.
class ConsoleBuffer {
public:
    ConsoleBuffer() : original_(GetStdHandle(STD_OUTPUT_HANDLE)) {
        DWORD mode = 0;
        requireConsole(GetConsoleMode(original_, &mode), "Opening the console");
        buffer_ = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
        if (buffer_ == INVALID_HANDLE_VALUE) requireConsole(FALSE, "Creating the screen buffer");

        try {
            // Shrink the window before resizing its backing buffer.
            const SMALL_RECT small{0, 0, 0, 0};
            requireConsole(SetConsoleWindowInfo(buffer_, TRUE, &small), "Preparing the console window");
            requireConsole(SetConsoleScreenBufferSize(buffer_, {screenWidth, screenHeight}),
                           "Sizing the console buffer");
            const SMALL_RECT window{0, 0, screenWidth - 1, screenHeight - 1};
            requireConsole(SetConsoleWindowInfo(buffer_, TRUE, &window),
                           "Sizing the console window; try a smaller console font");
            const CONSOLE_CURSOR_INFO cursor{1, FALSE};
            requireConsole(SetConsoleCursorInfo(buffer_, &cursor), "Hiding the cursor");
            requireConsole(SetConsoleActiveScreenBuffer(buffer_), "Activating the screen buffer");
        } catch (...) {
            CloseHandle(buffer_);
            buffer_ = INVALID_HANDLE_VALUE;
            throw;
        }
    }

    ~ConsoleBuffer() {
        if (buffer_ != INVALID_HANDLE_VALUE) {
            SetConsoleActiveScreenBuffer(original_);
            CloseHandle(buffer_);
        }
    }

    ConsoleBuffer(const ConsoleBuffer&) = delete;
    ConsoleBuffer& operator=(const ConsoleBuffer&) = delete;

    void display(const std::vector<wchar_t>& frame) const {
        DWORD written = 0;
        requireConsole(WriteConsoleOutputCharacterW(buffer_, frame.data(),
            static_cast<DWORD>(frame.size()), {0, 0}, &written), "Writing the frame");
        if (written != frame.size()) throw std::runtime_error("Incomplete console frame write");
    }

private:
    HANDLE original_;
    HANDLE buffer_ = INVALID_HANDLE_VALUE;
};

bool keyDown(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

wchar_t wallShade(float distance) {
    if (distance <= renderDepth / 4.0f) return L'\u2588';
    if (distance < renderDepth / 3.0f) return L'\u2593';
    if (distance < renderDepth / 2.0f) return L'\u2592';
    if (distance < renderDepth) return L'\u2591';
    return L' ';
}

wchar_t floorShade(int row) {
    const float distance = 1.0f - (row - screenHeight / 2.0f) / (screenHeight / 2.0f);
    if (distance < 0.25f) return L'#';
    if (distance < 0.5f) return L'x';
    if (distance < 0.75f) return L'.';
    if (distance < 0.9f) return L'-';
    return L' ';
}

void render(std::vector<wchar_t>& frame, const raycaster::Player& player, float fps) {
    for (int column = 0; column < screenWidth; ++column) {
        const float rayAngle = player.angle - fieldOfView / 2.0f +
            (column + 0.5f) / screenWidth * fieldOfView;
        const auto ray = raycaster::castRay(world, player.x, player.y, rayAngle, renderDepth);
        const float distance = raycaster::projectedDistance(ray.distance, rayAngle, player.angle);
        const int ceiling = static_cast<int>(screenHeight / 2.0f - screenHeight / distance);
        const int floor = screenHeight - ceiling;
        const wchar_t shade = ray.hit && !ray.boundary ? wallShade(ray.distance) : L' ';
        for (int row = 0; row < screenHeight; ++row) {
            frame[row * screenWidth + column] = row <= ceiling ? L' ' :
                (row <= floor ? shade : floorShade(row));
        }
    }

    // Use a separate string so a terminator never overwrites a rendered cell.
    wchar_t stats[screenWidth]{};
    const int length = std::swprintf(stats, screenWidth,
        L"X=%.2f Y=%.2f A=%.2f FPS=%.1f | W/S move A/D turn Esc quit",
        player.x, player.y, player.angle, fps);
    if (length > 0) std::copy_n(stats, std::min(length, screenWidth), frame.begin());

    for (int row = 0; row < world.height; ++row) {
        for (int column = 0; column < world.width; ++column) {
            frame[(row + 1) * screenWidth + column] = world.cells[row * world.width + column];
        }
    }
    frame[(static_cast<int>(player.y) + 1) * screenWidth + static_cast<int>(player.x)] = L'P';
}

int run() {
    ConsoleBuffer console;
    std::vector<wchar_t> frame(screenWidth * screenHeight, L' ');
    raycaster::Player player{14.7f, 5.09f, 0.0f};
    using Clock = std::chrono::steady_clock;
    const auto frameDuration = std::chrono::duration_cast<Clock::duration>(
        std::chrono::duration<double>(1.0 / 60.0));
    auto previousFrame = Clock::now();

    while (!keyDown(VK_ESCAPE)) {
        const auto frameStart = Clock::now();
        const float elapsed = std::chrono::duration<float>(frameStart - previousFrame).count();
        previousFrame = frameStart;
        // Pausing or dragging the window must not cause a large movement jump.
        const float delta = std::clamp(elapsed, 0.0f, 0.1f);
        const int turn = static_cast<int>(keyDown('D')) - static_cast<int>(keyDown('A'));
        player.angle = std::remainder(player.angle + turn * turningSpeed * delta, 2.0f * pi);
        const int direction = static_cast<int>(keyDown('W')) - static_cast<int>(keyDown('S'));
        const float movement = direction * walkingSpeed * delta;
        raycaster::movePlayer(world, player, std::sin(player.angle) * movement,
                              std::cos(player.angle) * movement);

        render(frame, player, elapsed > 0.0001f ? 1.0f / elapsed : 0.0f);
        console.display(frame);
        std::this_thread::sleep_until(frameStart + frameDuration);
    }
    return 0;
}
} // namespace

int main() {
    try {
        return run();
    } catch (const std::exception& error) {
        std::cerr << "Raycaster: " << error.what() << '\n';
        return 1;
    }
}
