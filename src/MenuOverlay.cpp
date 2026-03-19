#include "MenuOverlay.h"
#include <cstring>
#include <cstdio>

static void DrawFilledRect(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    RECT rc = {x, y, x + w, y + h};
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

static void DrawBorderRect(HDC hdc, int x, int y, int w, int h, COLORREF color, int thickness = 2) {
    HPEN pen = CreatePen(PS_SOLID, thickness, color);
    HGDIOBJ old = SelectObject(hdc, pen);
    SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(hdc, x, y, x + w, y + h);
    SelectObject(hdc, old);
    DeleteObject(pen);
}

static void DrawTextCentered(HDC hdc, int x, int y, int w, const char* text, COLORREF color) {
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    RECT rc = {x, y, x + w, y + 30};
    DrawTextA(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

static void DrawTextLeft(HDC hdc, int x, int y, const char* text, COLORREF color) {
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, x, y, text, static_cast<int>(strlen(text)));
}

static void DrawOutlineRect(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    DrawBorderRect(hdc, x, y, w, h, color, 1);
}

void MenuOverlay::Draw(HDC hdc, int width, int height) {
    const int x = Settings::MENU_X;
    const int y = Settings::MENU_Y;
    const int w = Settings::MENU_WIDTH;
    const int h = Settings::MENU_HEIGHT;

    // Outer border - chunky amber retro frame
    DrawBorderRect(hdc, x - 2, y - 2, w + 4, h + 4, Settings::BORDER, 2);

    // Main panel - dark
    DrawFilledRect(hdc, x, y, w, h, Settings::BG_PANEL);

    // Inner accent line
    DrawBorderRect(hdc, x, y, w, h, RGB(60, 55, 50), 1);

    // Title - retro block text
    DrawTextLeft(hdc, x + Settings::PADDING, y + Settings::PADDING, "SEEKER", Settings::ACCENT);
    DrawTextLeft(hdc, x + Settings::PADDING, y + Settings::PADDING + 18, "--- CONFIG ---", Settings::TEXT_DIM);

    // Toggle rows
    const char* labels[] = {"ESP", "AIMBOT", "DRAW FOV"};
    bool values[] = {g_Settings.espEnabled, g_Settings.aimbotEnabled, g_Settings.drawAimbotFov};
    for (int i = 0; i < 3; i++) {
        int rowY = y + 52 + i * Settings::ROW_HEIGHT;
        DrawTextLeft(hdc, x + Settings::PADDING, rowY + 6, labels[i], Settings::TEXT);

        int toggleX = x + w - Settings::PADDING - Settings::TOGGLE_SIZE;
        int toggleY = rowY + (Settings::ROW_HEIGHT - Settings::TOGGLE_SIZE) / 2;

        // Toggle background
        DrawFilledRect(hdc, toggleX, toggleY, Settings::TOGGLE_SIZE, Settings::TOGGLE_SIZE, Settings::OFF_FILL);
        DrawBorderRect(hdc, toggleX, toggleY, Settings::TOGGLE_SIZE, Settings::TOGGLE_SIZE, Settings::BORDER, 1);

        if (values[i]) {
            // ON - filled amber inner
            DrawFilledRect(hdc, toggleX + 4, toggleY + 4, Settings::TOGGLE_SIZE - 8, Settings::TOGGLE_SIZE - 8, Settings::ON_FILL);
        }
    }

    const int contentX = x + Settings::PADDING;
    const int contentW = w - Settings::PADDING * 2;

    // Aimbot key dropdown
    const int keyRowY = y + 52 + 3 * Settings::ROW_HEIGHT;
    const int dropdownY = keyRowY + 16;
    const int dropdownH = 26;
    DrawTextLeft(hdc, contentX, keyRowY, "Aimbot Key", Settings::TEXT);
    DrawFilledRect(hdc, contentX, dropdownY, contentW, dropdownH, Settings::OFF_FILL);
    DrawOutlineRect(hdc, contentX, dropdownY, contentW, dropdownH, Settings::BORDER);
    DrawTextLeft(hdc, contentX + 8, dropdownY + 6, g_Settings.GetAimbotKeyName(), Settings::ACCENT);
    DrawTextLeft(hdc, contentX + contentW - 12, dropdownY + 6, "v", Settings::TEXT_DIM);

    if (g_Settings.aimbotKeyDropdownOpen) {
        const char* options[] = {"Mouse 2", "Mouse 1", "Mouse 4", "Mouse 5"};
        const int optionsTop = dropdownY + dropdownH;
        for (int i = 0; i < 4; ++i) {
            const int optionY = optionsTop + i * dropdownH;
            DrawFilledRect(hdc, contentX, optionY, contentW, dropdownH, Settings::BG_DARK);
            DrawOutlineRect(hdc, contentX, optionY, contentW, dropdownH, RGB(70, 66, 58));
            COLORREF optionColor = (i == g_Settings.aimbotKeyIndex) ? Settings::ACCENT : Settings::TEXT;
            DrawTextLeft(hdc, contentX + 8, optionY + 6, options[i], optionColor);
        }
    }

    // Smoothness slider
    const int smoothRowY = y + 52 + 4 * Settings::ROW_HEIGHT + 8;
    const int sliderY = smoothRowY + 20;
    const int sliderH = 12;
    DrawTextLeft(hdc, contentX, smoothRowY, "Smoothness", Settings::TEXT);
    char smoothText[32];
    snprintf(smoothText, sizeof(smoothText), "%.1f", g_Settings.aimbotSmoothness);
    DrawTextLeft(hdc, contentX + contentW - 34, smoothRowY, smoothText, Settings::ACCENT);
    DrawFilledRect(hdc, contentX, sliderY, contentW, sliderH, Settings::OFF_FILL);
    DrawOutlineRect(hdc, contentX, sliderY, contentW, sliderH, Settings::BORDER);
    float smoothRatio = (g_Settings.aimbotSmoothness - 1.0f) / (25.0f - 1.0f);
    if (smoothRatio < 0.0f) smoothRatio = 0.0f;
    if (smoothRatio > 1.0f) smoothRatio = 1.0f;
    const int fillW = static_cast<int>(contentW * smoothRatio);
    DrawFilledRect(hdc, contentX, sliderY, fillW, sliderH, Settings::ON_FILL);

    // FOV slider
    const int fovRowY = y + 52 + 5 * Settings::ROW_HEIGHT + 8;
    const int fovSliderY = fovRowY + 20;
    DrawTextLeft(hdc, contentX, fovRowY, "Aimbot FOV", Settings::TEXT);
    char fovText[32];
    snprintf(fovText, sizeof(fovText), "%.0f", g_Settings.aimbotFov);
    DrawTextLeft(hdc, contentX + contentW - 34, fovRowY, fovText, Settings::ACCENT);
    DrawFilledRect(hdc, contentX, fovSliderY, contentW, sliderH, Settings::OFF_FILL);
    DrawOutlineRect(hdc, contentX, fovSliderY, contentW, sliderH, Settings::BORDER);
    float fovRatio = (g_Settings.aimbotFov - 1.0f) / (300.0f - 1.0f);
    if (fovRatio < 0.0f) fovRatio = 0.0f;
    if (fovRatio > 1.0f) fovRatio = 1.0f;
    const int fovFillW = static_cast<int>(contentW * fovRatio);
    DrawFilledRect(hdc, contentX, fovSliderY, fovFillW, sliderH, Settings::ON_FILL);

    // Save config button (bottom)
    const int saveY = y + h - 42;
    const int saveH = 28;
    DrawFilledRect(hdc, contentX, saveY, contentW, saveH, Settings::OFF_FILL);
    DrawBorderRect(hdc, contentX, saveY, contentW, saveH, Settings::BORDER, 1);
    DrawTextCentered(hdc, contentX, saveY + 2, contentW, "SAVE CONFIG", Settings::ACCENT);

    // Footer
    DrawTextLeft(hdc, x + Settings::PADDING, y + h - 14, "[INSERT] toggle menu", Settings::TEXT_DIM);
}
