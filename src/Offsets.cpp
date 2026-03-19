#include "Offsets.h"
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

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

bool Offsets::LoadFromFolder(const std::string& folderPath) {
    bool anyLoaded = false;

    std::string offsetsJson;
    if (ReadTextFile(folderPath + "/offsets.json", offsetsJson)) {
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwEntityList", Client::dwEntityList, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwLocalPlayerController", Client::dwLocalPlayerController, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwLocalPlayerPawn", Client::dwLocalPlayerPawn, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwViewMatrix", Client::dwViewMatrix, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwCSGOInput", Client::dwCSGOInput, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwWindowWidth", Engine::dwWindowWidth, false);
        anyLoaded |= ParseUIntAfterKey(offsetsJson, "dwWindowHeight", Engine::dwWindowHeight, false);
    }

    std::string buttonsJson;
    if (ReadTextFile(folderPath + "/buttons.json", buttonsJson)) {
        anyLoaded |= ParseUIntAfterKey(buttonsJson, "jump", Buttons::jump, false);
    }

    std::string clientDllJson;
    if (ReadTextFile(folderPath + "/client_dll.json", clientDllJson)) {
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_hPlayerPawn", CCSPlayerController::m_hPlayerPawn, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_iHealth", C_BaseEntity::m_iHealth, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_iTeamNum", C_BaseEntity::m_iTeamNum, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_fFlags", C_BaseEntity::m_fFlags, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_pGameSceneNode", C_BaseEntity::m_pGameSceneNode, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_vOldOrigin", C_BaseEntity::m_vOldOrigin, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_vecViewOffset", C_BaseEntity::m_vecViewOffset, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_iIDEntIndex", C_CSPlayerPawnBase::m_iIDEntIndex, true);
        anyLoaded |= ParseUIntAfterKey(clientDllJson, "m_vecAbsOrigin", CGameSceneNode::m_vecAbsOrigin, true);
    }

    return anyLoaded;
}
