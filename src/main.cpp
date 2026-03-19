#include "Memory.h"
#include "Game.h"
#include "Entity.h"
#include "Features.h"
#include "Overlay.h"
#include "Offsets.h"
#include "Settings.h"
#include "Debug.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "[Seeker] Waiting for CS2...\n";

    while (!g_Memory.Attach(L"cs2.exe")) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "[Seeker] Attached to CS2\n";

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

    std::vector<PlayerEntity> enemies;
    auto lastDebugTime = std::chrono::steady_clock::now();

    while (g_Overlay.running) {
        if (GetAsyncKeyState(VK_END) & 1) break;
        if (GetAsyncKeyState(VK_INSERT) & 1) g_Settings.ToggleMenu();

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
        int debugControllers = 0, debugPawns = 0, debugAlive = 0, debugBeforeTeam = 0;

        if (hasLocal) {
            // list2 at entityListEntry, controllers at ENTITY_SPACING*i for i=0..63 (0-based)
            for (int i = 0; i < 64; i++) {
                uintptr_t controllerAddr = 0;
                if (!g_Memory.Read(g_Game.entityListEntry + i * Offsets::ENTITY_SPACING, controllerAddr)) continue;
                if (!controllerAddr) continue;
                debugControllers++;

                PlayerEntity ent;
                if (!UpdatePlayerFromController(controllerAddr, ent)) continue;
                debugPawns++;
                // Skip local player (same pawn address)
                if (ent.pawnAddr == local.pawnAddr) continue;
                if (!ent.alive || ent.team == 0) continue;
                debugAlive++;

                // Get local team from controller
                uintptr_t localController = 0;
                if (g_Memory.Read(g_Game.localControllerAddr, localController) && localController) {
                    int localTeam = 0;
                    if (g_Memory.Read(localController + Offsets::C_BaseEntity::m_iTeamNum, localTeam)) {
                        if (ent.team == localTeam) continue;  // Skip teammates
                    }
                }
                debugBeforeTeam++;

                enemies.push_back(ent);
            }
        }

        // Debug log every 2 seconds
        auto now = std::chrono::steady_clock::now();
        if (g_DebugESP && std::chrono::duration_cast<std::chrono::seconds>(now - lastDebugTime).count() >= 2) {
            lastDebugTime = now;
            DebugLog("--- ESP Debug ---");
            DebugLog("  EntityListEntry: 0x%llX", (unsigned long long)g_Game.entityListEntry);
            const char* matrixSource =
                (g_Game.viewMatrixSource == Game::ViewMatrixSource::Direct) ? "direct" :
                (g_Game.viewMatrixSource == Game::ViewMatrixSource::Pointer) ? "pointer" : "none";
            DebugLog("  ViewMatrixSource: %s", matrixSource);
            DebugLog("  ViewMatrix[3][3]: %.4f", g_Game.viewMatrix.m[3][3]);
            DebugLog("  Screen: %dx%d", g_Game.screenWidth, g_Game.screenHeight);
            DebugLog("  LocalPawn: %s (addr 0x%llX)", hasLocal ? "OK" : "FAIL", hasLocal ? (unsigned long long)local.pawnAddr : 0);
            if (hasLocal) {
                const float localW = WorldToScreenW(local.position, g_Game.viewMatrix);
                DebugLog("  Local pos: %.1f %.1f %.1f (w=%.4f)", local.position.x, local.position.y, local.position.z, localW);
            }
            DebugLog("  Enemies: %zu (ctrl=%d pawn=%d alive=%d beforeTeam=%d)", enemies.size(), debugControllers, debugPawns, debugAlive, debugBeforeTeam);
            if (!enemies.empty()) {
                const auto& e = enemies[0];
                const float enemyW = WorldToScreenW(e.position, g_Game.viewMatrix);
                DebugLog("  First enemy: pos(%.1f,%.1f,%.1f) screen(%.0f,%.0f) hp=%d team=%d onScreen=%d w=%.4f",
                    e.position.x, e.position.y, e.position.z, e.screenPos.x, e.screenPos.y, e.health, e.team, e.onScreen, enemyW);
            }
        }

        g_Overlay.Run(enemies, hasLocal ? &local : nullptr);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    g_Overlay.Destroy();
    g_Memory.Detach();
    std::cout << "[Seeker] Exited\n";
    return 0;
}
