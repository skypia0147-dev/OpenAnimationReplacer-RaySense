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
  _verticalityPlayerGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Player");

  if (_verticalityFrontGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Verticality_Front");
  if (_verticalityLeftGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Verticality_Left");
  if (_verticalityRightGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Verticality_Right");
  if (_verticalityObstacleGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Verticality_Obstacle");
  if (_verticalityPlayerGlobal)
    SKSE::log::info("RaySenseLogic: Found Global Verticality_Player");

  SKSE::log::info("RaySenseLogic: Installation Complete.");
}

void RaySenseLogic::OnJump(RE::PlayerCharacter *a_player) {
  if (!a_player)
    return;

  // Immediate update to ensure OAR/Hooks can see the value
  _obstacleDist = UpdateObstacleDetection(a_player);

  RE::NiPoint3 pos = a_player->GetPosition();
  RE::NiPoint3 vel;
  a_player->GetLinearVelocity(vel);

  // Immediate update on jump signal for front prediction
  _frontDiff = UpdateVerticality(_verticalityFrontGlobal, a_player, pos,
                                 RE::NiPoint3(0.0f, 0.0f, 0.0f), vel, 0.5f);
}

void RaySenseLogic::OnUpdate(RE::PlayerCharacter *a_player, float a_delta) {
  if (!a_player || a_delta <= 0.0f)
    return;

  RE::NiPoint3 currentPos = a_player->GetPosition();
  float currentAngle = a_player->data.angle.z;

  // [Smart Caching]
  // Skip heavy calculations if player is stationary
  if (_initialized) {
    float distSq = currentPos.GetSquaredDistance(_lastUpdatePos);
    float angleDiff = std::abs(currentAngle - _lastUpdateAngle);

    if (distSq < 1.0f && angleDiff < 0.01f) {
      // Must continue updating if in mid-air to track landing prediction
      if (!a_player->IsInMidair()) {
        return;
      }
    }
  }

  RE::NiPoint3 forward(0, 1, 0);
  RE::NiPoint3 left(-1, 0, 0);
  RE::NiPoint3 right(1, 0, 0);

  if (auto root = a_player->Get3D()) {
    auto &m = root->world.rotate;
    forward = {m.entry[0][1], m.entry[1][1], m.entry[2][1]};
    right = {m.entry[0][0], m.entry[1][0], m.entry[2][0]};
    left = right * -1.0f;
  }

  // 1. Obstacle Detection
  _obstacleDist = UpdateObstacleDetection(a_player);
  _frontObstacleType = UpdateFrontObstacleType(a_player);

  // 2. Player Height Above Ground
  _playerHeight = UpdateVerticality(_verticalityPlayerGlobal, a_player,
                                    currentPos, RE::NiPoint3(0.0f, 0.0f, 0.0f),
                                    RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f);

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
                          RE::NiPoint3(0.0f, 0.0f, 0.0f), vel, 0.5f);
  } else {
    // Grounded: Check 80 units ahead
    _frontDiff = UpdateVerticality(_verticalityFrontGlobal, a_player,
                                   currentPos, forward * 80.0f,
                                   RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f);
  }

  // 4. Left/Right Check (60 units sideways)
  _leftDiff =
      UpdateVerticality(_verticalityLeftGlobal, a_player, currentPos,
                        left * 60.0f, RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f);
  _rightDiff =
      UpdateVerticality(_verticalityRightGlobal, a_player, currentPos,
                        right * 60.0f, RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f);

  _lastUpdatePos = currentPos;
  _lastUpdateAngle = currentAngle;
  _initialized = true;
}

float RaySenseLogic::UpdateObstacleDetection(RE::PlayerCharacter *a_player) {
  if (!a_player)
    return 0.0f;

  RE::NiPoint3 pos = a_player->GetPosition();
  RE::NiPoint3 forward(0, 1, 0);
  if (auto root = a_player->Get3D()) {
    auto &m = root->world.rotate;
    forward = {m.entry[0][1], m.entry[1][1], m.entry[2][1]};
  }

  bool isSprinting = a_player->AsActorState()->IsSprinting();
  float detectDistance = isSprinting ? 330.0f : 230.0f;

  auto CastHorizontalRay = [&](float a_height, float &a_dist) -> bool {
    RE::NiPoint3 rayStart = pos;
    rayStart.z += a_height;
    RE::NiPoint3 rayEnd = rayStart + (forward * detectDistance);

    RE::hkpWorldRayCastOutput rayOutput;
    if (PerformRayCast(a_player, rayStart, rayEnd, rayOutput)) {
      // Filter out slopes: If surface normal Z > 0.5 (approx < 60 deg slope),
      // ignore it.
      if (rayOutput.normal.quad.m128_f32[2] > 0.5f) {
        return false;
      }
      a_dist = rayOutput.hitFraction * detectDistance;
      return true;
    }
    return false;
  };

  float kneeDist = 0.0f;
  float dummyDist = 0.0f;
  bool kneeHit = CastHorizontalRay(40.0f, kneeDist);    // Knee height
  bool chestHit = CastHorizontalRay(120.0f, dummyDist); // Chest height

  // Valid obstacle = Knee blocked but Chest clear (Vaultable)
  float finalDist = (kneeHit && !chestHit) ? std::round(kneeDist) : 0.0f;

  if (_verticalityObstacleGlobal) {
    _verticalityObstacleGlobal->value = finalDist;
  }
  return finalDist;
}

std::uint32_t
RaySenseLogic::UpdateFrontObstacleType(RE::PlayerCharacter *a_player) {
  if (!a_player)
    return 0;

  RE::NiPoint3 pos = a_player->GetPosition();
  RE::NiPoint3 forward(0, 1, 0);
  if (auto root = a_player->Get3D()) {
    auto &m = root->world.rotate;
    forward = {m.entry[0][1], m.entry[1][1], m.entry[2][1]};
  }

  float detectDistance = 250.0f;
  RE::NiPoint3 rayStart = pos;
  rayStart.z += 100.0f; // Eye/Chest height
  RE::NiPoint3 rayEnd = rayStart + (forward * detectDistance);

  RE::hkpWorldRayCastOutput rayOutput;
  if (PerformRayCast(a_player, rayStart, rayEnd, rayOutput)) {
    auto *ref =
        RE::TESHavokUtilities::FindCollidableRef(*rayOutput.rootCollidable);
    if (ref) {
      auto *base = ref->GetBaseObject();
      if (base) {
        return static_cast<std::uint32_t>(base->GetFormType());
      }
    }
  }
  return 0; // kNone
}

bool RaySenseLogic::IsObstacleDetected() const { return _obstacleDist > 0.0f; }

float RaySenseLogic::UpdateVerticality(RE::TESGlobal *a_global,
                                       RE::PlayerCharacter *a_player,
                                       const RE::NiPoint3 &a_pos,
                                       const RE::NiPoint3 &a_offset,
                                       const RE::NiPoint3 &a_vel,
                                       float a_predictionTime) {
  if (!a_player)
    return 0.0f;

  RE::NiPoint3 rayStart = a_pos + a_offset + (a_vel * a_predictionTime);
  rayStart.z += 100.0f;
  RE::NiPoint3 rayEnd = rayStart;
  rayEnd.z -= (CAP_HEIGHT + 1000.0f);

  RE::hkpWorldRayCastOutput rayOutput;
  float terrainHeight = a_pos.z - CAP_HEIGHT; // Default to "far below"

  if (PerformRayCast(a_player, rayStart, rayEnd, rayOutput)) {
    terrainHeight =
        rayStart.z + (rayEnd.z - rayStart.z) * rayOutput.hitFraction;
  }

  float diff = std::round(a_pos.z - terrainHeight);

  // Clamp excessive values
  if (diff > CAP_HEIGHT)
    diff = CAP_HEIGHT;
  else if (diff < 0.0f && !rayOutput.HasHit())
    diff = CAP_HEIGHT;

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

  // Apply player's collision filter to ignore self/phantom objects
  std::uint32_t collisionFilter = 0;
  a_player->GetCollisionFilterInfo(collisionFilter);
  rayInput.filterInfo = collisionFilter;

  hkpWorld_->CastRay(rayInput, a_output);
  return a_output.HasHit();
}
