#include "Settings.h"

void Settings::ToggleMenu() {
    menuVisible = !menuVisible;
}

void Settings::HandleClick(int x, int y) {
    if (!menuVisible) return;

    const int baseY = MENU_Y + 52;  // Below title
    if (IsPointInToggle(x, y, 0)) {
        espEnabled = !espEnabled;
    }
}

bool Settings::IsPointInToggle(int x, int y, int index) const {
    const int baseY = MENU_Y + 52;
    const int toggleX = MENU_X + MENU_WIDTH - PADDING - TOGGLE_SIZE;
    const int toggleY = baseY + index * ROW_HEIGHT + (ROW_HEIGHT - TOGGLE_SIZE) / 2;

    return x >= toggleX && x <= toggleX + TOGGLE_SIZE &&
           y >= toggleY && y <= toggleY + TOGGLE_SIZE;
}
