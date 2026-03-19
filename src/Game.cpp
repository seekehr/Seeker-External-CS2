#include "Game.h"
#include "Debug.h"
#include <cmath>

static bool IsUsableViewMatrix(const ViewMatrix& m) {
    float absSum = 0.0f;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            const float v = m.m[r][c];
            if (!std::isfinite(v)) return false;
            absSum += std::fabs(v);
        }
    }

    // Reject fully zero / degenerate data.
    if (absSum < 0.001f) return false;

    const float perspectiveRow =
        std::fabs(m.m[3][0]) + std::fabs(m.m[3][1]) + std::fabs(m.m[3][2]) + std::fabs(m.m[3][3]);
    if (perspectiveRow < 0.0001f) return false;

    // Reject obvious garbage reads.
    if (absSum > 100000.0f) return false;

    return true;
}

bool Game::Init() {
    if (!g_Memory.IsValid()) return false;

    uintptr_t client = g_Memory.GetClientBase();
    if (!client) return false;

    entityList = client + Offsets::Client::dwEntityList;
    viewMatrixAddr = client + Offsets::Client::dwViewMatrix;
    localControllerAddr = client + Offsets::Client::dwLocalPlayerController;
    localPawnAddr = client + Offsets::Client::dwLocalPlayerPawn;
    csgoInputAddr = client + Offsets::Client::dwCSGOInput;

    return true;
}

bool Game::UpdateEntityList() {
    // Two-level dereference: entityList -> L1 -> L2 (list2)
    uintptr_t listPtr = 0;
    if (!g_Memory.Read(entityList, listPtr) || !listPtr) return false;

    if (!g_Memory.Read(listPtr + 0x10, entityListEntry) || !entityListEntry) return false;
    return true;
}

bool Game::UpdateViewMatrix() {
    // Try direct read first (common on CS2 external).
    ViewMatrix direct{};
    bool hasDirect = g_Memory.ReadRaw(viewMatrixAddr, &direct, sizeof(ViewMatrix)) && IsUsableViewMatrix(direct);

    // Fallback: some builds / bases expose dwViewMatrix as a pointer.
    uintptr_t matrixPtr = 0;
    ViewMatrix indirect{};
    bool hasIndirect = false;
    if (g_Memory.Read(viewMatrixAddr, matrixPtr) && matrixPtr &&
        matrixPtr >= 0x10000 && matrixPtr <= 0x7FFFFFFFFFFF) {
        hasIndirect = g_Memory.ReadRaw(matrixPtr, &indirect, sizeof(ViewMatrix)) && IsUsableViewMatrix(indirect);
    }

    // Prefer matrix candidate with realistic m[3][3] (normally near 0..1).
    if (hasDirect && std::fabs(direct.m[3][3]) <= 5.0f) {
        viewMatrix = direct;
        viewMatrixSource = ViewMatrixSource::Direct;
        return true;
    }
    if (hasIndirect && std::fabs(indirect.m[3][3]) <= 5.0f) {
        viewMatrix = indirect;
        viewMatrixSource = ViewMatrixSource::Pointer;
        return true;
    }
    if (hasDirect) {
        viewMatrix = direct;
        viewMatrixSource = ViewMatrixSource::Direct;
        return true;
    }
    if (hasIndirect) {
        viewMatrix = indirect;
        viewMatrixSource = ViewMatrixSource::Pointer;
        return true;
    }

    viewMatrixSource = ViewMatrixSource::None;
    return false;
}

bool Game::UpdateScreenSize() {
    uintptr_t engine = g_Memory.GetEngineBase();
    if (!engine) return false;

    int w = 0, h = 0;
    if (g_Memory.Read(engine + Offsets::Engine::dwWindowWidth, w) &&
        g_Memory.Read(engine + Offsets::Engine::dwWindowHeight, h)) {
        screenWidth = w;
        screenHeight = h;
        return true;
    }
    return false;
}

uintptr_t Game::GetEntityFromHandle(uint32_t handle) {
    if (handle == 0 || handle == 0xFFFFFFFF) return 0;

    uintptr_t listEntry = 0;
    if (!g_Memory.Read(entityList, listEntry) || !listEntry) return 0;

    // Entity list bucket: +0x10 + 8 * ((handle & 0x7FFF) >> 9)
    uintptr_t bucketAddr = listEntry + 0x10 + 8 * ((handle & 0x7FFF) >> 9);
    if (!g_Memory.Read(bucketAddr, listEntry) || !listEntry) return 0;

    // Entity within bucket: ENTITY_SPACING * (handle & 0x1FF) - 0x70 in recent CS2
    uintptr_t entityAddr = 0;
    if (!g_Memory.Read(listEntry + Offsets::ENTITY_SPACING * (handle & 0x1FF), entityAddr)) return 0;

    return entityAddr;
}
