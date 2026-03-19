#include "MenuOverlay.h"
#include <cstring>

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

    // Rows
    const char* labels[] = {"ESP"};
    bool values[] = {g_Settings.espEnabled};

    for (int i = 0; i < 1; i++) {
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

    // Footer
    DrawTextLeft(hdc, x + Settings::PADDING, y + h - 24, "[INSERT] toggle menu", Settings::TEXT_DIM);
}
