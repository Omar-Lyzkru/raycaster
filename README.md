# C++ Console Raycaster

A tutorial-based C++ learning project that turns a 2D grid map into a first-person, 3D-style view in the Windows console.

The renderer casts rays toward walls and draws vertical columns of text characters based on their distance from the player. It uses the Win32 console API, with no external graphics library.

## Features

- A 120 × 40 character display and a hardcoded 16 × 16 world.
- Forward/backward movement and left/right rotation.
- Frame-time-based movement with wall collision checks.
- Safe map boundaries and small movement steps to prevent passing through walls.
- Distance-based Unicode wall shading, floor shading, and wall tile outlines.
- Fish-eye correction using perpendicular wall distance.
- A map overlay with a player marker, position, viewing angle, and FPS.
- A target of 60 frames per second to avoid an unrestricted rendering loop.
- Escape to quit and restore the previous console screen buffer.
- Error messages for console setup or rendering failures.

## Controls

| Key | Action |
| --- | --- |
| W | Move forward |
| S | Move backward |
| A | Rotate left |
| D | Rotate right |
| Escape | Quit |

The program polls Windows key states, so these keys can also affect it while another window has focus.

## How it works

Each frame, the program reads input and updates the player. It casts one ray per screen column across a 45° field of view, advancing in 0.1-unit steps until reaching a wall, a map boundary, or the 16-unit rendering limit.

Wall distance determines the height and shade of each column. Perpendicular distance is used for projection so straight walls do not curve across the view. The renderer adds the ceiling, floor, statistics, and map before writing the frame to a Windows console screen buffer, one 120-character row at a time. Each write checks both the API result and the number of characters written.

Map coordinates consistently use X for columns and Y for rows. Out-of-bounds cells are treated as walls, including the open edges in the original map. Movement is checked in small steps and separately along each axis, allowing the player to slide along walls.

## Build and run

### Requirements

- Windows for the interactive raycaster.
- A C++17 compiler, such as MSYS2 MinGW-w64 GCC or Visual Studio's C++ tools.
- A console font that displays Unicode block characters. The window must fit 120 columns and 40 rows; use a smaller font if console sizing fails.
- CMake 3.16 or newer if using the CMake build.

Clone the repository:

~~~sh
git clone https://github.com/Omar-Lyzkru/raycaster.git
cd raycaster
~~~

### Direct build with MinGW-w64

From the repository root, with Windows MinGW-w64 `g++` on your PATH:

~~~sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic "1st project/confpsbutbetter.cpp" -o raycaster.exe -static -luser32
~~~

Run in a **standalone Command Prompt** from the repository root:

~~~bat
raycaster.exe
~~~

The intended display remains **120 × 40**. Row-by-row output has been tested successfully on the author's Windows PC in standalone Command Prompt. The VS Code integrated terminal can produce partial-write errors with this legacy Windows Console API renderer, so use standalone Command Prompt for interactive testing.

If a row is incomplete, the error reports its row number and the actual character count, for example: `Incomplete console row write: row 9 wrote 99 of 120`.

The static build bundles the GCC runtime libraries into the executable.

### Build with CMake

~~~sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
~~~

With a Visual Studio generator, run `.\build\Release\raycaster.exe`. With a single-configuration generator such as MinGW Makefiles or Ninja, run `.\build\raycaster.exe`.

## Tests

The movement and ray-calculation tests run on Windows, Linux, and macOS. The interactive application remains Windows-only; on other platforms CMake builds just the tests.

~~~sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
~~~

The tests cover row/column indexing, map boundaries, wall collisions, large movement steps, wall sliding, ray distances, rendering limits, and fish-eye correction.

**Verification:** Before the row-by-row output change, the application was cross-compiled for 64-bit Windows with MinGW-w64 GCC 13. All five calculation test groups passed on Linux, including a run with address and undefined-behavior sanitizers. The author has also confirmed that the row-by-row renderer runs in standalone Command Prompt on Windows with the MSYS2 UCRT64 compiler (`C:\msys64\ucrt64\bin\g++.exe`), including movement, rotation, the minimap, player marker, FPS, and wall/floor shading. The VS Code integrated terminal is not an interactive test target.

## Repository layout

| File | Purpose |
| --- | --- |
| `1st project/confpsbutbetter.cpp` | Windows console setup, input, frame timing, and rendering. |
| `1st project/raycaster_core.h` | Map lookup, movement, ray casting, and projection calculations. |
| `tests/raycaster_tests.cpp` | Calculation regression tests. |
| `CMakeLists.txt` | Builds the Windows application and portable tests. |
| `.gitignore` | Keeps generated binaries, build output, and personal editor settings out of Git. |

The older prototype, unrelated console test, committed executables, and personal editor settings were removed. Their originals remain available in Git history.

## Current scope

This is a learning project using a grid map and text rendering. It has no textures, enemies, shooting mechanics, or external map files. Collision treats the player as a point, and rays use fixed-size steps rather than a grid traversal algorithm. The console is restored when exiting with Escape or when a handled error occurs; forcibly terminating the process does not run that cleanup.

The project explores raycasting, trigonometry, frame timing, keyboard input, collision handling, and console rendering in C++.
