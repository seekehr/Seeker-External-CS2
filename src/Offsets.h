#pragma once

#include <cstdint>
#include <string>

// Wrapper around a2x cs2-dumper offsets for Seeker External
// Runtime-loaded from offsets/*.json, with sane defaults as fallback.
namespace Offsets {
    bool LoadFromFolder(const std::string& folderPath = "offsets");

    // Entity list spacing - changed from 0x78 to 0x70 in recent CS2.
    inline uintptr_t ENTITY_SPACING = 0x70;
    namespace Client {
        inline uintptr_t dwEntityList = 0x24AF268;
        inline uintptr_t dwLocalPlayerController = 0x22F4188;
        inline uintptr_t dwLocalPlayerPawn = 0x2069B50;
        inline uintptr_t dwViewMatrix = 0x230FF20;
        inline uintptr_t dwCSGOInput = 0x2319FC0;
    }
    namespace Engine {
        inline uintptr_t dwWindowWidth = 0x90C8F0;
        inline uintptr_t dwWindowHeight = 0x90C8F4;
    }
    namespace Buttons {
        inline uintptr_t jump = 0x2062E00;
    }
    // Schema offsets (from client_dll.json)
    namespace CCSPlayerController {
        inline uintptr_t m_hPlayerPawn = 0x90C;
    }
    namespace C_BaseEntity {
        inline uintptr_t m_iHealth = 0x354;
        inline uintptr_t m_iTeamNum = 0x3F3;
        inline uintptr_t m_fFlags = 0x400;  // FL_ONGROUND = 1
        inline uintptr_t m_pGameSceneNode = 0x338;  // CGameSceneNode*
        inline uintptr_t m_vOldOrigin = 0x1588;  // Vector (often better for ESP)
        inline uintptr_t m_vecViewOffset = 0xD58;  // CNetworkViewOffsetVector
    }
    namespace C_CSPlayerPawnBase {
        inline uintptr_t m_iIDEntIndex = 0x3EAC;  // CEntityIndex under crosshair
    }
    namespace CGameSceneNode {
        inline uintptr_t m_vecAbsOrigin = 0xD0;  // Vector (x,y,z)
    }
}
