#include "ShadowsLogic.h"
#include "RE/B/bhkWorld.h"
#include "RE/H/hkpWorld.h"
#include "RE/H/hkpWorldRayCastInput.h"
#include "RE/H/hkpWorldRayCastOutput.h"
#include "RE/T/TESObjectCELL.h"
#include <cmath>

void ShadowsLogic::Install() {
  SKSE::log::info("ShadowsLogic: Starting Installation...");

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

  if (_verticalityFrontGlobal) {
    SKSE::log::info("ShadowsLogic: Found Global Verticality_Front");
  } else {
    SKSE::log::error("ShadowsLogic: Failed to find Global Verticality_Front");
  }

  if (_verticalityLeftGlobal)
    SKSE::log::info("ShadowsLogic: Found Global Verticality_Left");
  if (_verticalityRightGlobal)
    SKSE::log::info("ShadowsLogic: Found Global Verticality_Right");
  if (_verticalityObstacleGlobal)
    SKSE::log::info("ShadowsLogic: Found Global Verticality_Obstacle");
  if (_verticalityPlayerGlobal)
    SKSE::log::info("ShadowsLogic: Found Global Verticality_Player");

  SKSE::log::info("ShadowsLogic: Installation Complete.");
}

void ShadowsLogic::OnJump(RE::PlayerCharacter *a_player) {
  if (!a_player)
    return;

  // Immediate update to ensure OAR/Hooks can see the value
  _obstacleDist = UpdateObstacleDetection(a_player);

  RE::NiPoint3 pos = a_player->GetPosition();
  RE::NiPoint3 vel;
  a_player->GetLinearVelocity(vel);

  // Get direction from 3D data if available
  RE::NiPoint3 forward(0, 1, 0);
  if (auto root = a_player->Get3D()) {
    auto &m = root->world.rotate;
    forward = {m.entry[0][1], m.entry[1][1], m.entry[2][1]};
  }

  // Immediate update on jump signal for front prediction
  _frontDiff = UpdateVerticality(_verticalityFrontGlobal, a_player, pos,
                                 RE::NiPoint3(0.0f, 0.0f, 0.0f), vel, 0.5f);
}

void ShadowsLogic::OnUpdate(RE::PlayerCharacter *a_player, float a_delta) {
  if (!a_player || a_delta <= 0.0f)
    return;

  RE::NiPoint3 pos = a_player->GetPosition();

  // Initialize last position on first frame to prevent velocity spike
  if (!_initialized) {
    _lastPosition = pos;
    _initialized = true;
    return;
  }

  // Determine directions using the player's actual 3D orientation
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

  // 2. Player Height Above Ground (Always relative to player's current pos)
  _playerHeight = UpdateVerticality(_verticalityPlayerGlobal, a_player, pos,
                                    RE::NiPoint3(0.0f, 0.0f, 0.0f),
                                    RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f);

  // 3. Front Check
  if (a_player->IsInMidair()) {
    // Mid-air: Prediction purely based on current velocity
    RE::NiPoint3 vel = (pos - _lastPosition) / a_delta;
    _frontDiff = UpdateVerticality(_verticalityFrontGlobal, a_player, pos,
                                   RE::NiPoint3(0.0f, 0.0f, 0.0f), vel, 0.5f);
  } else {
    // Grounded: Check 80 units ahead
    _frontDiff = UpdateVerticality(_verticalityFrontGlobal, a_player, pos,
                                   forward * 80.0f,
                                   RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f);
  }

  // 4. Left/Right Check (60 units sideways)
  _leftDiff =
      UpdateVerticality(_verticalityLeftGlobal, a_player, pos, left * 60.0f,
                        RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f);
  _rightDiff =
      UpdateVerticality(_verticalityRightGlobal, a_player, pos, right * 60.0f,
                        RE::NiPoint3(0.0f, 0.0f, 0.0f), 0.0f);

  _lastPosition = pos;
}

float ShadowsLogic::UpdateObstacleDetection(RE::PlayerCharacter *a_player) {
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

    float scale = RE::bhkWorld::GetWorldScale();
    RE::hkpWorldRayCastInput rayInput;
    rayInput.from = RE::hkVector4(rayStart.x * scale, rayStart.y * scale,
                                  rayStart.z * scale, 0.0f);
    rayInput.to = RE::hkVector4(rayEnd.x * scale, rayEnd.y * scale,
                                rayEnd.z * scale, 0.0f);

    std::uint32_t collisionFilter = 0;
    a_player->GetCollisionFilterInfo(collisionFilter);
    rayInput.filterInfo = collisionFilter;

    RE::hkpWorldRayCastOutput rayOutput;
    auto *parentCell = a_player->GetParentCell();
    if (parentCell) {
      auto *bhkWorld_ = parentCell->GetbhkWorld();
      if (bhkWorld_) {
        auto *hkpWorld_ = bhkWorld_->GetWorld1();
        if (hkpWorld_) {
          hkpWorld_->CastRay(rayInput, rayOutput);
          if (rayOutput.HasHit()) {
            // Filter out slopes: If surface normal Z > 0.5, it's a floor/slope,
            // not a wall.
            if (rayOutput.normal.quad.m128_f32[2] > 0.5f) {
              return false;
            }
            a_dist = rayOutput.hitFraction * detectDistance;
            return true;
          }
        }
      }
    }
    return false;
  };

  float kneeDist = 0.0f;
  float dummyDist = 0.0f;
  bool kneeHit = CastHorizontalRay(40.0f, kneeDist);    // Knee height
  bool chestHit = CastHorizontalRay(120.0f, dummyDist); // Chest height

  float finalDist = (kneeHit && !chestHit) ? std::round(kneeDist) : 0.0f;
  if (_verticalityObstacleGlobal) {
    _verticalityObstacleGlobal->value = finalDist;
  }
  return finalDist;
}

bool ShadowsLogic::IsObstacleDetected() const { return _obstacleDist > 0.0f; }

float ShadowsLogic::UpdateVerticality(RE::TESGlobal *a_global,
                                      RE::PlayerCharacter *a_player,
                                      const RE::NiPoint3 &a_pos,
                                      const RE::NiPoint3 &a_offset,
                                      const RE::NiPoint3 &a_vel,
                                      float a_predictionTime) {
  if (!a_player)
    return 0.0f;

  // Final measurement position = Player Position + Static Offset + Velocity
  // Prediction
  RE::NiPoint3 rayStart = a_pos + a_offset + (a_vel * a_predictionTime);
  rayStart.z += 100.0f; // Head height Ray
  RE::NiPoint3 rayEnd = rayStart;
  rayEnd.z -= (CAP_HEIGHT + 1000.0f);

  std::uint32_t collisionFilter = 0;
  a_player->GetCollisionFilterInfo(collisionFilter);

  float scale = RE::bhkWorld::GetWorldScale();
  RE::hkpWorldRayCastInput rayInput;
  rayInput.from = RE::hkVector4(rayStart.x * scale, rayStart.y * scale,
                                rayStart.z * scale, 0.0f);
  rayInput.to =
      RE::hkVector4(rayEnd.x * scale, rayEnd.y * scale, rayEnd.z * scale, 0.0f);
  rayInput.filterInfo = collisionFilter;

  RE::hkpWorldRayCastOutput rayOutput;
  float terrainHeight = a_pos.z;

  auto *parentCell = a_player->GetParentCell();
  if (parentCell) {
    auto *bhkWorld_ = parentCell->GetbhkWorld();
    if (bhkWorld_) {
      auto *hkpWorld_ = bhkWorld_->GetWorld1();
      if (hkpWorld_) {
        hkpWorld_->CastRay(rayInput, rayOutput);
        if (rayOutput.HasHit()) {
          terrainHeight =
              rayStart.z + (rayEnd.z - rayStart.z) * rayOutput.hitFraction;
        }
      }
    }
  }

  float diff = std::round(a_pos.z - terrainHeight);
  if (diff > CAP_HEIGHT)
    diff = CAP_HEIGHT;

  if (a_global) {
    a_global->value = diff;
  }

  return diff;
}
