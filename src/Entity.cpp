#include "Entity.h"
#include "Debug.h"
#include <cmath>

static bool ReadSceneNodeOrigin(uintptr_t pawnAddr, Vec3& outPos) {
    uintptr_t pSceneNode = 0;
    if (g_Memory.Read(pawnAddr + Offsets::C_BaseEntity::m_pGameSceneNode, pSceneNode) && pSceneNode) {
        if (g_Memory.Read(pSceneNode + Offsets::CGameSceneNode::m_vecAbsOrigin, outPos)) return true;
    }
    return false;
}

static bool ReadOldOrigin(uintptr_t pawnAddr, Vec3& outPos) {
    return g_Memory.Read(pawnAddr + Offsets::C_BaseEntity::m_vOldOrigin, outPos);
}

static float ReadPawnViewOffsetZ(uintptr_t pawnAddr) {
    Vec3 viewOffset{};
    if (g_Memory.Read(pawnAddr + Offsets::C_BaseEntity::m_vecViewOffset, viewOffset)) {
        // Clamp to sane player eye offsets (standing/crouching).
        if (viewOffset.z > 20.0f && viewOffset.z < 90.0f) return viewOffset.z;
    }
    return 72.0f;
}

struct ProjectionCandidate {
    bool valid = false;
    Vec3 origin{};
    Vec3 head{};
    Vec2 feetScreen{};
    Vec2 headScreen{};
    float boxHeight = 0.0f;
};

static ProjectionCandidate ProjectFromOrigin(const Vec3& origin, float headOffsetZ) {
    ProjectionCandidate c{};
    c.origin = origin;
    c.head = origin;
    c.head.z += headOffsetZ;

    bool feetOn = WorldToScreen(c.origin, c.feetScreen, g_Game.viewMatrix,
        static_cast<float>(g_Game.screenWidth), static_cast<float>(g_Game.screenHeight));
    bool headOn = WorldToScreen(c.head, c.headScreen, g_Game.viewMatrix,
        static_cast<float>(g_Game.screenWidth), static_cast<float>(g_Game.screenHeight));

    if (!feetOn && !headOn) return c;

    // If one point is missing, estimate from the other.
    if (!headOn && feetOn) {
        c.headScreen.x = c.feetScreen.x;
        c.headScreen.y = c.feetScreen.y - 60.0f;
    } else if (!feetOn && headOn) {
        c.feetScreen.x = c.headScreen.x;
        c.feetScreen.y = c.headScreen.y + 60.0f;
    }

    c.boxHeight = std::fabs(c.feetScreen.y - c.headScreen.y);
    c.valid = c.boxHeight >= 8.0f && c.boxHeight <= 1200.0f;
    return c;
}

bool UpdatePlayerFromController(uintptr_t controllerAddr, PlayerEntity& out) {
    if (!controllerAddr) return false;

    out.controllerAddr = controllerAddr;

    // Read m_hPlayerPawn (CHandle = uint32)
    uint32_t pawnHandle = 0;
    if (!g_Memory.Read(controllerAddr + Offsets::CCSPlayerController::m_hPlayerPawn, pawnHandle)) return false;
    if (pawnHandle == 0 || pawnHandle == 0xFFFFFFFF) return false;

    out.pawnAddr = g_Game.GetEntityFromHandle(pawnHandle);
    if (!out.pawnAddr) return false;

    // Read both origin sources and choose the better projected candidate.
    Vec3 sceneOrigin{}, oldOrigin{};
    bool hasScene = ReadSceneNodeOrigin(out.pawnAddr, sceneOrigin);
    bool hasOld = ReadOldOrigin(out.pawnAddr, oldOrigin);
    if (!hasScene && !hasOld) return false;
    const float headOffsetZ = ReadPawnViewOffsetZ(out.pawnAddr);

    ProjectionCandidate best{};
    if (hasScene) best = ProjectFromOrigin(sceneOrigin, headOffsetZ);
    if (hasOld) {
        ProjectionCandidate alt = ProjectFromOrigin(oldOrigin, headOffsetZ);
        if (!best.valid || (alt.valid && alt.boxHeight > best.boxHeight)) best = alt;
    }
    if (!best.valid) return false;

    out.position = best.origin;
    out.headPosition = best.head;
    out.screenPos = best.feetScreen;
    out.screenHeadPos = best.headScreen;

    if (!g_Memory.Read(out.pawnAddr + Offsets::C_BaseEntity::m_iHealth, out.health)) return false;
    if (!g_Memory.Read(controllerAddr + Offsets::C_BaseEntity::m_iTeamNum, out.team)) return false;

    out.alive = out.health > 0;
    const bool feetInBounds =
        out.screenPos.x >= 0.0f && out.screenPos.x <= static_cast<float>(g_Game.screenWidth) &&
        out.screenPos.y >= 0.0f && out.screenPos.y <= static_cast<float>(g_Game.screenHeight);
    const bool headInBounds =
        out.screenHeadPos.x >= 0.0f && out.screenHeadPos.x <= static_cast<float>(g_Game.screenWidth) &&
        out.screenHeadPos.y >= 0.0f && out.screenHeadPos.y <= static_cast<float>(g_Game.screenHeight);
    out.onScreen = feetInBounds || headInBounds;

    return true;
}

bool UpdateLocalPlayer(PlayerEntity& out) {
    // Local pawn is direct pointer
    uintptr_t pawnAddr = 0;
    if (!g_Memory.Read(g_Game.localPawnAddr, pawnAddr)) return false;
    if (!pawnAddr) return false;

    out.pawnAddr = pawnAddr;
    out.controllerAddr = 0;

    Vec3 sceneOrigin{}, oldOrigin{};
    bool hasScene = ReadSceneNodeOrigin(pawnAddr, sceneOrigin);
    bool hasOld = ReadOldOrigin(pawnAddr, oldOrigin);
    if (!hasScene && !hasOld) return false;
    const float headOffsetZ = ReadPawnViewOffsetZ(pawnAddr);

    ProjectionCandidate best{};
    if (hasScene) best = ProjectFromOrigin(sceneOrigin, headOffsetZ);
    if (hasOld) {
        ProjectionCandidate alt = ProjectFromOrigin(oldOrigin, headOffsetZ);
        if (!best.valid || (alt.valid && alt.boxHeight > best.boxHeight)) best = alt;
    }
    if (!best.valid) return false;

    out.position = best.origin;
    out.headPosition = best.head;
    out.screenPos = best.feetScreen;
    out.screenHeadPos = best.headScreen;

    if (!g_Memory.Read(pawnAddr + Offsets::C_BaseEntity::m_iHealth, out.health)) return false;

    out.alive = out.health > 0;
    out.onScreen = out.screenPos.x >= 0.0f && out.screenPos.x <= static_cast<float>(g_Game.screenWidth) &&
                   out.screenPos.y >= 0.0f && out.screenPos.y <= static_cast<float>(g_Game.screenHeight);

    return true;
}
