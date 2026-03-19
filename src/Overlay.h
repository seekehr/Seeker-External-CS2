#pragma once

#include <Windows.h>
#include <vector>
#include "Entity.h"
#include "Math.h"

class Overlay {
public:
    HWND hwnd = nullptr;
    int width = 1920;
    int height = 1080;
    bool running = false;

    bool Create();
    void Destroy();
    void Run(std::vector<PlayerEntity>& enemies, PlayerEntity* localPlayer = nullptr);
    void DrawESP(std::vector<PlayerEntity>& enemies, PlayerEntity* localPlayer = nullptr);

private:
    HDC hdc = nullptr;
    HDC memDC = nullptr;
    HBITMAP memBitmap = nullptr;
    HBITMAP oldBitmap = nullptr;
    HWND targetWindow = nullptr;
    RECT lastTargetRect{0, 0, 0, 0};
    LONG_PTR cachedExStyle = 0;
    bool windowShown = false;

    bool FindTargetWindow();
    void SetupTransparentWindow();
    void RecreateBackbuffer(HDC windowDC);
    void ReleaseBackbuffer();
};

inline Overlay g_Overlay;
