#pragma once

#include <Windows.h>

// Retro-themed settings - dark, amber accents, CRT feel
struct Settings {
    bool menuVisible = false;
    bool espEnabled = true;

    // Menu layout
    static constexpr int MENU_X = 24;
    static constexpr int MENU_Y = 24;
    static constexpr int MENU_WIDTH = 280;
    static constexpr int MENU_HEIGHT = 200;
    static constexpr int ROW_HEIGHT = 36;
    static constexpr int TOGGLE_SIZE = 20;
    static constexpr int PADDING = 12;

    // Retro color palette
    static constexpr COLORREF BG_DARK = RGB(10, 10, 12);
    static constexpr COLORREF BG_PANEL = RGB(18, 18, 22);
    static constexpr COLORREF BORDER = RGB(230, 168, 0);   // Amber
    static constexpr COLORREF ACCENT = RGB(255, 200, 50);  // Bright amber
    static constexpr COLORREF TEXT = RGB(200, 195, 185);  // Warm gray
    static constexpr COLORREF TEXT_DIM = RGB(120, 115, 105);
    static constexpr COLORREF ON_FILL = RGB(230, 168, 0);
    static constexpr COLORREF OFF_FILL = RGB(45, 45, 50);

    void ToggleMenu();
    void HandleClick(int x, int y);
    bool IsPointInToggle(int x, int y, int index) const;
};

inline Settings g_Settings;
