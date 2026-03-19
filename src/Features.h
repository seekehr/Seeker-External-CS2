#pragma once

#include "Entity.h"
#include "Game.h"
#include "Memory.h"
#include "Offsets.h"
#include <vector>

namespace Features {

// ESP: draw boxes/health for visible enemies (drawing done in Overlay)
void RunESP(std::vector<PlayerEntity>& enemies);
void RunAimbot(const std::vector<PlayerEntity>& enemies);

}
