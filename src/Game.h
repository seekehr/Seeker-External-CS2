#pragma once

#include "Memory.h"
#include "Offsets.h"
#include "Math.h"
#include <cstdint>

class Game {
public:
    enum class ViewMatrixSource : uint8_t {
        None = 0,
        Direct,
        Pointer
    };

    uintptr_t entityList = 0;
    uintptr_t entityListEntry = 0;
    uintptr_t viewMatrixAddr = 0;
    uintptr_t localControllerAddr = 0;
    uintptr_t localPawnAddr = 0;
    uintptr_t csgoInputAddr = 0;

    ViewMatrix viewMatrix{};
    ViewMatrixSource viewMatrixSource = ViewMatrixSource::None;
    int screenWidth = 1920;
    int screenHeight = 1080;

    bool Init();
    bool UpdateEntityList();
    bool UpdateViewMatrix();
    bool UpdateScreenSize();

    // Resolve CHandle to entity address (for m_hPlayerPawn)
    uintptr_t GetEntityFromHandle(uint32_t handle);
};

inline Game g_Game;
