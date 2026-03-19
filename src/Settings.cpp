#include "Settings.h"
#include <algorithm>

namespace {
constexpr int kDropdownHeight = 26;
constexpr int kSliderHeight = 12;
constexpr float kMinSmoothness = 1.0f;
constexpr float kMaxSmoothness = 25.0f;
constexpr float kMinFov = 1.0f;
constexpr float kMaxFov = 300.0f;
}

void Settings::ToggleMenu() {
    menuVisible = !menuVisible;
    if (!menuVisible) aimbotKeyDropdownOpen = false;
}

void Settings::HandleClick(int x, int y) {
    if (!menuVisible) return;

    const int baseY = MENU_Y + 52;
    const int contentX = MENU_X + PADDING;
    const int contentW = MENU_WIDTH - PADDING * 2;

    if (IsPointInToggle(x, y, 0)) {
        espEnabled = !espEnabled;
        return;
    }
    if (IsPointInToggle(x, y, 1)) {
        aimbotEnabled = !aimbotEnabled;
        return;
    }
    if (IsPointInToggle(x, y, 2)) {
        drawAimbotFov = !drawAimbotFov;
        return;
    }

    const int keyRowY = baseY + 3 * ROW_HEIGHT;
    const int dropdownY = keyRowY + 16;
    if (x >= contentX && x <= contentX + contentW && y >= dropdownY && y <= dropdownY + kDropdownHeight) {
        aimbotKeyDropdownOpen = !aimbotKeyDropdownOpen;
        return;
    }

    if (aimbotKeyDropdownOpen) {
        const int optionsTop = dropdownY + kDropdownHeight;
        for (int i = 0; i < 4; ++i) {
            const int optionY = optionsTop + i * kDropdownHeight;
            if (x >= contentX && x <= contentX + contentW && y >= optionY && y <= optionY + kDropdownHeight) {
                SetAimbotKeyByIndex(i);
                aimbotKeyDropdownOpen = false;
                return;
            }
        }
    }

    const int smoothRowY = baseY + 4 * ROW_HEIGHT + 8;
    const int sliderY = smoothRowY + 20;
    if (x >= contentX && x <= contentX + contentW && y >= sliderY && y <= sliderY + kSliderHeight) {
        const float ratio = static_cast<float>(x - contentX) / static_cast<float>(contentW);
        aimbotSmoothness = kMinSmoothness + ratio * (kMaxSmoothness - kMinSmoothness);
        Clamp();
        return;
    }

    const int fovRowY = baseY + 5 * ROW_HEIGHT + 8;
    const int fovSliderY = fovRowY + 20;
    if (x >= contentX && x <= contentX + contentW && y >= fovSliderY && y <= fovSliderY + kSliderHeight) {
        const float ratio = static_cast<float>(x - contentX) / static_cast<float>(contentW);
        aimbotFov = kMinFov + ratio * (kMaxFov - kMinFov);
        Clamp();
        return;
    }

    const int saveY = MENU_Y + MENU_HEIGHT - 42;
    const int saveH = 28;
    if (x >= contentX && x <= contentX + contentW && y >= saveY && y <= saveY + saveH) {
        saveConfigRequested = true;
        aimbotKeyDropdownOpen = false;
        return;
    }

    // Close dropdown if click is elsewhere in menu.
    aimbotKeyDropdownOpen = false;
}

bool Settings::IsPointInToggle(int x, int y, int index) const {
    const int baseY = MENU_Y + 52;
    const int toggleX = MENU_X + MENU_WIDTH - PADDING - TOGGLE_SIZE;
    const int toggleY = baseY + index * ROW_HEIGHT + (ROW_HEIGHT - TOGGLE_SIZE) / 2;

    return x >= toggleX && x <= toggleX + TOGGLE_SIZE &&
           y >= toggleY && y <= toggleY + TOGGLE_SIZE;
}

const char* Settings::GetAimbotKeyName() const {
    switch (aimbotKeyIndex) {
        case 0: return "Mouse 2";
        case 1: return "Mouse 1";
        case 2: return "Mouse 4";
        case 3: return "Mouse 5";
        default: return "Mouse 2";
    }
}

int Settings::GetAimbotVirtualKey() const {
    switch (aimbotKeyIndex) {
        case 0: return VK_RBUTTON;
        case 1: return VK_LBUTTON;
        case 2: return VK_XBUTTON1;
        case 3: return VK_XBUTTON2;
        default: return VK_RBUTTON;
    }
}

void Settings::SetAimbotKeyByIndex(int index) {
    aimbotKeyIndex = std::clamp(index, 0, 3);
}

bool Settings::ConsumeSaveConfigRequest() {
    if (!saveConfigRequested) return false;
    saveConfigRequested = false;
    return true;
}

void Settings::Clamp() {
    aimbotSmoothness = std::clamp(aimbotSmoothness, kMinSmoothness, kMaxSmoothness);
    aimbotFov = std::clamp(aimbotFov, kMinFov, kMaxFov);
    aimbotKeyIndex = std::clamp(aimbotKeyIndex, 0, 3);
}
