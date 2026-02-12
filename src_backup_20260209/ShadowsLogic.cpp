#include "ShadowsLogic.h"
#include "RE/B/bhkWorld.h"
#include "RE/H/hkpWorld.h"
#include "RE/H/hkpWorldRayCastInput.h"
#include "RE/H/hkpWorldRayCastOutput.h"
#include "RE/T/TESObjectCELL.h"
#include <cmath>

void ShadowsLogic::Install() {
  _verticalityFrontGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Front");
  _verticalityLeftGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Left");
  _verticalityRightGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Right");
  _verticalityObstacleGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Obstacle");

  if (_verticalityFrontGlobal) {
    SKSE::log::info("ShadowsLogic: Found Global Verticality_Front");
  } else {
    SKSE::log::error("ShadowsLogic: Failed to find Global Verticality_Front");
  }
}

void ShadowsLogic::OnJump(RE::PlayerCharacter *a_player) {
  if (!a_player || !_verticalityFrontGlobal)
    return;

  // Immediate obstacle detection to ensure Jump hook can see the value
  UpdateObstacleDetection(a_player);

  RE::NiPoint3 pos = a_player->GetPosition();
  RE::NiPoint3 vel;
  a_player->GetLinearVelocity(vel);

  // Immediate update on jump signal for front
  UpdateVerticality(_verticalityFrontGlobal, a_player, pos, vel, 0.5f);
}

void ShadowsLogic::OnUpdate(RE::PlayerCharacter *a_player, float a_delta) {
  if (!a_player || !_verticalityFrontGlobal || a_delta <= 0.0f)
    return;

  RE::NiPoint3 pos = a_player->GetPosition();

  // Initialize last position on first frame to prevent velocity spike
  if (!_initialized) {
    _lastPosition = pos;
    _initialized = true;
    return;
  }

  // Common Setup
  float angle = a_player->data.angle.z;
  float sinA = std::sin(angle);
  float cosA = std::cos(angle);
  RE::NiPoint3 forward(sinA, cosA, 0.0f);

  // Obstacle Detection
  UpdateObstacleDetection(a_player);

  if (a_player->IsInMidair()) {
    // Mid-air: Use manual velocity delta for prediction for front
    RE::NiPoint3 vel = (pos - _lastPosition) / a_delta;
    UpdateVerticality(_verticalityFrontGlobal, a_player, pos, vel, 0.5f);
  } else {
    // Grounded: Front check 80 units ahead
    UpdateVerticality(_verticalityFrontGlobal, a_player, pos, forward * 80.0f,
                      1.0f);
  }

  // Left/Right check 50 units sideways (regardless of mid-air or not)
  if (_verticalityLeftGlobal || _verticalityRightGlobal) {
    if (_verticalityLeftGlobal) {
      RE::NiPoint3 leftDir(-cosA, sinA, 0.0f);
      UpdateVerticality(_verticalityLeftGlobal, a_player, pos, leftDir * 50.0f,
                        1.0f);
    }
    if (_verticalityRightGlobal) {
      RE::NiPoint3 rightDir(cosA, -sinA, 0.0f);
      UpdateVerticality(_verticalityRightGlobal, a_player, pos,
                        rightDir * 50.0f, 1.0f);
    }
  }

  _lastPosition = pos;
}

void ShadowsLogic::UpdateObstacleDetection(RE::PlayerCharacter *a_player) {
  if (!_verticalityObstacleGlobal || !a_player)
    return;

  RE::NiPoint3 pos = a_player->GetPosition();
  float angle = a_player->data.angle.z;
  float sinA = std::sin(angle);
  float cosA = std::cos(angle);
  RE::NiPoint3 forward(sinA, cosA, 0.0f);

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
  _verticalityObstacleGlobal->value = (kneeHit && !chestHit) ? kneeDist : 0.0f;
}

bool ShadowsLogic::IsObstacleDetected() const {
  return _verticalityObstacleGlobal && _verticalityObstacleGlobal->value > 0.0f;
}

void ShadowsLogic::UpdateVerticality(RE::TESGlobal *a_global,
                                     RE::PlayerCharacter *a_player,
                                     const RE::NiPoint3 &a_pos,
                                     const RE::NiPoint3 &a_vel,
                                     float a_predictionTime) {
  if (!a_global || !a_player)
    return;

  RE::NiPoint3 forward;
  float speedSq = a_vel.x * a_vel.x + a_vel.y * a_vel.y;

  if (speedSq > 0.01f) {
    float invSpeed = 1.0f / std::sqrt(speedSq);
    forward = {a_vel.x * invSpeed, a_vel.y * invSpeed, 0.0f};
  } else {
    float angle = a_player->data.angle.z;
    forward = {std::sin(angle), std::cos(angle), 0.0f};
  }

  RE::NiPoint3 rayStart =
      a_pos + (forward * (std::sqrt(speedSq) * a_predictionTime));
  rayStart.z += 100.0f;
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
  float terrainHeight = a_pos.z - (CAP_HEIGHT + 1000.0f); // Default to bottom

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

  float diff = a_pos.z - terrainHeight;
  if (diff > CAP_HEIGHT)
    diff = CAP_HEIGHT;
  if (diff < 0.0f)
    diff = 0.0f;

  a_global->value = diff;

  // Debug logging disabled for performance
  // SKSE::log::info("UpdateVerticality: Global={:s}, TargetDiff={:.2f}",
  //                 a_global->GetFormEditorID(), diff);
}
