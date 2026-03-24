#pragma once

// ============================================================
// DebugDraw.h
// Real-time sensor visualization via ImGui + D3D11.
// Hooks IDXGISwapChain::Present and draws sensor dots in-world.
// Controlled via bEnableDebugDraw in the INI file.
// ============================================================

namespace RaySense::DebugDraw {

    void Install();
    void Clear();   // Reset draw list (call when logic is suspended)

    void Submit(
        RE::PlayerCharacter* player,
        const RE::NiPoint3&  forward,
        const RE::NiPoint3&  right,
        float playerHeight,
        float frontDiff,
        float leftDiff,
        float rightDiff,
        float wallFrontDist,
        float wallFrontLDist,
        float wallFrontRDist,
        float wallLeftDist,
        float wallRightDist,
        float obstacleVaultDist,
        float detectDist
    );

} // namespace RaySense::DebugDraw
