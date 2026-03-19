#pragma once

#include "Memory.h"
#include "Game.h"
#include "Math.h"
#include "Offsets.h"
#include <cstdint>

struct PlayerEntity {
    uintptr_t controllerAddr = 0;
    uintptr_t pawnAddr = 0;
    Vec3 position{};       // Feet position (world)
    Vec3 headPosition{};    // Head position (feet + 0,0,72)
    Vec2 screenPos{};      // Feet screen position
    Vec2 screenHeadPos{};  // Head screen position (for bounding box top)
    int health = 0;
    int team = 0;
    bool alive = false;
    bool onScreen = false;
};

// Update from controller address (from entity list iteration)
bool UpdatePlayerFromController(uintptr_t controllerAddr, PlayerEntity& out);

// Update local player (uses dwLocalPlayerPawn directly - simpler)
bool UpdateLocalPlayer(PlayerEntity& out);
