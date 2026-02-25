#include "RaySenseLogic.h"
#include "RE/B/bhkWorld.h"
#include "RE/H/hkpWorld.h"
#include "RE/H/hkpWorldRayCastInput.h"
#include "RE/H/hkpWorldRayCastOutput.h"
#include "RE/T/TESObjectCELL.h"
#include <cmath>

void RaySenseLogic::Install() {
  SKSE::log::info("RaySenseLogic: Starting Installation...");

  _verticalityFrontGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Front");
  _verticalityLeftGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Left");
  _verticalityRightGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Right");
  _verticalityObstacleGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Obstacle");
  _wallFrontGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_Wall_Front");
  _wallFrontLGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_Wall_Front_L");
  _wallFrontRGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_Wall_Front_R");
  _wallLeftGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_Wall_Left");
  _wallRightGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_Wall_Right");
  _obstacleTypeFrontGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Obstacle_Type_Front");
  _obstacleTypeLeftGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Obstacle_Type_Left");
  _obstacleTypeRightGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Obstacle_Type_Right");
  _verticalityPlayerGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Player");
  _surfaceTypeGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_SurfaceType");
  _platformTypeGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_PlatformType");
  _rawMaterialIDGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_RawMID");
  _rawLayerIDGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_RawLayer");

  if (_verticalityFrontGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Verticality_Front");
  if (_verticalityLeftGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Verticality_Left");
  if (_verticalityRightGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Verticality_Right");
  if (_verticalityObstacleGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Verticality_Obstacle");
  if (_wallFrontGlobal)
    SKSE::log::info("RaySenseLogic: Found Global RaySense_Wall_Front");
  if (_wallFrontLGlobal)
    SKSE::log::info("RaySenseLogic: Found Global RaySense_Wall_Front_L");
  if (_wallFrontRGlobal)
    SKSE::log::info("RaySenseLogic: Found Global RaySense_Wall_Front_R");
  if (_wallLeftGlobal)
    SKSE::log::info("RaySenseLogic: Found Global RaySense_Wall_Left");
  if (_wallRightGlobal)
    SKSE::log::info("RaySenseLogic: Found Global RaySense_Wall_Right");
  if (_obstacleTypeFrontGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Obstacle_Type_Front");
  if (_obstacleTypeLeftGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Obstacle_Type_Left");
  if (_obstacleTypeRightGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Obstacle_Type_Right");
  if (_verticalityPlayerGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Verticality_Player");
  if (_surfaceTypeGlobal)
    SKSE::log::info("RaySenseLogic: Found Global RaySense_SurfaceType");
  if (_platformTypeGlobal)
    SKSE::log::info("RaySenseLogic: Found Global RaySense_PlatformType");

  SKSE::log::info("RaySenseLogic: Installation Complete.");
}

void RaySenseLogic::OnJump(RE::PlayerCharacter *a_player) {
  if (!a_player)
    return;

  // Redundant updates removed to prevent crash during jump frame.
  // OnUpdate already handles these every frame.
}

void RaySenseLogic::OnUpdate(RE::PlayerCharacter *a_player, float a_delta) {
  if (!a_player || a_delta <= 0.0f || !a_player->Is3DLoaded() ||
      a_player->IsDead() || a_player->IsInKillMove())
    return;

  // Surface Info should update even when swimming or mounted
  UpdateSurfaceInfo(a_player);

  if (auto *state = a_player->AsActorState()) {
    if (a_player->IsOnMount() || state->IsSwimming()) {
      return;
    }
  }

  RE::NiPoint3 currentPos = a_player->GetPosition();
  float currentAngle = a_player->data.angle.z;

  // [Smart Caching]
  // Skip heavy calculations if player is stationary
  if (_initialized) {
    float distSq = currentPos.GetSquaredDistance(_lastUpdatePos);
    float angleDiff = std::abs(currentAngle - _lastUpdateAngle);

    // Tightened caching logic for maximum performance
    if (distSq < 0.25f && angleDiff < 0.05f) {
      // Must continue updating if in mid-air to track landing prediction
      if (!a_player->IsInMidair()) {
        return;
      }
    }
  }

  if (!IsFinite(currentPos)) {
    return;
  }

  RE::NiPoint3 forward(0, 1, 0);
  RE::NiPoint3 right(1, 0, 0);
  RE::NiPoint3 left(-1, 0, 0);

  if (auto root = a_player->Get3D()) {
    const auto &m = root->world.rotate;
    RE::NiPoint3 f = {m.entry[0][1], m.entry[1][1], m.entry[2][1]};
    RE::NiPoint3 r = {m.entry[0][0], m.entry[1][0], m.entry[2][0]};

    if (IsFinite(f) && IsFinite(r)) {
      forward = f;
      right = r;
      left = right * -1.0f;
    }
  }

  // 1. Obstacle Detection
  _obstacleVaultDist = UpdateObstacleDetection(a_player);

  _obstacleTypeFront = UpdateObstacleType(a_player, forward);
  _obstacleTypeLeft = UpdateObstacleType(a_player, left);
  _obstacleTypeRight = UpdateObstacleType(a_player, right);

  if (_obstacleTypeFrontGlobal)
    _obstacleTypeFrontGlobal->value =
        static_cast<float>(_obstacleTypeFront.load());
  if (_obstacleTypeLeftGlobal)
    _obstacleTypeLeftGlobal->value =
        static_cast<float>(_obstacleTypeLeft.load());
  if (_obstacleTypeRightGlobal)
    _obstacleTypeRightGlobal->value =
        static_cast<float>(_obstacleTypeRight.load());

  // 2. Player Height Above Ground
  _playerHeight = UpdateVerticality(_verticalityPlayerGlobal, a_player,
                                    currentPos, RE::NiPoint3(0.0f, 0.0f, 0.0f),
                                    RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f);

  // Update Surface and Platform Info
  UpdateSurfaceInfo(a_player);

  // 3. Front Check
  if (a_player->IsInMidair()) {
    // [Mid-air Velocity Calculation]
    // Calculate manual velocity from position delta for precise frame-by-frame
    // prediction. Safety: If distance is too large (Teleport/FastTravel),
    // fallback to zero or engine velocity to prevent RayCasting to infinity and
    // crashing/lagging.
    RE::NiPoint3 vel(0.0f, 0.0f, 0.0f);
    if (_initialized) {
      float distSq = currentPos.GetSquaredDistance(_lastUpdatePos);
      if (distSq < 250000.0f) { // 500 units^2. Sanity check for teleport.
        vel = (currentPos - _lastUpdatePos) / a_delta;
      } else {
        // Teleport detected: Use engine velocity as fallback or zero
        a_player->GetLinearVelocity(vel);
      }
    }

    _frontDiff =
        UpdateVerticality(_verticalityFrontGlobal, a_player, currentPos,
                          RE::NiPoint3(0.0f, 0.0f, 0.0f), vel, 0.5f, 7.0f);
  } else {
    // Grounded: Check 80 units ahead
    _frontDiff = UpdateVerticality(_verticalityFrontGlobal, a_player,
                                   currentPos, forward * 80.0f,
                                   RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f, 7.0f);
  }

  // 4. Left/Right Check (50 units sideways)
  _leftDiff = UpdateVerticality(_verticalityLeftGlobal, a_player, currentPos,
                                left * 50.0f, RE::NiPoint3(0.0f, 0.0f, 0.0f),
                                0.0f, 5.0f);
  _rightDiff = UpdateVerticality(_verticalityRightGlobal, a_player, currentPos,
                                 right * 50.0f, RE::NiPoint3(0.0f, 0.0f, 0.0f),
                                 0.0f, 5.0f);

  _lastUpdatePos = currentPos;
  _lastUpdateAngle = currentAngle;
  _initialized = true;
}

float RaySenseLogic::UpdateObstacleDetection(RE::PlayerCharacter *a_player) {
  if (!a_player)
    return 0.0f;

  RE::NiPoint3 pos = a_player->GetPosition();
  if (!IsFinite(pos))
    return 0.0f;

  RE::NiPoint3 forward(0, 1, 0);
  if (auto root = a_player->Get3D()) {
    const auto &m = root->world.rotate;
    RE::NiPoint3 f = {m.entry[0][1], m.entry[1][1], m.entry[2][1]};
    if (IsFinite(f)) {
      forward = f;
    }
  }

  bool isSprinting = a_player->AsActorState()->IsSprinting();
  float detectDistance = isSprinting ? 330.0f : 230.0f;

  auto CastHorizontalRay = [&](const RE::NiPoint3 &a_dir, float a_height,
                               float &a_dist) -> bool {
    RE::NiPoint3 rayStart = pos;
    rayStart.z += a_height;
    RE::NiPoint3 rayEnd = rayStart + (a_dir * detectDistance);

    RE::hkpWorldRayCastOutput rayOutput;
    if (PerformRayCast(a_player, rayStart, rayEnd, rayOutput)) {
      if (rayOutput.normal.quad.m128_f32[2] > 0.5f) {
        return false;
      }
      a_dist = rayOutput.hitFraction * detectDistance;
      return true;
    }
    return false;
  };

  // Front Detection
  float kneeDistFront = 0.0f, dummyDist = 0.0f;
  bool kneeHitFront = CastHorizontalRay(forward, 40.0f, kneeDistFront);
  bool chestHitFront = CastHorizontalRay(forward, 120.0f, dummyDist);

  _wallFrontDist = kneeHitFront ? std::round(kneeDistFront) : detectDistance;
  _obstacleVaultDist =
      (kneeHitFront && !chestHitFront) ? _wallFrontDist.load() : 0.0f;

  if (_wallFrontGlobal)
    _wallFrontGlobal->value = _wallFrontDist.load();
  if (_verticalityObstacleGlobal)
    _verticalityObstacleGlobal->value = _obstacleVaultDist.load();

  // Front Left/Right (Offset) Detection - Only if center hit
  if (kneeHitFront) {
    RE::NiPoint3 rightVec(0, 1, 0);
    if (auto root = a_player->Get3D()) {
      auto &m = root->world.rotate;
      rightVec = {m.entry[0][0], m.entry[1][0], m.entry[2][0]};
    }

    auto CastOffsetFrontRay = [&](const RE::NiPoint3 &a_offset,
                                  float &a_dist) -> bool {
      RE::NiPoint3 rayStart = pos + a_offset - (forward * 50.0f);
      rayStart.z += 40.0f; // Knee height
      float totalReach = detectDistance + 50.0f;
      RE::NiPoint3 rayEnd = rayStart + (forward * totalReach);

      RE::hkpWorldRayCastOutput rayOutput;
      if (PerformRayCast(a_player, rayStart, rayEnd, rayOutput)) {
        if (rayOutput.normal.quad.m128_f32[2] > 0.5f)
          return false;
        a_dist = (rayOutput.hitFraction * totalReach) - 50.0f;
        return true;
      }
      return false;
    };

    float distFrontL = 0.0f, distFrontR = 0.0f;
    bool hitL = CastOffsetFrontRay(rightVec * -100.0f, distFrontL);
    bool hitR = CastOffsetFrontRay(rightVec * 100.0f, distFrontR);

    _wallFrontLDist =
        hitL ? std::max(0.0f, std::round(distFrontL)) : detectDistance;
    _wallFrontRDist =
        hitR ? std::max(0.0f, std::round(distFrontR)) : detectDistance;
  } else {
    _wallFrontLDist = detectDistance;
    _wallFrontRDist = detectDistance;
  }

  if (_wallFrontLGlobal)
    _wallFrontLGlobal->value = _wallFrontLDist.load();
  if (_wallFrontRGlobal)
    _wallFrontRGlobal->value = _wallFrontRDist.load();

  // Left Detection
  float kneeDistLeft = 0.0f;
  RE::NiPoint3 left(-forward.y, forward.x, 0.0f);
  bool kneeHitLeft = CastHorizontalRay(left, 40.0f, kneeDistLeft);
  _wallLeftDist = kneeHitLeft ? std::round(kneeDistLeft) : detectDistance;
  if (_wallLeftGlobal)
    _wallLeftGlobal->value = _wallLeftDist.load();

  // Right Detection
  float kneeDistRight = 0.0f;
  RE::NiPoint3 right(forward.y, -forward.x, 0.0f);
  bool kneeHitRight = CastHorizontalRay(right, 40.0f, kneeDistRight);
  _wallRightDist = kneeHitRight ? std::round(kneeDistRight) : detectDistance;
  if (_wallRightGlobal)
    _wallRightGlobal->value = _wallRightDist.load();

  return _obstacleVaultDist;
}

std::uint32_t
RaySenseLogic::UpdateObstacleType(RE::PlayerCharacter *a_player,
                                  const RE::NiPoint3 &a_direction) {
  if (!a_player || !IsFinite(a_direction))
    return 0;

  float detectDistance = 250.0f;
  RE::NiPoint3 rayStart = a_player->GetPosition();
  if (!IsFinite(rayStart))
    return 0;

  rayStart.z += 100.0f; // Eye/Chest height
  RE::NiPoint3 rayEnd = rayStart + (a_direction * detectDistance);

  RE::hkpWorldRayCastOutput rayOutput;
  if (PerformRayCast(a_player, rayStart, rayEnd, rayOutput)) {
    if (rayOutput.rootCollidable) {
      auto *ref =
          RE::TESHavokUtilities::FindCollidableRef(*rayOutput.rootCollidable);
      if (ref) {
        auto *base = ref->GetBaseObject();
        if (base) {
          return static_cast<std::uint32_t>(base->GetFormType());
        }
      }
    }
  }
  return 0; // kNone
}

bool RaySenseLogic::IsObstacleDetected() const {
  return _obstacleVaultDist.load() > 0.0f;
}

float RaySenseLogic::UpdateVerticality(
    RE::TESGlobal *a_global, RE::PlayerCharacter *a_player,
    const RE::NiPoint3 &a_pos, const RE::NiPoint3 &a_offset,
    const RE::NiPoint3 &a_vel, float a_predictionTime, float a_slantAngle) {
  if (!a_player)
    return 0.0f;

  if (!IsFinite(a_pos) || !IsFinite(a_offset) || !IsFinite(a_vel) ||
      !std::isfinite(a_predictionTime) || !std::isfinite(a_slantAngle)) {
    return 0.0f;
  }

  RE::NiPoint3 rayStart = a_pos + a_offset + (a_vel * a_predictionTime);
  rayStart.z += 100.0f;

  float totalDepth = CAP_HEIGHT + 1000.0f;
  RE::NiPoint3 rayEnd = rayStart;
  rayEnd.z -= totalDepth;

  // Apply a diagonal slant to the raycast based on a_slantAngle.
  // This helps the ray over-shoot the immediate edge/slope of a cliff to find
  // the true bottom. We only slant it if there is a directional offset provided
  // (e.g., front/left/right check) and a_slantAngle > 0.
  // Player-centered drops (offset length 0) will remain strictly vertical.
  if (a_offset.Length() > 0.1f && a_slantAngle > 0.01f) {
    RE::NiPoint3 dir = a_offset;
    dir.Unitize();

    // Convert angle to radians
    float angleRad = a_slantAngle * (3.1415926535f / 180.0f);
    float tanAngle = std::tan(angleRad);
    float horizontalShift = totalDepth * tanAngle;

    rayEnd.x += dir.x * horizontalShift;
    rayEnd.y += dir.y * horizontalShift;
  }

  RE::hkpWorldRayCastOutput rayOutput;
  float terrainHeight = a_pos.z - CAP_HEIGHT; // Default to "far below"

  if (PerformRayCast(a_player, rayStart, rayEnd, rayOutput)) {
    terrainHeight =
        rayStart.z + (rayEnd.z - rayStart.z) * rayOutput.hitFraction;
  }

  float diff = std::round(a_pos.z - terrainHeight);

  // Clamp excessive values and ensure no negative distances
  if (diff > CAP_HEIGHT) {
    diff = CAP_HEIGHT;
  } else if (diff < 0.0f) {
    // If terrain is higher (diff < 0) and we hit something, set to 0.
    // If we didn't hit anything (e.g., out of bounds), treat as extreme depth.
    diff = rayOutput.HasHit() ? 0.0f : CAP_HEIGHT;
  }

  if (rayOutput.HasHit() && a_predictionTime == 0.0f &&
      a_offset.Length() < 0.1f) {
    // Terrain normal analysis was here, removed as part of Slope-to-Surface
    // update
  }

  if (a_global) {
    a_global->value = diff;
  }
  return diff;
}

// [Core Helper: PerformRayCast]
// Centralizes all Havok interaction to ensure safety and consistent settings.
bool RaySenseLogic::PerformRayCast(RE::PlayerCharacter *a_player,
                                   const RE::NiPoint3 &a_start,
                                   const RE::NiPoint3 &a_end,
                                   RE::hkpWorldRayCastOutput &a_output) {
  if (!IsFinite(a_start) || !IsFinite(a_end))
    return false;

  auto *parentCell = a_player->GetParentCell();
  if (!parentCell)
    return false;

  auto *bhkWorld_ = parentCell->GetbhkWorld();
  if (!bhkWorld_)
    return false;

  auto *hkpWorld_ = bhkWorld_->GetWorld1();
  if (!hkpWorld_)
    return false;

  float scale = RE::bhkWorld::GetWorldScale();
  RE::hkpWorldRayCastInput rayInput;
  rayInput.from = RE::hkVector4(a_start.x * scale, a_start.y * scale,
                                a_start.z * scale, 0.0f);
  rayInput.to =
      RE::hkVector4(a_end.x * scale, a_end.y * scale, a_end.z * scale, 0.0f);

  std::uint32_t filter = 0;
  a_player->GetCollisionFilterInfo(filter);
  rayInput.filterInfo = filter;

  // CRITICAL THREAD SAFETY: Havok Engine is multi-threaded.
  // We MUST acquire a read lock before casting rays.
  {
    RE::BSReadLockGuard lock(bhkWorld_->worldLock);
    hkpWorld_->CastRay(rayInput, a_output);
  }

  return a_output.HasHit();
}

bool RaySenseLogic::PerformWaterRayCast(RE::PlayerCharacter *a_player,
                                        const RE::NiPoint3 &a_start,
                                        const RE::NiPoint3 &a_end,
                                        RE::hkpWorldRayCastOutput &a_output) {
  if (!IsFinite(a_start) || !IsFinite(a_end))
    return false;

  auto *parentCell = a_player->GetParentCell();
  if (!parentCell)
    return false;

  auto *bhkWorld_ = parentCell->GetbhkWorld();
  if (!bhkWorld_)
    return false;

  auto *hkpWorld_ = bhkWorld_->GetWorld1();
  if (!hkpWorld_)
    return false;

  float scale = RE::bhkWorld::GetWorldScale();
  RE::hkpWorldRayCastInput rayInput;
  rayInput.from = RE::hkVector4(a_start.x * scale, a_start.y * scale,
                                a_start.z * scale, 0.0f);
  rayInput.to =
      RE::hkVector4(a_end.x * scale, a_end.y * scale, a_end.z * scale, 0.0f);

  // Ignore player using collision filter
  std::uint32_t filter = 0;
  a_player->GetCollisionFilterInfo(filter);
  rayInput.filterInfo = filter;

  // CRITICAL THREAD SAFETY: Havok Engine is multi-threaded.
  // We MUST acquire a read lock before casting rays.
  {
    RE::BSReadLockGuard lock(bhkWorld_->worldLock);
    hkpWorld_->CastRay(rayInput, a_output);
  }

  if (a_output.HasHit() && a_output.rootCollidable) {
    if (a_output.rootCollidable->GetCollisionLayer() == RE::COL_LAYER::kWater) {
      return true;
    }
  }
  return false;
}

void RaySenseLogic::UpdateSurfaceInfo(RE::PlayerCharacter *a_player) {
  if (!a_player)
    return;

  SurfaceType surfaceType = SurfaceType::kDefault;
  PlatformType platformType = PlatformType::kNone;

  RE::NiPoint3 pos = a_player->GetPosition();
  RE::MATERIAL_ID mID = RE::MATERIAL_ID::kNone;
  RE::COL_LAYER layer = RE::COL_LAYER::kUnidentified;

  // 1. 최우선 순위: IsSwimming() 확인
  if (auto *actorState = a_player->AsActorState()) {
    if (actorState->IsSwimming()) {
      surfaceType = SurfaceType::kWater;
      goto FinishUpdate;
    }
  }

  // 2. Precise Material Detection (Standard Raycast)
  {
    RE::NiPoint3 rayStart = pos;
    rayStart.z += 20.0f; // Lowered from 50 to avoid self-collision
    RE::NiPoint3 rayEnd = pos;
    rayEnd.z -= 40.0f;

    RE::hkpWorldRayCastOutput rayOutput;
    bool hit = PerformRayCast(a_player, rayStart, rayEnd, rayOutput);

    if (hit && rayOutput.rootCollidable) {
      layer = rayOutput.rootCollidable->GetCollisionLayer();

      if (layer == RE::COL_LAYER::kTerrain || layer == RE::COL_LAYER::kGround) {
        if (auto *tes = RE::TES::GetSingleton()) {
          RE::NiPoint3 hitPos =
              rayStart + (rayEnd - rayStart) * rayOutput.hitFraction;
          mID = tes->GetLandMaterialType(hitPos);
        }
      } else {
        if (auto *hkShape = rayOutput.rootCollidable->GetShape()) {
          if (auto *bhkShape = hkShape->userData) {
            mID = bhkShape->materialID;
          }
        }
      }
    }

    // Capture Sound Material from Controller (Splash/Footstep detect)
    RE::MATERIAL_ID soundMID = RE::MATERIAL_ID::kNone;
    if (auto *charController = a_player->GetCharController()) {
      soundMID =
          *SKSE::stl::adjust_pointer<RE::MATERIAL_ID>(charController, 0x304);
    }

    // Capture Raycast Material
    RE::MATERIAL_ID raycastMID = mID;

    // Log for debugging
    static RE::MATERIAL_ID lastSoundMID = RE::MATERIAL_ID::kNone;
    static RE::MATERIAL_ID lastRayMID = RE::MATERIAL_ID::kNone;
    if (soundMID != lastSoundMID || raycastMID != lastRayMID) {
      SKSE::log::info("DEBUG | RayMID: {} | SoundMID: {} | Layer: {}",
                      static_cast<std::uint32_t>(raycastMID),
                      static_cast<std::uint32_t>(soundMID),
                      static_cast<std::uint32_t>(layer));
      lastSoundMID = soundMID;
      lastRayMID = raycastMID;
    }

    // Debug: Export raw values
    if (_rawMaterialIDGlobal)
      _rawMaterialIDGlobal->value = static_cast<float>(raycastMID);
    if (_rawLayerIDGlobal)
      _rawLayerIDGlobal->value = static_cast<float>(soundMID);

    // 1. WATER FORCE CHECK (Splash Sound Material)
    // Sometimes water splash sound MID is picked up.
    std::uint32_t sMID = static_cast<std::uint32_t>(soundMID);
    if (sMID == 4 || sMID == 1024582599 ||
        soundMID == RE::MATERIAL_ID::kWater ||
        soundMID == RE::MATERIAL_ID::kWaterPuddle) {
      surfaceType = SurfaceType::kWater;
      goto FinishUpdate;
    }

    // 2. ICE PROTECTION
    if (raycastMID == RE::MATERIAL_ID::kIce ||
        raycastMID == RE::MATERIAL_ID::kIceForm ||
        soundMID == RE::MATERIAL_ID::kIce ||
        soundMID == RE::MATERIAL_ID::kIceForm) {
      surfaceType = SurfaceType::kIce;
      goto FinishUpdate;
    }

    // 3. WATER SUBMERSION CHECK (using GetSubmergedLevel)
    {
      static REL::Relocation<float (*)(RE::Actor *, float, RE::TESObjectCELL *)>
          GetSubmergedLevel{REL::RelocationID(36452, 37448)};

      if (GetSubmergedLevel.address()) {
        float waterLevel =
            GetSubmergedLevel(a_player, pos.z, a_player->GetParentCell());
        if (waterLevel > 0.05f) { // If Submerged Level > 0.05, the player is
                                  // wading in water.
          surfaceType = SurfaceType::kWater;
          if (_rawLayerIDGlobal)
            _rawLayerIDGlobal->value = waterLevel; // Log submerged level
          goto FinishUpdate;
        }
      }
    }

    // 4. GENERAL MATERIAL MAPPING
    RE::MATERIAL_ID finalMID =
        (raycastMID != RE::MATERIAL_ID::kNone) ? raycastMID : soundMID;
    switch (finalMID) {
    case RE::MATERIAL_ID::kGrass:
    case static_cast<RE::MATERIAL_ID>(3):
      surfaceType = SurfaceType::kGrass;
      break;
    case RE::MATERIAL_ID::kSnow:
    case RE::MATERIAL_ID::kSnowStairs:
    case static_cast<RE::MATERIAL_ID>(5):
      surfaceType = SurfaceType::kSnow;
      break;
    case RE::MATERIAL_ID::kWater:
    case RE::MATERIAL_ID::kWaterPuddle:
    case static_cast<RE::MATERIAL_ID>(4):
      surfaceType = SurfaceType::kWater;
      goto FinishUpdate;
    case RE::MATERIAL_ID::kWood:
    case RE::MATERIAL_ID::kWoodLight:
    case RE::MATERIAL_ID::kWoodHeavy:
    case RE::MATERIAL_ID::kWoodStairs:
    case RE::MATERIAL_ID::kWoodAsStairs:
    case RE::MATERIAL_ID::kBarrel:
    case RE::MATERIAL_ID::kBasket:
    case RE::MATERIAL_ID::kCarriageWheel:
    case static_cast<RE::MATERIAL_ID>(7):
    case static_cast<RE::MATERIAL_ID>(17):
    case static_cast<RE::MATERIAL_ID>(18):
      surfaceType = SurfaceType::kWood;
      break;
    case RE::MATERIAL_ID::kStone:
    case RE::MATERIAL_ID::kStoneBroken:
    case RE::MATERIAL_ID::kStoneStairs:
    case RE::MATERIAL_ID::kStoneHeavy:
    case RE::MATERIAL_ID::kStoneAsStairs:
    case RE::MATERIAL_ID::kStoneStairsBroken:
    case RE::MATERIAL_ID::kBoulderSmall:
    case RE::MATERIAL_ID::kBoulderMedium:
    case RE::MATERIAL_ID::kBoulderLarge:
    case static_cast<RE::MATERIAL_ID>(1):
    case static_cast<RE::MATERIAL_ID>(13):
      surfaceType = SurfaceType::kStone;
      break;
    case RE::MATERIAL_ID::kDirt:
    case RE::MATERIAL_ID::kMud:
    case static_cast<RE::MATERIAL_ID>(2):
    case static_cast<RE::MATERIAL_ID>(11):
      surfaceType = SurfaceType::kDirt;
      break;
    case RE::MATERIAL_ID::kSand:
    case static_cast<RE::MATERIAL_ID>(12):
      surfaceType = SurfaceType::kSand;
      break;
    case RE::MATERIAL_ID::kGravel:
    case static_cast<RE::MATERIAL_ID>(14):
      surfaceType = SurfaceType::kGravel;
      break;
    }

    // 5. LAYER-BASED FALLBACK (Wood/Props)
    if (surfaceType == SurfaceType::kDefault) {
      if (layer == RE::COL_LAYER::kTrees || layer == RE::COL_LAYER::kProps) {
        surfaceType = SurfaceType::kWood;
      }
    }
  }

FinishUpdate:
  // 5. Platform Type (Velocity-based)
  if (auto *charController = a_player->GetCharController()) {
    auto &surfaceInfo = charController->surfaceInfo;
    RE::NiPoint3 surfaceVel = {surfaceInfo.surfaceVelocity.quad.m128_f32[0],
                               surfaceInfo.surfaceVelocity.quad.m128_f32[1],
                               surfaceInfo.surfaceVelocity.quad.m128_f32[2]};

    if (surfaceVel.Length() > 0.1f) {
      platformType = PlatformType::kMoving;
    }
  }

  _surfaceType = static_cast<float>(surfaceType);
  _platformType = static_cast<float>(platformType);

  if (_surfaceTypeGlobal)
    _surfaceTypeGlobal->value = _surfaceType.load();
  if (_platformTypeGlobal)
    _platformTypeGlobal->value = _platformType.load();
}
