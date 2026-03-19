#include "Overlay.h"
#include "Game.h"
#include "Settings.h"
#include "MenuOverlay.h"
#include <cstdio>
#include <cstring>

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_DESTROY) PostQuitMessage(0);
    if (msg == WM_LBUTTONDOWN && g_Settings.menuVisible) {
        g_Settings.HandleClick(LOWORD(lp), HIWORD(lp));
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// Simple GDI drawing - no external deps
static void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int thickness = 1) {
    HPEN pen = CreatePen(PS_SOLID, thickness, color);
    HGDIOBJ old = SelectObject(hdc, pen);
    MoveToEx(hdc, x1, y1, nullptr);
    LineTo(hdc, x2, y2);
    SelectObject(hdc, old);
    DeleteObject(pen);
}

static void DrawRect(HDC hdc, int x, int y, int w, int h, COLORREF color, int thickness = 1) {
    HPEN pen = CreatePen(PS_SOLID, thickness, color);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(hdc, x, y, x + w, y + h);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

static void DrawText(HDC hdc, int x, int y, const char* text, COLORREF color) {
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, x, y, text, static_cast<int>(strlen(text)));
}

static void DrawCircle(HDC hdc, int cx, int cy, int radius, COLORREF color, int thickness = 1) {
    if (radius <= 0) return;
    HPEN pen = CreatePen(PS_SOLID, thickness, color);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

static bool PointInMenuBounds(int x, int y) {
    return x >= Settings::MENU_X &&
           x <= Settings::MENU_X + Settings::MENU_WIDTH &&
           y >= Settings::MENU_Y &&
           y <= Settings::MENU_Y + Settings::MENU_HEIGHT;
}

void Overlay::RecreateBackbuffer(HDC windowDC) {
    ReleaseBackbuffer();
    memDC = CreateCompatibleDC(windowDC);
    if (!memDC) return;
    memBitmap = CreateCompatibleBitmap(windowDC, width, height);
    if (!memBitmap) {
        DeleteDC(memDC);
        memDC = nullptr;
        return;
    }
    oldBitmap = static_cast<HBITMAP>(SelectObject(memDC, memBitmap));
}

void Overlay::ReleaseBackbuffer() {
    if (memDC && oldBitmap) {
        SelectObject(memDC, oldBitmap);
        oldBitmap = nullptr;
    }
    if (memBitmap) {
        DeleteObject(memBitmap);
        memBitmap = nullptr;
    }
    if (memDC) {
        DeleteDC(memDC);
        memDC = nullptr;
    }
}

bool Overlay::FindTargetWindow() {
    targetWindow = FindWindowW(nullptr, L"Counter-Strike 2");
    if (!targetWindow) targetWindow = FindWindowW(nullptr, L"cs2");
    return targetWindow != nullptr;
}

void Overlay::SetupTransparentWindow() {
    RECT rc;
    GetClientRect(targetWindow, &rc);
    ClientToScreen(targetWindow, reinterpret_cast<POINT*>(&rc.left));
    ClientToScreen(targetWindow, reinterpret_cast<POINT*>(&rc.right));

    const int newWidth = rc.right - rc.left;
    const int newHeight = rc.bottom - rc.top;
    const bool rectChanged = (rc.left != lastTargetRect.left) || (rc.top != lastTargetRect.top) ||
                             (rc.right != lastTargetRect.right) || (rc.bottom != lastTargetRect.bottom);
    if (!rectChanged) {
        if (!windowShown || !IsWindowVisible(hwnd)) {
            ShowWindow(hwnd, SW_SHOWNA);
            windowShown = true;
        }
        return;
    }

    width = newWidth;
    height = newHeight;
    lastTargetRect = rc;

    SetWindowPos(
        hwnd,
        HWND_TOPMOST,
        rc.left,
        rc.top,
        width,
        height,
        SWP_NOACTIVATE | SWP_SHOWWINDOW
    );

    HDC windowDC = GetDC(hwnd);
    if (windowDC) {
        RecreateBackbuffer(windowDC);
        ReleaseDC(hwnd, windowDC);
    }

    if (!windowShown || !IsWindowVisible(hwnd)) {
        ShowWindow(hwnd, SW_SHOWNA);
        windowShown = true;
    }
}

bool Overlay::Create() {
    if (!FindTargetWindow()) return false;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName = L"SeekerOverlay";

    RegisterClassExW(&wc);

    RECT rc;
    GetClientRect(targetWindow, &rc);
    ClientToScreen(targetWindow, reinterpret_cast<POINT*>(&rc.left));
    ClientToScreen(targetWindow, reinterpret_cast<POINT*>(&rc.right));

    width = rc.right - rc.left;
    height = rc.bottom - rc.top;

    hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        L"SeekerOverlay",
        L"Seeker",
        WS_POPUP,
        rc.left, rc.top, width, height,
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr
    );

    if (!hwnd) return false;

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    lastTargetRect = rc;
    cachedExStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    ShowWindow(hwnd, SW_SHOWNA);
    windowShown = true;

    HDC windowDC = GetDC(hwnd);
    if (windowDC) {
        RecreateBackbuffer(windowDC);
        ReleaseDC(hwnd, windowDC);
    }

    running = true;
    return true;
}

void Overlay::Destroy() {
    running = false;
    ReleaseBackbuffer();
    windowShown = false;
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

void Overlay::DrawESP(std::vector<PlayerEntity>& enemies, PlayerEntity* localPlayer) {
    if (!memDC) return;

    // Clear to black (transparent via LWA_COLORKEY)
    RECT rc = {0, 0, width, height};
    HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(memDC, &rc, black);
    DeleteObject(black);

    // ESP only when enabled
    if (g_Settings.espEnabled) {
        // Draw local player (blue) to verify overlay + W2S work
        if (localPlayer && localPlayer->pawnAddr) {
            int lx = static_cast<int>(localPlayer->screenPos.x);
            int ly = static_cast<int>(localPlayer->screenPos.y);
            if (localPlayer->onScreen && lx >= 0 && lx < width && ly >= 0 && ly < height) {
                int size = 6;
                DrawRect(memDC, lx - size, ly - size, size * 2, size * 2, RGB(0, 150, 255), 2);
            }
        }

        for (const auto& e : enemies) {
            if (!e.alive) continue;
            if (!e.onScreen) continue;

            // Bounding box: head (top) to feet (bottom), width ~45% of height (32/72)
            float topY = (e.screenHeadPos.y < e.screenPos.y) ? e.screenHeadPos.y : e.screenPos.y;
            float bottomY = (e.screenHeadPos.y > e.screenPos.y) ? e.screenHeadPos.y : e.screenPos.y;
            float centerX = (e.screenHeadPos.x + e.screenPos.x) * 0.5f;
            float boxHeight = bottomY - topY;
            float boxWidth = boxHeight * 0.45f;  // Player 32 units wide, 72 tall
            int left = static_cast<int>(centerX - boxWidth * 0.5f);
            int top = static_cast<int>(topY);
            int w = static_cast<int>(boxWidth);
            int h = static_cast<int>(boxHeight);

            // Skip if box is invalid or off-screen
            if (w < 4 || h < 4) continue;
            if (left + w < 0 || left > width || top + h < 0 || top > height) continue;

            // Draw full player bounding box
            DrawRect(memDC, left, top, w, h, RGB(0, 255, 0), 2);

            // Health text above box
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", e.health);
            int textX = static_cast<int>(centerX) - 15;
            int textY = top - 18;
            if (textY < 0) textY = top;
            DrawText(memDC, textX, textY, buf, RGB(255, 255, 255));
        }
    }

    // Draw settings menu when visible
    if (g_Settings.menuVisible) {
        MenuOverlay::Draw(memDC, width, height);
    }

    // Optional aimbot FOV circle
    if (g_Settings.drawAimbotFov) {
        const int cx = width / 2;
        const int cy = height / 2;
        const int radius = static_cast<int>(g_Settings.aimbotFov);
        DrawCircle(memDC, cx, cy, radius, RGB(255, 200, 50), 1);
    }

    HDC windowDC = GetDC(hwnd);
    if (!windowDC) return;
    BitBlt(windowDC, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, windowDC);
}

void Overlay::Run(std::vector<PlayerEntity>& enemies, PlayerEntity* localPlayer) {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) running = false;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (FindTargetWindow()) {
        if (!windowShown || !IsWindowVisible(hwnd)) {
            ShowWindow(hwnd, SW_SHOWNA);
            windowShown = true;
        }
        SetupTransparentWindow();

        // Click-through behavior:
        // - Menu closed: always click-through.
        // - Menu open: capture clicks only while cursor is inside menu bounds.
        LONG_PTR exStyle = cachedExStyle;
        bool shouldCaptureInput = false;
        if (g_Settings.menuVisible) {
            POINT cursor{};
            if (GetCursorPos(&cursor) && ScreenToClient(hwnd, &cursor)) {
                shouldCaptureInput = PointInMenuBounds(cursor.x, cursor.y);
            }
        }

        if (shouldCaptureInput) {
            if (exStyle & WS_EX_TRANSPARENT) {
                exStyle &= ~WS_EX_TRANSPARENT;
                SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle);
                cachedExStyle = exStyle;
            }
        } else {
            if (!(exStyle & WS_EX_TRANSPARENT)) {
                exStyle |= WS_EX_TRANSPARENT;
                SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle);
                cachedExStyle = exStyle;
            }
        }

        DrawESP(enemies, localPlayer);
    }
}
