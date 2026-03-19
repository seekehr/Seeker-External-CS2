#include "Config.h"
#include "Settings.h"
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <string>
#include <Windows.h>

namespace {
constexpr const char* kConfigDir = "config";
constexpr const char* kConfigPath = "config/config.json";

size_t FindValueStart(const std::string& content, const char* key) {
    const std::string quotedKey = std::string("\"") + key + "\"";
    size_t keyPos = content.find(quotedKey);
    if (keyPos == std::string::npos) return std::string::npos;
    size_t colonPos = content.find(':', keyPos + quotedKey.size());
    if (colonPos == std::string::npos) return std::string::npos;
    size_t valuePos = colonPos + 1;
    while (valuePos < content.size() && std::isspace(static_cast<unsigned char>(content[valuePos]))) {
        ++valuePos;
    }
    return valuePos;
}

bool ParseBool(const std::string& content, const char* key, bool current) {
    const size_t valuePos = FindValueStart(content, key);
    if (valuePos == std::string::npos) return current;
    if (content.compare(valuePos, 4, "true") == 0) return true;
    if (content.compare(valuePos, 5, "false") == 0) return false;
    return current;
}

float ParseFloat(const std::string& content, const char* key, float current) {
    const size_t valuePos = FindValueStart(content, key);
    if (valuePos == std::string::npos) return current;

    size_t endPos = valuePos;
    while (endPos < content.size()) {
        const char c = content[endPos];
        if ((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.') {
            ++endPos;
            continue;
        }
        break;
    }
    if (endPos == valuePos) return current;

    const std::string num = content.substr(valuePos, endPos - valuePos);
    char* endPtr = nullptr;
    const float parsed = std::strtof(num.c_str(), &endPtr);
    if (endPtr == num.c_str()) return current;
    return parsed;
}

std::string ParseString(const std::string& content, const char* key, const std::string& current) {
    const size_t valuePos = FindValueStart(content, key);
    if (valuePos == std::string::npos || valuePos >= content.size() || content[valuePos] != '"') return current;
    const size_t strStart = valuePos + 1;
    const size_t strEnd = content.find('"', strStart);
    if (strEnd == std::string::npos) return current;
    return content.substr(strStart, strEnd - strStart);
}
}

bool Config::Save() {
    CreateDirectoryA(kConfigDir, nullptr);

    std::ofstream out(kConfigPath, std::ios::binary | std::ios::trunc);
    if (!out) return false;

    out << "{\n";
    out << "  \"espEnabled\": " << (g_Settings.espEnabled ? "true" : "false") << ",\n";
    out << "  \"aimbotEnabled\": " << (g_Settings.aimbotEnabled ? "true" : "false") << ",\n";
    out << "  \"drawAimbotFov\": " << (g_Settings.drawAimbotFov ? "true" : "false") << ",\n";
    out << "  \"aimbotKey\": \"" << g_Settings.GetAimbotKeyName() << "\",\n";
    out << "  \"aimbotSmoothness\": " << g_Settings.aimbotSmoothness << ",\n";
    out << "  \"aimbotFov\": " << g_Settings.aimbotFov << "\n";
    out << "}\n";

    return out.good();
}

bool Config::Load() {
    std::ifstream in(kConfigPath, std::ios::binary);
    if (!in) return false;

    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (content.empty()) return false;

    g_Settings.espEnabled = ParseBool(content, "espEnabled", g_Settings.espEnabled);
    g_Settings.aimbotEnabled = ParseBool(content, "aimbotEnabled", g_Settings.aimbotEnabled);
    g_Settings.drawAimbotFov = ParseBool(content, "drawAimbotFov", g_Settings.drawAimbotFov);
    g_Settings.aimbotSmoothness = ParseFloat(content, "aimbotSmoothness", g_Settings.aimbotSmoothness);
    g_Settings.aimbotFov = ParseFloat(content, "aimbotFov", g_Settings.aimbotFov);

    std::string key = ParseString(content, "aimbotKey", g_Settings.GetAimbotKeyName());
    if (key == "Mouse 2") g_Settings.SetAimbotKeyByIndex(0);
    else if (key == "Mouse 1") g_Settings.SetAimbotKeyByIndex(1);
    else if (key == "Mouse 4") g_Settings.SetAimbotKeyByIndex(2);
    else if (key == "Mouse 5") g_Settings.SetAimbotKeyByIndex(3);

    g_Settings.Clamp();
    return true;
}
