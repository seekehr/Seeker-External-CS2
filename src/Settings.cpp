#include "Settings.h"
#include <algorithm>
#include <array>
#include <cstdio>

namespace {
constexpr int kDropdownHeight = 26;
constexpr int kKeybindButtonHeight = 26;
constexpr int kSliderHeight = 12;
constexpr float kMinSmoothness = 1.0f;
constexpr float kMaxSmoothness = 10.0f;
constexpr float kMinFov = 1.0f;
constexpr float kMaxFov = 300.0f;

std::string VkToName(int vk) {
    switch (vk) {
        case VK_LBUTTON: return "Mouse 1";
        case VK_RBUTTON: return "Mouse 2";
        case VK_MBUTTON: return "Mouse 3";
        case VK_XBUTTON1: return "Mouse 4";
        case VK_XBUTTON2: return "Mouse 5";
        case VK_SHIFT: return "Shift";
        case VK_CONTROL: return "Ctrl";
        case VK_MENU: return "Alt";
        case VK_SPACE: return "Space";
        case VK_TAB: return "Tab";
        case VK_CAPITAL: return "Caps";
        case VK_ESCAPE: return "Esc";
        case VK_RETURN: return "Enter";
        case VK_BACK: return "Backspace";
        case VK_UP: return "Up";
        case VK_DOWN: return "Down";
        case VK_LEFT: return "Left";
        case VK_RIGHT: return "Right";
        default: break;
    }

    if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9')) {
        char c[2] = {static_cast<char>(vk), '\0'};
        return std::string(c);
    }

    if (vk >= VK_F1 && vk <= VK_F24) {
        char fn[8];
        std::snprintf(fn, sizeof(fn), "F%d", (vk - VK_F1) + 1);
        return std::string(fn);
    }

    // Generic fallback with virtual-key number.
    char buf[16];
    std::snprintf(buf, sizeof(buf), "VK %d", vk);
    return std::string(buf);
}
}

void Settings::ToggleMenu() {
    menuVisible = !menuVisible;
    if (!menuVisible) {
        aimbotKeyDropdownOpen = false;
        triggerbotAwaitingKeybind = false;
        triggerbotCaptureStartTick = 0;
    }
}

void Settings::HandleClick(int x, int y) {
    if (!menuVisible) return;

    // While waiting for a trigger keybind, keep capture mode active until
    // an actual key/mouse input is pressed in UpdateTriggerbotKeyCapture().
    if (triggerbotAwaitingKeybind) return;

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

    const int keyRowY = baseY + 4 * ROW_HEIGHT;
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

    const int smoothRowY = baseY + 5 * ROW_HEIGHT + 8;
    const int sliderY = smoothRowY + 20;
    if (x >= contentX && x <= contentX + contentW && y >= sliderY && y <= sliderY + kSliderHeight) {
        const float ratio = static_cast<float>(x - contentX) / static_cast<float>(contentW);
        aimbotSmoothness = kMinSmoothness + ratio * (kMaxSmoothness - kMinSmoothness);
        Clamp();
        return;
    }

    const int fovRowY = baseY + 6 * ROW_HEIGHT + 8;
    const int fovSliderY = fovRowY + 20;
    if (x >= contentX && x <= contentX + contentW && y >= fovSliderY && y <= fovSliderY + kSliderHeight) {
        const float ratio = static_cast<float>(x - contentX) / static_cast<float>(contentW);
        aimbotFov = kMinFov + ratio * (kMaxFov - kMinFov);
        Clamp();
        return;
    }

    // Triggerbot toggle
    if (IsPointInToggle(x, y, 3)) {
        triggerbotEnabled = !triggerbotEnabled;
        return;
    }

    // Triggerbot keybind button
    const int triggerRowY = baseY + 7 * ROW_HEIGHT;
    const int triggerKeyY = triggerRowY + 16;
    if (x >= contentX && x <= contentX + contentW && y >= triggerKeyY && y <= triggerKeyY + kKeybindButtonHeight) {
        BeginTriggerbotKeybindCapture();
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

std::string Settings::GetTriggerbotKeyName() const {
    return VkToName(triggerbotKeyVk);
}

void Settings::BeginTriggerbotKeybindCapture() {
    triggerbotAwaitingKeybind = true;
    triggerbotCaptureStartTick = GetTickCount64();
    aimbotKeyDropdownOpen = false;
}

void Settings::UpdateTriggerbotKeyCapture() {
    if (!triggerbotAwaitingKeybind) return;

    // Prevent capturing the same click that opened the capture button.
    const ULONGLONG now = GetTickCount64();
    if (now - triggerbotCaptureStartTick < 150ULL) return;

    std::array<int, 256> keys{};
    for (int i = 0; i < 256; ++i) keys[i] = i;

    for (int vk : keys) {
        if (vk == VK_LBUTTON) continue;  // avoid accidental bind from click
        if (GetAsyncKeyState(vk) & 1) {
            triggerbotKeyVk = vk;
            triggerbotAwaitingKeybind = false;
            triggerbotCaptureStartTick = 0;
            return;
        }
    }
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
    triggerbotKeyVk = std::clamp(triggerbotKeyVk, 1, 255);
}
