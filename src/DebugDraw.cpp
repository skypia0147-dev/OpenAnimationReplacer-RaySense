#include "PCH.h"
#include "DebugDraw.h"

#include <RE/M/Main.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>

// ============================================================
// DebugDraw.cpp
//
// Sensor color legend:
//   Green  = Vertical diff < 50  (ground nearby)
//   Yellow = Vertical diff < 200 (moderate drop)
//   Red    = Vertical diff >= 200 (cliff / void)
//
//   Yellow dot = Wall detected (knee height)
//   Blue dot   = Vaultable obstacle detected
//   Blue line  = Player facing direction
// ============================================================

namespace RaySense::DebugDraw {

    struct DrawPoint { RE::NiPoint3 world; ImU32 color; float radius; };
    struct DrawLine  { RE::NiPoint3 a, b;  ImU32 color; };

    static constexpr int kMaxPts = 64;
    static constexpr int kMaxLns = 8;

    static DrawPoint g_pts[kMaxPts];
    static DrawLine  g_lns[kMaxLns];
    static int       g_nPts = 0;
    static int       g_nLns = 0;

    // ----------------------------------------------------------
    using PresentFn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
    static PresentFn g_origPresent = nullptr;
    static void**    g_vtable      = nullptr;
    static bool      g_ready       = false;

    // ----------------------------------------------------------
    static bool W2S(const RE::NiPoint3& wp, ImVec2& out, float W, float H)
    {
        auto* nc = RE::Main::WorldRootCamera();
        if (!nc) return false;
        float sx{}, sy{}, sz{};
        if (!RE::NiCamera::WorldPtToScreenPt3(
                nc->GetRuntimeData().worldToCam,
                nc->GetRuntimeData2().port,
                wp, sx, sy, sz, 1e-5f))
            return false;
        if (sz <= 0.0f) return false;
        out.x = sx * W;
        out.y = (1.0f - sy) * H;
        return true;
    }

    // ----------------------------------------------------------
    static void Render(float W, float H)
    {
        auto* dl = ImGui::GetBackgroundDrawList();
        for (int i = 0; i < g_nPts; ++i) {
            ImVec2 s;
            if (W2S(g_pts[i].world, s, W, H))
                dl->AddCircleFilled(s, g_pts[i].radius, g_pts[i].color);
        }
        for (int i = 0; i < g_nLns; ++i) {
            ImVec2 sa, sb;
            if (W2S(g_lns[i].a, sa, W, H) && W2S(g_lns[i].b, sb, W, H))
                dl->AddLine(sa, sb, g_lns[i].color, 2.0f);
        }
    }

    // ----------------------------------------------------------
    static HRESULT WINAPI HookedPresent(IDXGISwapChain* sc, UINT sync, UINT flags)
    {
        if (!g_ready) {
            ID3D11Device*        dev = nullptr;
            ID3D11DeviceContext* ctx = nullptr;
            sc->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&dev));
            if (dev) {
                dev->GetImmediateContext(&ctx);
                ImGui::CreateContext();
                ImGui::GetIO().IniFilename = nullptr;
                ImGui::GetStyle().Alpha    = 1.0f;
                ImGui_ImplDX11_Init(dev, ctx);
                dev->Release();
                if (ctx) ctx->Release();
                g_ready = true;
                SKSE::log::info("[RaySense::DebugDraw] ImGui DX11 initialized.");
            }
        }
        if (g_ready) {
            DXGI_SWAP_CHAIN_DESC desc{};
            sc->GetDesc(&desc);
            const float W = static_cast<float>(desc.BufferDesc.Width);
            const float H = static_cast<float>(desc.BufferDesc.Height);
            auto& io       = ImGui::GetIO();
            io.DisplaySize = ImVec2(W, H);
            io.DeltaTime   = 1.0f / 60.0f;
            ImGui_ImplDX11_NewFrame();
            ImGui::NewFrame();
            Render(W, H);
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }
        return g_origPresent(sc, sync, flags);
    }

    // ----------------------------------------------------------
    void Install()
    {
        auto* rend = RE::BSGraphics::Renderer::GetSingleton();
        if (!rend) { SKSE::log::error("[RaySense::DebugDraw] No Renderer."); return; }
        auto* data = rend->GetRendererData();
        if (!data) { SKSE::log::error("[RaySense::DebugDraw] No RendererData."); return; }
        auto* sc = reinterpret_cast<IDXGISwapChain*>(data->renderWindows[0].swapChain);
        if (!sc)  { SKSE::log::error("[RaySense::DebugDraw] No SwapChain."); return; }

        g_vtable = *reinterpret_cast<void***>(sc);
        DWORD old{};
        ::VirtualProtect(&g_vtable[8], sizeof(void*), PAGE_READWRITE, &old);
        g_origPresent = reinterpret_cast<PresentFn>(g_vtable[8]);
        g_vtable[8]   = reinterpret_cast<void*>(&HookedPresent);
        ::VirtualProtect(&g_vtable[8], sizeof(void*), old, &old);
        SKSE::log::info("[RaySense::DebugDraw] SwapChain::Present hooked.");
    }

    // ----------------------------------------------------------
    void Clear()
    {
        g_nPts = 0;
        g_nLns = 0;
    }

    // ----------------------------------------------------------
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
        float detectDist)
    {
        g_nPts = 0;
        g_nLns = 0;
        if (!player) return;

        const auto pPos = player->GetPosition();

        // Diff-based color: green = nearby ground, yellow = moderate, red = cliff
        auto DiffColor = [](float diff) -> ImU32 {
            if (diff < 50.0f)       return IM_COL32( 50, 220,  80, 220);
            else if (diff < 200.0f) return IM_COL32(255, 200,   0, 220);
            else                    return IM_COL32(220,  50,  50, 180);
        };

        auto AddPt = [&](const RE::NiPoint3& wp, ImU32 col, float radius) {
            if (g_nPts >= kMaxPts) return;
            g_pts[g_nPts++] = { wp, col, radius };
        };

        auto AddLn = [&](const RE::NiPoint3& a, const RE::NiPoint3& b, ImU32 col) {
            if (g_nLns < kMaxLns) g_lns[g_nLns++] = { a, b, col };
        };

        // ── 1. Vertical sensors (dot placed at actual terrain height) ──────────

        // Player: terrain directly below
        {
            RE::NiPoint3 wp = pPos;
            wp.z -= playerHeight;
            AddPt(wp, DiffColor(playerHeight), 6.0f);
        }

        // Front: 80 units forward
        {
            RE::NiPoint3 wp = pPos + forward * 80.0f;
            wp.z = pPos.z - frontDiff;
            AddPt(wp, DiffColor(frontDiff), 5.0f);
        }

        // Left: 50 units left
        {
            RE::NiPoint3 wp = pPos - right * 50.0f;
            wp.z = pPos.z - leftDiff;
            AddPt(wp, DiffColor(leftDiff), 5.0f);
        }

        // Right: 50 units right
        {
            RE::NiPoint3 wp = pPos + right * 50.0f;
            wp.z = pPos.z - rightDiff;
            AddPt(wp, DiffColor(rightDiff), 5.0f);
        }

        // ── 2. Wall sensors (knee height z+40, shown only when detected) ───────
        const float kNoHit    = detectDist * 0.95f;
        const ImU32 kWallCol  = IM_COL32(255, 200, 0, 200);

        if (wallFrontDist < kNoHit) {
            RE::NiPoint3 wp = pPos + forward * wallFrontDist;
            wp.z += 40.0f;
            AddPt(wp, kWallCol, 5.0f);
        }
        if (wallFrontLDist < kNoHit) {
            RE::NiPoint3 wp = (pPos - right * 100.0f) + forward * wallFrontLDist;
            wp.z += 40.0f;
            AddPt(wp, kWallCol, 4.0f);
        }
        if (wallFrontRDist < kNoHit) {
            RE::NiPoint3 wp = (pPos + right * 100.0f) + forward * wallFrontRDist;
            wp.z += 40.0f;
            AddPt(wp, kWallCol, 4.0f);
        }
        if (wallLeftDist < kNoHit) {
            RE::NiPoint3 wp = pPos - right * wallLeftDist;
            wp.z += 40.0f;
            AddPt(wp, kWallCol, 4.0f);
        }
        if (wallRightDist < kNoHit) {
            RE::NiPoint3 wp = pPos + right * wallRightDist;
            wp.z += 40.0f;
            AddPt(wp, kWallCol, 4.0f);
        }

        // ── 3. Vaultable obstacle (blue, larger) ──────────────────────────────
        if (obstacleVaultDist > 0.0f) {
            RE::NiPoint3 wp = pPos + forward * obstacleVaultDist;
            wp.z += 40.0f;
            AddPt(wp, IM_COL32(80, 140, 255, 220), 7.0f);
        }

        // ── 4. Player facing direction line ───────────────────────────────────
        constexpr float kHalf = 60.0f;
        RE::NiPoint3 lineBase = pPos;
        lineBase.z += 5.0f;
        AddLn(lineBase - forward * kHalf,
              lineBase + forward * kHalf,
              IM_COL32(80, 140, 255, 180));
    }

} // namespace RaySense::DebugDraw
