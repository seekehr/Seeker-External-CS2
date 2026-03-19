# Seeker External - CS2 External Cheat

C++ external cheat for Counter-Strike 2

## Features

- **ESP**
- **Aimbot**
- **Triggerbot**

## Requirements

- Windows 10/11
- MinGW-w64 (gcc/g++) – e.g. from [MSYS2](https://www.msys2.org/)
- CMake 3.16+
- Ninja or mingw32-make (for building)
- CS2 running

## Build

**With MSYS2** (recommended):
```bash
.\build.bat
```

## Offsets

Offsets are from [a2x/cs2-dumper](https://github.com/a2x/cs2-dumper). After each CS2 update:

1. Run the dumper with CS2 open
2. Rename 'output' to 'offsets
3. Copy to this directory