#include "RaySenseLogic.h"
#include "DebugDraw.h"
#include "RE/B/bhkWorld.h"
#include "RE/H/hkpWorld.h"
#include "RE/H/hkpWorldRayCastInput.h"
#include "RE/H/hkpWorldRayCastOutput.h"
#include "RE/T/TESObjectCELL.h"
#include "Settings.h"
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

  // Slope Globals
  _surfaceSlopeGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_SurfaceSlope");
  _macroFrontSlopeGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_MacroSlope_Front");
  _macroLeftSlopeGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_MacroSlope_Left");
  _macroRightSlopeGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("RaySense_MacroSlope_Right");

  if (_surfaceSlopeGlobal)
    SKSE::log::info("RaySenseLogic: Found Global RaySense_SurfaceSlope");
  if (_macroFrontSlopeGlobal)
    SKSE::log::info("RaySenseLogic: Found Global RaySense_MacroSlope_Front");

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

void RaySenseLogic::OnUpdate(RE::PlayerCharacter *a_player, float a_delta) {
  if (!a_player || a_delta <= 0.0f || !a_player->Is3DLoaded() ||
      a_player->IsDead() || a_player->IsInKillMove())
    return;

  // Surface Info should update even when swimming or mounted
  UpdateSurfaceInfo(a_player);

  if (auto *state = a_player->AsActorState()) {
    if (a_player->IsOnMount() || state->IsSwimming()) {
      if (Settings::GetSingleton()->enableDebugDraw) RaySense::DebugDraw::Clear();
      return;
    }

    bool inFurniture =
        (state->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal);
    if (inFurniture) {
      _wasInFurniture = true;
      _furnitureExitCooldown = Settings::GetSingleton()->furnitureExitCooldown;
      if (Settings::GetSingleton()->enableDebugDraw) RaySense::DebugDraw::Clear();
      ClearSensors();
      return;
    }

    if (_wasInFurniture) {
      if (_furnitureExitCooldown > 0.0f) {
        _furnitureExitCooldown -= a_delta;
        if (Settings::GetSingleton()->enableDebugDraw) RaySense::DebugDraw::Clear();
        ClearSensors();
        return;
      }
      _wasInFurniture = false;
    }
  }

  RE::NiPoint3 currentPos = a_player->GetPosition();
  float currentAngle = a_player->data.angle.z;

  bool inMidair = a_player->IsInMidair();
  bool midAirChanged = inMidair != _wasInMidair;
  _wasInMidair = inMidair;

  // Keep fJumpHeightMin boosted whenever an obstacle is detected.
  // Must run BEFORE caching early-return so movement stop is reflected immediately.
  auto *settings = Settings::GetSingleton();
  if (settings->enableJumpBonus) {
    auto *gmst = RE::GameSettingCollection::GetSingleton();
    if (gmst) {
      if (auto *setting = gmst->GetSetting("fJumpHeightMin")) {
        auto* controls = RE::PlayerControls::GetSingleton();
        bool isMoving = controls && controls->data.moveInputVec.Length() > 0.1f;
        bool obstacleNow = IsObstacleDetected() && isMoving;
        if (obstacleNow && !_jumpBoosted) {
          _origJumpHeight = setting->GetFloat();
          setting->data.f = _origJumpHeight + settings->jumpBonus;
          _jumpBoosted = true;
          if (settings->enableLog)
            SKSE::log::info("JumpBonus: obstacle detected — fJumpHeightMin set to {:.1f}", setting->data.f);
        } else if (!obstacleNow && _jumpBoosted) {
          setting->data.f = _origJumpHeight;
          _jumpBoosted = false;
          if (settings->enableLog)
            SKSE::log::info("JumpBonus: obstacle cleared — fJumpHeightMin restored to {:.1f}", _origJumpHeight);
        }
      }
    }
  }

  // [Smart Caching]
  // Skip heavy calculations if player is stationary
  if (_initialized) {
    float distSq = currentPos.GetSquaredDistance(_lastUpdatePos);
    float angleDiff = std::abs(currentAngle - _lastUpdateAngle);

    // Tightened caching logic for maximum performance
    // Bypass caching if Midair state just changed (Landing/Takeoff)
    if (!midAirChanged && distSq < 0.25f && angleDiff < 0.05f) {
      // Must continue updating if in mid-air to track landing prediction
      if (!inMidair) {
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
  UpdateObstacleDetection(a_player, forward, right);

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

  // 2. Player Height Above Ground & Surface Slope
  VerticalityHitInfo playerHitInfo;
  _playerHeight = UpdateVerticality(_verticalityPlayerGlobal, a_player,
                                    currentPos, RE::NiPoint3(0.0f, 0.0f, 0.0f),
                                    RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f,
                                    &playerHitInfo);

  if (playerHitInfo.hasHit) {
    // If standing on a static object (Stairs/Building), generic normalZ is 0 or 90.
    // Use a small-scale differential to get the actual ramp/stair incline.
    if (playerHitInfo.layer == RE::COL_LAYER::kStatic || 
        playerHitInfo.layer == RE::COL_LAYER::kClutter) {
        VerticalityHitInfo stepBack, stepFront;
        float bH = UpdateVerticality(nullptr, a_player, currentPos, forward * -25.0f, {0,0,0}, 0, 0, &stepBack, true, true);
        float fH = UpdateVerticality(nullptr, a_player, currentPos, forward * 25.0f, {0,0,0}, 0, 0, &stepFront, true, true);
        if (stepBack.hasHit && stepFront.hasHit) {
            _surfaceSlope = std::atan2(bH - fH, 50.0f) * (180.0f / 3.1415926535f);
        } else {
             _surfaceSlope = std::acos(std::clamp(playerHitInfo.normalZ, -1.0f, 1.0f)) * (180.0f / 3.1415926535f);
        }
    } else {
        _surfaceSlope = std::acos(std::clamp(playerHitInfo.normalZ, -1.0f, 1.0f)) * (180.0f / 3.1415926535f);
    }
  } else {
    _surfaceSlope = 0.0f;
  }
  if (_surfaceSlopeGlobal)
    _surfaceSlopeGlobal->value = _surfaceSlope.load();

  // Helper to calculate macro slope with filtering and stair-smoothing
  auto CalculateMacroSlope = [&](RE::PlayerCharacter* a_player, const RE::NiPoint3& a_pos, 
                                 const RE::NiPoint3& a_dir_vec, float a_dist) -> float {
    VerticalityHitInfo mainHit;
    float mainDiff = UpdateVerticality(nullptr, a_player, a_pos, a_dir_vec * a_dist, {0,0,0}, 0, 7.0f, &mainHit, true, true);

    if (!mainHit.hasHit) return 0.0f;

    // Filter: Rejection of Props/Trees/Water
    if (mainHit.layer == RE::COL_LAYER::kProps || 
        mainHit.layer == RE::COL_LAYER::kTrees || 
        mainHit.layer == RE::COL_LAYER::kWater) {
      return 0.0f;
    }

    float finalTargetZ = a_pos.z - mainDiff;

    // Stair/Static Smoothing: If target is a static object, sample slightly before and after 
    // to infer the actual ramp incline of the stairs.
    if (mainHit.layer == RE::COL_LAYER::kStatic || mainHit.layer == RE::COL_LAYER::kClutter) {
        VerticalityHitInfo hitPre, hitPost;
        float diffPre = UpdateVerticality(nullptr, a_player, a_pos, a_dir_vec * (a_dist - 15.0f), {0,0,0}, 0, 0, &hitPre, true, true);
        float diffPost = UpdateVerticality(nullptr, a_player, a_pos, a_dir_vec * (a_dist + 15.0f), {0,0,0}, 0, 0, &hitPost, true, true);
        
        if (hitPre.hasHit && hitPost.hasHit) {
            // Average the surrounding geometry to treat steps as a ramp
            finalTargetZ = ((a_pos.z - diffPre) + (a_pos.z - diffPost)) / 2.0f;
        }
    }

    // atan2(height_diff, horizontal_dist)
    float slope = std::atan2(finalTargetZ - a_pos.z, a_dist) * (180.0f / 3.1415926535f);

    // Wall Rejection: Ignore vertical transitions (> 75 degrees)
    if (std::abs(slope) > 75.0f) return 0.0f;

    return slope;
  };

  // 3. Front Check
  if (a_player->IsInMidair()) {
    // [Mid-air Velocity Prediction]
    RE::NiPoint3 vel(0.0f, 0.0f, 0.0f);
    if (_initialized) {
      float dSq = currentPos.GetSquaredDistance(_lastUpdatePos);
      vel = (dSq < 250000.0f) ? (currentPos - _lastUpdatePos) / a_delta : RE::NiPoint3(0.0f, 0.0f, 0.0f);
    }
    
    _frontDiff = UpdateVerticality(_verticalityFrontGlobal, a_player, currentPos, RE::NiPoint3(0,0,0), vel, 0.5f, 7.0f, nullptr);
    // Simple slope for midair
    _macroFrontSlope = std::atan2(-_frontDiff.load(), std::max(1.0f, vel.Length() * 0.5f)) * (180.0f / 3.1415926535f);
  } else {
    _macroFrontSlope = CalculateMacroSlope(a_player, currentPos, forward, 80.0f);
    if (_verticalityFrontGlobal) {
        // Update front diff for other OAR conditions using a simple sample
        _frontDiff = UpdateVerticality(nullptr, a_player, currentPos, forward * 80.0f, {0,0,0}, 0, 7.0f, nullptr);
        _verticalityFrontGlobal->value = _frontDiff;
    }
  }
  if (_macroFrontSlopeGlobal) _macroFrontSlopeGlobal->value = _macroFrontSlope.load();

  // 4. Left/Right Check
  _macroLeftSlope = CalculateMacroSlope(a_player, currentPos, left, 50.0f);
  if (_macroLeftSlopeGlobal) _macroLeftSlopeGlobal->value = _macroLeftSlope.load();
  {
    float lVal = UpdateVerticality(nullptr, a_player, currentPos, left * 50.0f, {0,0,0}, 0, 5.0f, nullptr, true);
    _leftDiff = lVal;
    if (_verticalityLeftGlobal) _verticalityLeftGlobal->value = lVal;
  }

  _macroRightSlope = CalculateMacroSlope(a_player, currentPos, right, 50.0f);
  if (_macroRightSlopeGlobal) _macroRightSlopeGlobal->value = _macroRightSlope.load();
  {
    float rVal = UpdateVerticality(nullptr, a_player, currentPos, right * 50.0f, {0,0,0}, 0, 5.0f, nullptr, true);
    _rightDiff = rVal;
    if (_verticalityRightGlobal) _verticalityRightGlobal->value = rVal;
  }

  // DebugDraw visualization
  if (settings->enableDebugDraw) {
    bool isSprinting = a_player->AsActorState() && a_player->AsActorState()->IsSprinting();
    float detectDist = isSprinting ? settings->obstacleSprintDistance
                                   : settings->obstacleWalkDistance;
    RaySense::DebugDraw::Submit(
        a_player, forward, right,
        _playerHeight.load(),
        _frontDiff.load(), _leftDiff.load(), _rightDiff.load(),
        _wallFrontDist.load(), _wallFrontLDist.load(), _wallFrontRDist.load(),
        _wallLeftDist.load(), _wallRightDist.load(),
        _obstacleVaultDist.load(),
        detectDist);
  }

  _lastUpdatePos = currentPos;
  _lastUpdateAngle = currentAngle;
  _initialized = true;
}

void RaySenseLogic::UpdateObstacleDetection(RE::PlayerCharacter *a_player,
                                            const RE::NiPoint3 &a_forward,
                                            const RE::NiPoint3 &a_right) {
  if (!a_player)
    return;

  RE::NiPoint3 pos = a_player->GetPosition();
  if (!IsFinite(pos))
    return;

  bool isSprinting = a_player->AsActorState()->IsSprinting();
  auto *settings = Settings::GetSingleton();
  float detectDistance = isSprinting ? settings->obstacleSprintDistance
                                     : settings->obstacleWalkDistance;

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
  bool kneeHitFront = CastHorizontalRay(a_forward, 40.0f, kneeDistFront);
  bool chestHitFront = CastHorizontalRay(a_forward, 120.0f, dummyDist);

  _wallFrontDist = kneeHitFront ? std::round(kneeDistFront) : detectDistance;
  _obstacleVaultDist =
      (kneeHitFront && !chestHitFront) ? _wallFrontDist.load() : 0.0f;

  if (_wallFrontGlobal)
    _wallFrontGlobal->value = _wallFrontDist.load();
  if (_verticalityObstacleGlobal)
    _verticalityObstacleGlobal->value = _obstacleVaultDist.load();

  // Front Left/Right (Offset) Detection - Only if center hit
  if (kneeHitFront) {
    auto CastOffsetFrontRay = [&](const RE::NiPoint3 &a_offset,
                                  float &a_dist) -> bool {
      RE::NiPoint3 rayStart = pos + a_offset - (a_forward * 50.0f);
      rayStart.z += 40.0f; // Knee height
      float totalReach = detectDistance + 50.0f;
      RE::NiPoint3 rayEnd = rayStart + (a_forward * totalReach);

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
    bool hitL = CastOffsetFrontRay(a_right * -100.0f, distFrontL);
    bool hitR = CastOffsetFrontRay(a_right * 100.0f, distFrontR);

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
  RE::NiPoint3 left = a_right * -1.0f;
  bool kneeHitLeft = CastHorizontalRay(left, 40.0f, kneeDistLeft);
  _wallLeftDist = kneeHitLeft ? std::round(kneeDistLeft) : detectDistance;
  if (_wallLeftGlobal)
    _wallLeftGlobal->value = _wallLeftDist.load();

  // Right Detection
  float kneeDistRight = 0.0f;
  bool kneeHitRight = CastHorizontalRay(a_right, 40.0f, kneeDistRight);
  _wallRightDist = kneeHitRight ? std::round(kneeDistRight) : detectDistance;
  if (_wallRightGlobal)
    _wallRightGlobal->value = _wallRightDist.load();
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
    const RE::NiPoint3 &a_vel, float a_predictionTime, float a_slantAngle,
    VerticalityHitInfo *a_outHitInfo, bool a_skipPreCheck, bool a_allowNegativeDiff) {
  if (!a_player)
    return 0.0f;

  if (a_outHitInfo)
    *a_outHitInfo = {};

  if (!IsFinite(a_pos) || !IsFinite(a_offset) || !IsFinite(a_vel) ||
      !std::isfinite(a_predictionTime) || !std::isfinite(a_slantAngle)) {
    return 0.0f;
  }

  // [Pre-check] Horizontal ray from player to offset position.
  // Detects obstacles (tables, boxes, etc.) between the player and the target.
  if (!a_skipPreCheck && a_offset.Length() > 0.1f) {
    RE::NiPoint3 dir = a_offset;
    dir.Unitize();
    
    // Start ray 35 units away from player center to avoid hitting self.
    RE::NiPoint3 hRayStart = a_pos + (dir * 35.0f);
    hRayStart.z += 60.0f; // Waist height
    RE::NiPoint3 hRayEnd = a_pos + a_offset + (a_vel * a_predictionTime);
    hRayEnd.z = hRayStart.z; 

    RE::hkpWorldRayCastOutput hRayOutput;
    if (PerformRayCast(a_player, hRayStart, hRayEnd, hRayOutput)) {
      if (a_global)
        a_global->value = 0.0f;
      return 0.0f;
    }
  }

  RE::NiPoint3 rayStart = a_pos + a_offset + (a_vel * a_predictionTime);
  rayStart.z += 150.0f; // Increased from 100 for better terrain clearance

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

    if (a_outHitInfo) {
      a_outHitInfo->hasHit = true;
      a_outHitInfo->normalZ = rayOutput.normal.quad.m128_f32[2];
      if (rayOutput.rootCollidable) {
        a_outHitInfo->layer = rayOutput.rootCollidable->GetCollisionLayer();
        if (auto *hkShape = rayOutput.rootCollidable->GetShape()) {
          if (auto *bhkShape = hkShape->userData) {
            a_outHitInfo->materialID = bhkShape->materialID;
          }
        }
      }
    }
  }

  float diff = std::round(a_pos.z - terrainHeight);

  // Clamp excessive values.
  if (diff > CAP_HEIGHT) {
    diff = CAP_HEIGHT;
  } else if (diff < 0.0f) {
    if (a_allowNegativeDiff) {
      // Slope calculations: keep negative diff so atan2 produces correct uphill angle.
      if (!rayOutput.HasHit()) diff = CAP_HEIGHT;
    } else {
      // All other sensors: clamp to 0 (terrain higher than player = flat ground for this sensor).
      diff = rayOutput.HasHit() ? 0.0f : CAP_HEIGHT;
    }
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

void RaySenseLogic::ClearSensors() {
  // Restore fJumpHeightMin if it was boosted before sensor state is wiped.
  // Prevents the value from being stuck boosted during mount/furniture/swim states.
  if (_jumpBoosted) {
    if (auto *gmst = RE::GameSettingCollection::GetSingleton()) {
      if (auto *setting = gmst->GetSetting("fJumpHeightMin")) {
        setting->data.f = _origJumpHeight;
      }
    }
    _jumpBoosted = false;
  }

  _frontDiff = 0.0f;
  _leftDiff = 0.0f;
  _rightDiff = 0.0f;
  _obstacleVaultDist = 0.0f;
  _wallFrontDist = 0.0f;
  _wallFrontLDist = 0.0f;
  _wallFrontRDist = 0.0f;
  _wallLeftDist = 0.0f;
  _wallRightDist = 0.0f;
  _playerHeight = 0.0f;
  _surfaceType = 0.0f;
  _platformType = 0.0f;
  _obstacleTypeFront = 0;
  _obstacleTypeLeft = 0;
  _obstacleTypeRight = 0;

  _surfaceSlope = 0.0f;
  _macroFrontSlope = 0.0f;
  _macroLeftSlope = 0.0f;
  _macroRightSlope = 0.0f;

  if (_verticalityFrontGlobal)
    _verticalityFrontGlobal->value = 0.0f;
  if (_verticalityLeftGlobal)
    _verticalityLeftGlobal->value = 0.0f;
  if (_verticalityRightGlobal)
    _verticalityRightGlobal->value = 0.0f;
  if (_verticalityObstacleGlobal)
    _verticalityObstacleGlobal->value = 0.0f;
  if (_wallFrontGlobal)
    _wallFrontGlobal->value = 0.0f;
  if (_wallFrontLGlobal)
    _wallFrontLGlobal->value = 0.0f;
  if (_wallFrontRGlobal)
    _wallFrontRGlobal->value = 0.0f;
  if (_wallLeftGlobal)
    _wallLeftGlobal->value = 0.0f;
  if (_wallRightGlobal)
    _wallRightGlobal->value = 0.0f;
  if (_obstacleTypeFrontGlobal)
    _obstacleTypeFrontGlobal->value = 0.0f;
  if (_obstacleTypeLeftGlobal)
    _obstacleTypeLeftGlobal->value = 0.0f;
  if (_obstacleTypeRightGlobal)
    _obstacleTypeRightGlobal->value = 0.0f;
  if (_verticalityPlayerGlobal)
    _verticalityPlayerGlobal->value = 0.0f;
  if (_surfaceTypeGlobal)
    _surfaceTypeGlobal->value = 0.0f;
  if (_platformTypeGlobal)
    _platformTypeGlobal->value = 0.0f;
}

void RaySenseLogic::UpdateSurfaceInfo(RE::PlayerCharacter *a_player) {
  if (!a_player)
    return;

  SurfaceType surfaceType = SurfaceType::kDefault;
  PlatformType platformType = PlatformType::kNone;

  RE::NiPoint3 pos = a_player->GetPosition();
  RE::MATERIAL_ID mID = RE::MATERIAL_ID::kNone;
  RE::COL_LAYER layer = RE::COL_LAYER::kUnidentified;

  // 1. Highest priority: Check IsSwimming() first
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

      if ((layer == RE::COL_LAYER::kTerrain || layer == RE::COL_LAYER::kGround) &&
          a_player->GetParentCell() && !a_player->GetParentCell()->IsInteriorCell()) {
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
    // bhkCharacterController internal Havok struct field - SE/AE layout
    // identical
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
    bool changed = (soundMID != lastSoundMID || raycastMID != lastRayMID);
    lastSoundMID = soundMID;
    lastRayMID = raycastMID;
    if (changed && Settings::GetSingleton()->enableLog) {
      SKSE::log::info("DEBUG | RayMID: {} | SoundMID: {} | Layer: {}",
                      static_cast<std::uint32_t>(raycastMID),
                      static_cast<std::uint32_t>(soundMID),
                      static_cast<std::uint32_t>(layer));
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
