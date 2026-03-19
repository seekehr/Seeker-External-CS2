# Seeker External - CS2 External Cheat

C++ external cheat for Counter-Strike 2. Uses ReadProcessMemory + overlay. **Educational use only. Practice in LAN/offline mode.**

## Features

- **ESP** – Enemy positions and health on screen
- **Bunnyhop** – Hold SPACE for auto bhop

## Requirements

- Windows 10/11
- MinGW-w64 (gcc/g++) – e.g. from [MSYS2](https://www.msys2.org/)
- CMake 3.16+
- Ninja or mingw32-make (for building)
- CS2 running (LAN/offline recommended)

## Build

**With MSYS2** (recommended):
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
# Add C:\msys64\mingw64\bin to PATH, then:
.\build.bat
```

**Or with make instead of ninja:**
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-cmake
```

## Usage

1. Start CS2 (join a match or LAN server)
2. Run `SeekerExternal.exe` **as Administrator**
3. ESP draws automatically
4. Hold **SPACE** for bunnyhop
5. Press **END** to exit

## Offsets

Offsets are from [a2x/cs2-dumper](https://github.com/a2x/cs2-dumper). After each CS2 update:

1. Run the dumper with CS2 open
2. Copy new values into `src/Offsets.h`

## Project Structure

```
src/
  Memory.h/cpp    - Process attach, ReadProcessMemory
  Game.h/cpp      - Addresses, entity list, view matrix
  Entity.h/cpp    - Player data, world-to-screen
  Features.h/cpp  - Bunnyhop logic
  Overlay.h/cpp   - Transparent GDI overlay
  Offsets.h       - CS2 offsets (update after patches)
  main.cpp        - Main loop
```

## Disclaimer

For educational and research purposes only. Do not use in competitive play. VAC can detect external memory access.
