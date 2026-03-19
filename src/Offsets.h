#pragma once

#include <cstdint>

// Wrapper around a2x cs2-dumper offsets for Seeker External
// Update after each CS2 patch - run: https://github.com/a2x/cs2-dumper
namespace Offsets {
    // Entity list spacing - changed from 0x78 to 0x70 in recent CS2 (UnknownCheats Dec 2025)
    constexpr uintptr_t ENTITY_SPACING = 0x70;
    namespace Client {
        constexpr uintptr_t dwEntityList = 0x24AF268;
        constexpr uintptr_t dwLocalPlayerController = 0x22F4188;
        constexpr uintptr_t dwLocalPlayerPawn = 0x2069B50;
        constexpr uintptr_t dwViewMatrix = 0x230FF20;
        constexpr uintptr_t dwCSGOInput = 0x2319FC0;
    }
    namespace Engine {
        constexpr uintptr_t dwWindowWidth = 0x90C8F0;
        constexpr uintptr_t dwWindowHeight = 0x90C8F4;
    }
    namespace Buttons {
        constexpr uintptr_t jump = 0x2062E00;
    }
    // Schema offsets (from client_dll.hpp)
    namespace CCSPlayerController {
        constexpr uintptr_t m_hPlayerPawn = 0x90C;
    }
    namespace C_BaseEntity {
        constexpr uintptr_t m_iHealth = 0x354;
        constexpr uintptr_t m_iTeamNum = 0x3F3;
        constexpr uintptr_t m_fFlags = 0x400;  // FL_ONGROUND = 1
        constexpr uintptr_t m_pGameSceneNode = 0x338;  // CGameSceneNode*
        constexpr uintptr_t m_vOldOrigin = 0x1588;  // Vector (often better for ESP)
        constexpr uintptr_t m_vecViewOffset = 0xD58;  // CNetworkViewOffsetVector
    }
    namespace CGameSceneNode {
        constexpr uintptr_t m_vecAbsOrigin = 0xD0;  // Vector (x,y,z)
    }
}
