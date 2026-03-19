#include "Features.h"

namespace Features {

void RunESP(std::vector<PlayerEntity>& enemies) {
    // ESP drawing is done in Overlay - this just populates the enemy list
    // The overlay will iterate enemies and draw
    (void)enemies;
}

}  // namespace Features
