#include "Offsets.h"
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <Windows.h>

namespace {
bool ReadTextFile(const std::string& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    out.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return !out.empty();
}

bool ParseUIntAfterKey(const std::string& content, const char* key, uintptr_t& outValue, bool useLast = true) {
    const std::string quotedKey = std::string("\"") + key + "\"";
    size_t keyPos = useLast ? content.rfind(quotedKey) : content.find(quotedKey);
    if (keyPos == std::string::npos) return false;

    size_t colonPos = content.find(':', keyPos + quotedKey.size());
    if (colonPos == std::string::npos) return false;

    size_t valuePos = colonPos + 1;
    while (valuePos < content.size() && std::isspace(static_cast<unsigned char>(content[valuePos]))) {
        ++valuePos;
    }

    size_t endPos = valuePos;
    while (endPos < content.size() && content[endPos] >= '0' && content[endPos] <= '9') {
        ++endPos;
    }
    if (endPos == valuePos) return false;

    const std::string num = content.substr(valuePos, endPos - valuePos);
    char* endPtr = nullptr;
    const unsigned long long parsed = std::strtoull(num.c_str(), &endPtr, 10);
    if (endPtr == num.c_str()) return false;

    outValue = static_cast<uintptr_t>(parsed);
    return true;
}
}

static bool LoadFromSingleFolder(const std::string& folderPath) {
    bool anyLoaded = false;

    std::string offsetsJson;
    if (ReadTextFile(folderPath + "/offsets.json", offsetsJson)) {
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwEntityList", Offsets::Client::dwEntityList, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwLocalPlayerController", Offsets::Client::dwLocalPlayerController, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwLocalPlayerPawn", Offsets::Client::dwLocalPlayerPawn, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwViewMatrix", Offsets::Client::dwViewMatrix, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwCSGOInput", Offsets::Client::dwCSGOInput, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwWindowWidth", Offsets::Engine::dwWindowWidth, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwWindowHeight", Offsets::Engine::dwWindowHeight, false);
    }

    std::string buttonsJson;
    if (ReadTextFile(folderPath + "/buttons.json", buttonsJson)) {
        anyLoaded |= ParseUIntAfterKey(buttonsJson, "jump", Offsets::Buttons::jump, false);
    }

    std::string clientDllJson;
    if (ReadTextFile(folderPath + "/client_dll.json", clientDllJson)) {
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_hPlayerPawn", Offsets::CCSPlayerController::m_hPlayerPawn, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_iHealth", Offsets::C_BaseEntity::m_iHealth, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_iTeamNum", Offsets::C_BaseEntity::m_iTeamNum, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_fFlags", Offsets::C_BaseEntity::m_fFlags, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_pGameSceneNode", Offsets::C_BaseEntity::m_pGameSceneNode, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_vOldOrigin", Offsets::C_BaseEntity::m_vOldOrigin, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_vecViewOffset", Offsets::C_BaseEntity::m_vecViewOffset, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_iIDEntIndex", Offsets::C_CSPlayerPawnBase::m_iIDEntIndex, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_vecAbsOrigin", Offsets::CGameSceneNode::m_vecAbsOrigin, true);
    }

    return anyLoaded;
}

bool Offsets::LoadFromFolder(const std::string& folderPath) {
    std::vector<std::string> candidates;
    candidates.push_back(folderPath);
    candidates.push_back("./offsets");
    candidates.push_back("../offsets");

    char modulePath[MAX_PATH] = {0};
    if (GetModuleFileNameA(nullptr, modulePath, MAX_PATH) > 0) {
        std::string exePath(modulePath);
        size_t sep = exePath.find_last_of("\\/");
        if (sep != std::string::npos) {
            const std::string exeDir = exePath.substr(0, sep);
            candidates.push_back(exeDir + "/offsets");
            candidates.push_back(exeDir + "/../offsets");
        }
    }

    for (const auto& candidate : candidates) {
        if (LoadFromSingleFolder(candidate)) return true;
    }
    return false;
}
