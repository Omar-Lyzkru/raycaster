# C++ Console Raycaster

An early C++ project that turns a small 2D map into a first-person, 3D-style view inside the Windows console.

The renderer casts rays toward walls and draws vertical columns of text characters based on their distance from the player. Everything is rendered through the Windows console API; no SFML or other graphics library is used.

## Features

The more complete version, [confpsbutbetter.cpp](1st%20project/confpsbutbetter.cpp), includes:

- A 120-column × 40-row text display.
- A hardcoded 16 × 16 map with walls and open spaces.
- Forward and backward movement, plus left and right rotation.
- Movement scaled by elapsed frame time.
- Wall collision checks that reverse movement when a wall is hit.
- Distance-based wall shading using Unicode block characters.
- Floor shading using `#`, `x`, `.`, and `-`.
- Wall tile boundary outlines.
- A map overlay with a `P` player marker.
- Player position, viewing angle, and FPS displayed above the map.

## Controls

| Key | Action |
| --- | --- |
| W | Move forward |
| S | Move backward |
| A | Rotate left |
| D | Rotate right |

There is no built-in exit key. Close the console window to stop the program.

## How it works

1. Read keyboard input and update the player's position and angle.
2. Cast one ray for each screen column across a 45° field of view.
3. Advance each ray in 0.1-unit steps until it reaches a wall, leaves the map, or reaches the 16-unit rendering limit.
4. Use the distance to calculate the height of the wall column: nearby walls appear taller.
5. Choose wall characters based on distance and leave thin gaps near detected tile corners.
6. Fill the ceiling and floor, add the statistics and map, and write the frame to a Windows console screen buffer.

The world is a 2D grid. The apparent depth comes from projecting wall distances into the console display.

## Repository contents

| File | Purpose |
| --- | --- |
| `1st project/confpsbutbetter.cpp` | More complete interactive raycaster with shading, collision checks, statistics, and map overlay. |
| `1st project/confps.cpp` | Earlier fixed-view prototype that draws walls with `#` characters; contains source errors. |
| `1st project/idk.cpp` | Small console test that prints `Omar`. |
| `1st project/*.exe` | Committed Windows executables; their correspondence to the current source has not been verified. |
| `.vscode/` | Windows editor and build settings, including a machine-specific MSYS2 compiler path. |

## Building on Windows

Requirements:

- Windows, because the source uses `Windows.h` and Win32 console functions.
- A C++ compiler with Windows headers and C++11 support or newer.
- A console configured for at least 120 columns and 40 rows, with a font that displays Unicode block characters.

Clone the repository:

```sh
git clone https://github.com/Omar-Lyzkru/raycaster.git
cd raycaster
```

With a Windows MinGW-w64 compiler available as `g++`, the intended build command for the more complete version is:

```sh
g++ -std=c++11 "1st project/confpsbutbetter.cpp" -o "1st project/confpsbutbetter.exe"
```

Run it from PowerShell:

```powershell
& ".\1st project\confpsbutbetter.exe"
```

The repository's VS Code build task expects `C:/msys64/ucrt64/bin/g++.exe`. Adjust that path for your installation.

**Build status:** These instructions are based on source inspection. Compilation and interactive execution have not been verified, and the current source may need the compatibility fixes below.

## Current limitations

This is an early learning project with a few rough edges:

- The older `confps.cpp` uses `nMapwidth` instead of `nMapWidth` and does not explicitly include `<cmath>`.
- The more complete version defines `NOMINMAXd` instead of `NOMINMAX`. Its `std::sinf` and `std::cosf` declarations may also need adjustment for the chosen compiler.
- Movement checks walls without checking map bounds first. Some map edges are open, so leaving the map can cause an invalid memory access.
- Rays use their raw distance for projection, without fish-eye correction.
- There is no frame limit, exit handling, or console buffer cleanup.
- There are no textures, enemies, shooting mechanics, or external map files.

## Learning focus

This project explores raycasting, trigonometry, grid maps, frame timing, keyboard input, collision checks, and console rendering in C++.
