#include "Features.h"
#include "Settings.h"
#include <Windows.h>
#include <chrono>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace Features {

void RunESP(std::vector<PlayerEntity>& enemies) {
    // ESP drawing is done in Overlay - this just populates the enemy list
    // The overlay will iterate enemies and draw
    (void)enemies;
}

void RunAimbot(const std::vector<PlayerEntity>& enemies) {
    if (!g_Settings.aimbotEnabled) return;
    if (enemies.empty()) return;

    const int key = g_Settings.GetAimbotVirtualKey();
    if (!(GetAsyncKeyState(key) & 0x8000)) return;

    const float cx = static_cast<float>(g_Game.screenWidth) * 0.5f;
    const float cy = static_cast<float>(g_Game.screenHeight) * 0.5f;

    struct TrackedHead {
        Vec2 pos{};
        Vec2 vel{};  // pixels/second
        std::chrono::steady_clock::time_point t{};
    };
    static std::unordered_map<uintptr_t, TrackedHead> track;
    static auto lastUpdate = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - lastUpdate).count();
    lastUpdate = now;
    if (dt < 0.001f) dt = 0.001f;
    if (dt > 0.2f) dt = 0.2f;

    Vec2 bestPredictedHead{};
    float bestDistSq = std::numeric_limits<float>::max();
    bool hasBest = false;

    for (const auto& e : enemies) {
        if (!e.alive) continue;

        const Vec2 headScreen = e.screenHeadPos;
        if (headScreen.x < 0.0f || headScreen.x > static_cast<float>(g_Game.screenWidth) ||
            headScreen.y < 0.0f || headScreen.y > static_cast<float>(g_Game.screenHeight)) {
            continue;
        }

        // Track per-target screen-space velocity and aim slightly ahead.
        Vec2 predicted = headScreen;
        auto it = track.find(e.pawnAddr);
        if (it != track.end()) {
            const float measuredVx = (headScreen.x - it->second.pos.x) / dt;
            const float measuredVy = (headScreen.y - it->second.pos.y) / dt;
            // Light smoothing reduces jitter from noisy world-to-screen updates.
            it->second.vel.x = it->second.vel.x * 0.7f + measuredVx * 0.3f;
            it->second.vel.y = it->second.vel.y * 0.7f + measuredVy * 0.3f;
            it->second.pos = headScreen;
            it->second.t = now;
            // Higher smoothness means we need a bit more lead to avoid lagging behind.
            const float leadTime = 0.015f + std::fmin(0.09f, g_Settings.aimbotSmoothness * 0.0035f);
            predicted.x += it->second.vel.x * leadTime;
            predicted.y += it->second.vel.y * leadTime;
        } else {
            track[e.pawnAddr] = {headScreen, {0.0f, 0.0f}, now};
        }

        if (predicted.x < 0.0f || predicted.x > static_cast<float>(g_Game.screenWidth) ||
            predicted.y < 0.0f || predicted.y > static_cast<float>(g_Game.screenHeight)) {
            continue;
        }

        const float dx = predicted.x - cx;
        const float dy = predicted.y - cy;
        const float distSq = dx * dx + dy * dy;
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestPredictedHead = predicted;
            hasBest = true;
        }
    }

    // Drop stale tracked entries.
    for (auto it = track.begin(); it != track.end();) {
        float age = std::chrono::duration<float>(now - it->second.t).count();
        if (age > 1.0f) it = track.erase(it);
        else ++it;
    }

    // Use configurable on-screen FOV radius around crosshair.
    const float fov = g_Settings.aimbotFov;
    if (!hasBest || bestDistSq > (fov * fov)) return;

    float moveX = (bestPredictedHead.x - cx) / g_Settings.aimbotSmoothness;
    float moveY = (bestPredictedHead.y - cy) / g_Settings.aimbotSmoothness;

    // Preserve sub-pixel movement across frames so movement does not stall at 0.
    static float residualX = 0.0f;
    static float residualY = 0.0f;
    moveX += residualX;
    moveY += residualY;

    LONG stepX = static_cast<LONG>(moveX);
    LONG stepY = static_cast<LONG>(moveY);

    if (stepX == 0 && std::fabs(moveX) >= 0.35f) stepX = (moveX > 0.0f) ? 1L : -1L;
    if (stepY == 0 && std::fabs(moveY) >= 0.35f) stepY = (moveY > 0.0f) ? 1L : -1L;

    if (stepX == 0 && stepY == 0) return;

    residualX = moveX - static_cast<float>(stepX);
    residualY = moveY - static_cast<float>(stepY);

    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.dx = stepX;
    input.mi.dy = stepY;
    if (SendInput(1, &input, sizeof(INPUT)) == 0) {
        mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(stepX), static_cast<DWORD>(stepY), 0, 0);
    }
}

}  // namespace Features
