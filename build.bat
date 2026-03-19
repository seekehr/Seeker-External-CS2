@echo off
REM Build Seeker External with MinGW
REM Auto-detects MinGW from common locations if not in PATH

REM Find MinGW bin directory (gcc must be there)
set "MINGW_BIN="
where gcc >nul 2>&1
if %errorlevel% equ 0 (
    set "MINGW_BIN=in_path"
) else if exist "C:\msys64\mingw64\bin\gcc.exe" (
    set "MINGW_BIN=C:\msys64\mingw64\bin"
) else if exist "C:\mingw64\bin\gcc.exe" (
    set "MINGW_BIN=C:\mingw64\bin"
) else if exist "C:\MinGW\bin\gcc.exe" (
    set "MINGW_BIN=C:\MinGW\bin"
) else if exist "C:\msys64\ucrt64\bin\gcc.exe" (
    set "MINGW_BIN=C:\msys64\ucrt64\bin"
)

if "%MINGW_BIN%"=="" (
    echo MinGW not found. Install MSYS2 and run:
    echo   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
    echo Then add C:\msys64\mingw64\bin to PATH, or run build.bat from MSYS2 MinGW shell.
    goto :error
)

if not "%MINGW_BIN%"=="in_path" set "PATH=%MINGW_BIN%;%PATH%"

if not exist build mkdir build
cd build

REM Clear stale cache when switching generators
if exist CMakeCache.txt del /q CMakeCache.txt
if exist CMakeFiles rmdir /s /q CMakeFiles 2>nul

REM Find mingw32-make for MinGW Makefiles
set "MAKE_EXE="
where mingw32-make >nul 2>&1 && set "MAKE_EXE=mingw32-make"
if "%MAKE_EXE%"=="" if exist "%MINGW_BIN%\mingw32-make.exe" set "MAKE_EXE=%MINGW_BIN%\mingw32-make.exe"
if "%MAKE_EXE%"=="" if exist "C:\msys64\mingw64\bin\mingw32-make.exe" set "MAKE_EXE=C:\msys64\mingw64\bin\mingw32-make.exe"

if not defined CMAKE_EXECUTABLE set CMAKE_EXECUTABLE=cmake

REM Set compiler paths (full path when using auto-detected MINGW_BIN)
set "GCC_EXE=gcc"
set "GPP_EXE=g++"
if not "%MINGW_BIN%"=="in_path" (
    set "GCC_EXE=%MINGW_BIN%\gcc.exe"
    set "GPP_EXE=%MINGW_BIN%\g++.exe"
)

REM Try Ninja first, else MinGW Makefiles
where ninja >nul 2>&1
if %errorlevel% equ 0 (
    %CMAKE_EXECUTABLE% .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=%GCC_EXE% -DCMAKE_CXX_COMPILER=%GPP_EXE%
) else if not "%MAKE_EXE%"=="" (
    %CMAKE_EXECUTABLE% .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=%GCC_EXE% -DCMAKE_CXX_COMPILER=%GPP_EXE% -DCMAKE_MAKE_PROGRAM=%MAKE_EXE%
) else (
    echo Build tool not found. Install: pacman -S mingw-w64-x86_64-ninja
    echo Or: pacman -S mingw-w64-x86_64-make
    goto :error
)
if errorlevel 1 goto :error

%CMAKE_EXECUTABLE% --build .
if errorlevel 1 goto :error

echo.
echo Build succeeded: build\SeekerExternal.exe
goto :eof

:error
echo Build failed
exit /b 1
