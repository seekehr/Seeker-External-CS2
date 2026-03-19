#include "Memory.h"
#include "Game.h"
#include "Entity.h"
#include "Features.h"
#include "Overlay.h"
#include "Offsets.h"
#include "Settings.h"
#include "Config.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "[Seeker] Waiting for CS2...\n";

    while (!g_Memory.Attach(L"cs2.exe")) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "[Seeker] Attached to CS2\n";
    if (Offsets::LoadFromFolder("offsets")) {
        std::cout << "[Seeker] Loaded runtime offsets from offsets/\n";
    } else {
        std::cout << "[Seeker] Failed to load offsets from offsets/, using built-in defaults\n";
    }

    if (!g_Game.Init()) {
        std::cerr << "[Seeker] Failed to init game addresses\n";
        return 1;
    }

    if (!g_Overlay.Create()) {
        std::cerr << "[Seeker] Failed to create overlay (is CS2 in focus?)\n";
        return 1;
    }

    std::cout << "[Seeker] Running. Press END to exit.\n";
    std::cout << "  - INSERT: Toggle settings menu\n";
    if (Config::Load()) {
        std::cout << "[Seeker] Loaded config/config.json\n";
    }

    std::vector<PlayerEntity> enemies;

    while (g_Overlay.running) {
        if (GetAsyncKeyState(VK_END) & 1) break;
        if (GetAsyncKeyState(VK_INSERT) & 1) g_Settings.ToggleMenu();
        g_Settings.UpdateTriggerbotKeyCapture();

        if (!g_Memory.IsValid()) {
            std::cout << "[Seeker] CS2 closed\n";
            break;
        }

        // Update game state
        g_Game.UpdateEntityList();
        g_Game.UpdateViewMatrix();
        // Use overlay client dimensions for W2S; engine window offsets can mismatch in windowed/borderless.
        g_Game.screenWidth = g_Overlay.width;
        g_Game.screenHeight = g_Overlay.height;

        // Gather enemies for ESP
        enemies.clear();
        PlayerEntity local;
        bool hasLocal = UpdateLocalPlayer(local);

        if (hasLocal) {
            // list2 at entityListEntry, controllers at ENTITY_SPACING*i for i=0..63 (0-based)
            for (int i = 0; i < 64; i++) {
                uintptr_t controllerAddr = 0;
                if (!g_Memory.Read(g_Game.entityListEntry + i * Offsets::ENTITY_SPACING, controllerAddr)) continue;
                if (!controllerAddr) continue;

                PlayerEntity ent;
                if (!UpdatePlayerFromController(controllerAddr, ent)) continue;
                // Skip local player (same pawn address)
                if (ent.pawnAddr == local.pawnAddr) continue;
                if (!ent.alive || ent.team == 0) continue;

                // Get local team from controller
                uintptr_t localController = 0;
                if (g_Memory.Read(g_Game.localControllerAddr, localController) && localController) {
                    int localTeam = 0;
                    if (g_Memory.Read(localController + Offsets::C_BaseEntity::m_iTeamNum, localTeam)) {
                        if (ent.team == localTeam) continue;  // Skip teammates
                    }
                }

                enemies.push_back(ent);
            }
        }

        Features::RunAimbot(enemies);
        Features::RunTriggerbot(enemies);

        if (g_Settings.ConsumeSaveConfigRequest()) {
            if (Config::Save()) std::cout << "[Seeker] Config saved to config/config.json\n";
            else std::cout << "[Seeker] Failed to save config/config.json\n";
        }

        g_Overlay.Run(enemies, hasLocal ? &local : nullptr);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    g_Overlay.Destroy();
    g_Memory.Detach();
    std::cout << "[Seeker] Exited\n";
    return 0;
}
