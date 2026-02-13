#include "ShadowsLogic.h"
#include "RE/B/BGSMaterialType.h"
#include "RE/B/bhkWorld.h"
#include "RE/H/hkpWorld.h"
#include "RE/H/hkpWorldRayCastInput.h"
#include "RE/H/hkpWorldRayCastOutput.h"
#include "RE/T/TESHavokUtilities.h"
#include "RE/T/TESModel.h"
#include "RE/T/TESObjectCELL.h"
#include "RE/T/TESObjectREFR.h"
#include <algorithm>
#include <cmath>
#include <string>

// --- Material ID 상수 ---
namespace MaterialID {
constexpr RE::FormID Snow = 0x00012F45;
constexpr RE::FormID SnowStairs = 0x00052ED0;
constexpr RE::FormID Ice = 0x00012F47;
constexpr RE::FormID IceForm = 0x000D9B1E;
constexpr RE::FormID Glacier = 0x0006A0E2;
constexpr RE::FormID IceBroken = 0x00012F4C;
constexpr RE::FormID Water = 0x00012F40;
constexpr RE::FormID WaterPuddle = 0x000D6C11;
} // namespace MaterialID

// 문자열 소문자 변환 헬퍼
std::string ToLower(const std::string &str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

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
  _verticalityObstacleTypeGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Obstacle_Type");
  _verticalityPlayerGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Player");
  _verticalityTypeGlobal =
      RE::TESForm::LookupByEditorID<RE::TESGlobal>("Verticality_Type");
  SKSE::log::info("ShadowsLogic: Installation Complete.");
}

void ShadowsLogic::OnJump(RE::PlayerCharacter *a_player) {
  if (!a_player)
    return;
  _obstacleDist = UpdateObstacleDetection(a_player);
  RE::NiPoint3 pos = a_player->GetPosition();
  RE::NiPoint3 vel;
  a_player->GetLinearVelocity(vel);
  _frontDiff = UpdateVerticality(_verticalityFrontGlobal, a_player, pos,
                                 RE::NiPoint3(0, 0, 0), vel, 0.5f);
}

void ShadowsLogic::OnUpdate(RE::PlayerCharacter *a_player, float a_delta) {
  if (!a_player || a_delta <= 0.0f)
    return;
  RE::NiPoint3 pos = a_player->GetPosition();
  if (!_initialized) {
    _lastPosition = pos;
    _initialized = true;
    return;
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

  _obstacleDist = UpdateObstacleDetection(a_player);
  _playerHeight =
      UpdateVerticality(_verticalityPlayerGlobal, a_player, pos,
                        RE::NiPoint3(0, 0, 0), RE::NiPoint3(0, 0, 0), 0.0f);

  if (a_player->IsInMidair()) {
    RE::NiPoint3 vel = (pos - _lastPosition) / a_delta;
    _frontDiff = UpdateVerticality(_verticalityFrontGlobal, a_player, pos,
                                   RE::NiPoint3(0, 0, 0), vel, 0.5f);
  } else {
    _frontDiff =
        UpdateVerticality(_verticalityFrontGlobal, a_player, pos,
                          forward * 80.0f, RE::NiPoint3(0, 0, 0), 0.0f);
  }
  _leftDiff = UpdateVerticality(_verticalityLeftGlobal, a_player, pos,
                                left * 60.0f, RE::NiPoint3(0, 0, 0), 0.0f);
  _rightDiff = UpdateVerticality(_verticalityRightGlobal, a_player, pos,
                                 right * 60.0f, RE::NiPoint3(0, 0, 0), 0.0f);
  _lastPosition = pos;
}

// [헬퍼] 정확한 물 높이 가져오기
float ShadowsLogic::GetCellWaterHeight(RE::TESObjectCELL *a_cell,
                                       const RE::NiPoint3 &a_pos) {
  if (!a_cell)
    return -100000.0f;
  float waterHeight = -100000.0f;

  // 1. 특정 좌표의 물 높이 확인
  if (a_cell->GetWaterHeight(a_pos, waterHeight))
    return waterHeight;

  // 2. 외부(Exterior) 셀의 전역 물 높이 확인
  if (a_cell->IsExteriorCell()) {
    waterHeight = a_cell->GetExteriorWaterHeight();
    if (waterHeight > -10000.0f)
      return waterHeight;
  }
  return -100000.0f;
}

// =========================================================================================
// [핵심 로직] 물, 눈, 얼음을 완벽하게 구분하는 최종 판별 함수
// =========================================================================================
float ShadowsLogic::ResolveObjectType(
    const RE::hkpWorldRayCastOutput &a_rayOutput, const RE::NiPoint3 &a_hitPos,
    RE::TESObjectCELL *a_cell) {
  auto *colObj = a_rayOutput.rootCollidable;
  if (!colObj)
    return 0.0f;

  // -------------------------------------------------------------------------
  // 1. [최우선] 물(Water) 판정 (지형 투과 문제 해결)
  // 레이캐스트가 물을 뚫고 바닥 흙을 찍었더라도, 히트 지점이 수면 아래면 물로
  // 간주
  // -------------------------------------------------------------------------
  float waterHeight = GetCellWaterHeight(a_cell, a_hitPos);
  if (waterHeight > -10000.0f) {
    // 히트 지점이 수면보다 아래에 있거나, 수면과 매우 가까우면(오차 15유닛) 물
    if (a_hitPos.z < waterHeight + 15.0f) {
      return 2.0f; // Water
    }
  }

  // 2. [물리 재질] Collision Filter에서 ID 추출
  uint32_t filterInfo = colObj->broadPhaseHandle.collisionFilterInfo;
  uint32_t materialID_8bit = (filterInfo >> 16) & 0xFF;
  auto *materialType = RE::BGSMaterialType::GetMaterialType(
      static_cast<RE::MATERIAL_ID>(materialID_8bit));
  RE::FormID matFormID = materialType ? materialType->GetFormID() : 0;

  // 3. [ID 판정] 확실한 특수 재질
  if (matFormID == MaterialID::Water || matFormID == MaterialID::WaterPuddle)
    return 2.0f;
  if (matFormID == MaterialID::Snow || matFormID == MaterialID::SnowStairs)
    return 3.0f;
  if (matFormID == MaterialID::Ice || matFormID == MaterialID::Glacier ||
      matFormID == MaterialID::IceForm || matFormID == MaterialID::IceBroken)
    return 4.0f;

  // 4. [이름 판정] ID가 다르지만 이름이 Snow/Ice인 경우 (모드 호환성)
  if (materialType) {
    std::string matName = ToLower(materialType->materialName.c_str());
    if (matName.find("snow") != std::string::npos)
      return 3.0f; // Name contains "snow"
    if (matName.find("ice") != std::string::npos ||
        matName.find("glacier") != std::string::npos)
      return 4.0f;
  }

  // 5. [시각 판정] 모델 파일명 확인 (돌 재질로 설정된 빙하/눈 바위 구제)
  auto *refr = RE::TESHavokUtilities::FindCollidableRef(*colObj);
  if (refr) {
    if (refr->GetBaseObject()->GetFormType() == RE::FormType::Furniture)
      return 5.0f;

    auto *base = refr->GetBaseObject();
    auto *modelBase = base ? base->As<RE::TESModel>() : nullptr;
    if (modelBase) {
      std::string path = ToLower(modelBase->GetModel());
      // 파일명에 얼음 관련 단어
      if (path.find("glacier") != std::string::npos ||
          path.find("ice") != std::string::npos ||
          path.find("frozen") != std::string::npos)
        return 4.0f;
      // 파일명에 눈 관련 단어
      if (path.find("snow") != std::string::npos)
        return 3.0f;
    }
  }

  // 6. [기본값] 걸러지지 않은 나머지는 일반 고체(돌, 흙 등)
  return 1.0f;
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
  float detectDist = isSprinting ? 330.0f : 230.0f;
  float scale = RE::bhkWorld::GetWorldScale();
  auto *parentCell = a_player->GetParentCell();

  // 무릎 높이 레이캐스트
  RE::NiPoint3 rayStart = pos;
  rayStart.z += 40.0f;
  RE::NiPoint3 rayEnd = rayStart + (forward * detectDist);
  RE::hkpWorldRayCastInput rayInput;
  rayInput.from = {rayStart.x * scale, rayStart.y * scale, rayStart.z * scale,
                   0.0f};
  rayInput.to = {rayEnd.x * scale, rayEnd.y * scale, rayEnd.z * scale, 0.0f};
  uint32_t filter = 0;
  a_player->GetCollisionFilterInfo(filter);
  rayInput.filterInfo = filter;

  RE::hkpWorldRayCastOutput kneeOutput;
  float kneeDist = 0.0f;
  bool kneeHit = false;

  if (parentCell && parentCell->GetbhkWorld()) {
    parentCell->GetbhkWorld()->GetWorld1()->CastRay(rayInput, kneeOutput);
    if (kneeOutput.HasHit() && kneeOutput.normal.quad.m128_f32[2] <= 0.5f) {
      kneeDist = kneeOutput.hitFraction * detectDist;
      kneeHit = true;
    }
  }

  // 가슴 높이 (통과 여부)
  RE::NiPoint3 chestStart = pos;
  chestStart.z += 120.0f;
  RE::NiPoint3 chestEnd = chestStart + (forward * detectDist);
  rayInput.from = {chestStart.x * scale, chestStart.y * scale,
                   chestStart.z * scale, 0.0f};
  rayInput.to = {chestEnd.x * scale, chestEnd.y * scale, chestEnd.z * scale,
                 0.0f};
  RE::hkpWorldRayCastOutput chestOutput;
  bool chestHit = false;
  if (parentCell && parentCell->GetbhkWorld()) {
    parentCell->GetbhkWorld()->GetWorld1()->CastRay(rayInput, chestOutput);
    if (chestOutput.HasHit() && chestOutput.normal.quad.m128_f32[2] <= 0.5f)
      chestHit = true;
  }

  float finalDist = (kneeHit && !chestHit) ? std::round(kneeDist) : 0.0f;

  // [중요] 장애물 타입 판별 시 parentCell 넘겨주기 (물 높이 계산용)
  if (kneeHit && !chestHit) {
    RE::NiPoint3 hitPos =
        rayStart + (rayEnd - rayStart) * kneeOutput.hitFraction;
    _obstacleType = ResolveObjectType(kneeOutput, hitPos, parentCell);
  } else {
    _obstacleType = 0.0f;
  }

  if (_verticalityObstacleGlobal)
    _verticalityObstacleGlobal->value = finalDist;
  if (_verticalityObstacleTypeGlobal)
    _verticalityObstacleTypeGlobal->value = _obstacleType;
  return finalDist;
}

float ShadowsLogic::UpdateVerticality(RE::TESGlobal *a_global,
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

  float scale = RE::bhkWorld::GetWorldScale();
  RE::hkpWorldRayCastInput rayInput;
  rayInput.from = {rayStart.x * scale, rayStart.y * scale, rayStart.z * scale,
                   0.0f};
  rayInput.to = {rayEnd.x * scale, rayEnd.y * scale, rayEnd.z * scale, 0.0f};
  uint32_t filter = 0;
  a_player->GetCollisionFilterInfo(filter);
  rayInput.filterInfo = filter;

  RE::hkpWorldRayCastOutput rayOutput;
  float terrainHeight = a_pos.z - CAP_HEIGHT;
  auto *parentCell = a_player->GetParentCell();

  if (parentCell && parentCell->GetbhkWorld()) {
    parentCell->GetbhkWorld()->GetWorld1()->CastRay(rayInput, rayOutput);
    if (rayOutput.HasHit()) {
      RE::NiPoint3 hitPos =
          rayStart + (rayEnd - rayStart) * rayOutput.hitFraction;
      terrainHeight = hitPos.z;
      // [중요] 바닥 타입 판별 시 parentCell 넘겨주기
      _verticalityType = ResolveObjectType(rayOutput, hitPos, parentCell);
    } else {
      _verticalityType = 0.0f;
    }
  }

  if (_verticalityTypeGlobal)
    _verticalityTypeGlobal->value = _verticalityType;
  float diff = std::round(a_pos.z - terrainHeight);
  if (diff > CAP_HEIGHT)
    diff = CAP_HEIGHT;
  else if (diff < 0.0f && !rayOutput.HasHit())
    diff = CAP_HEIGHT;

  if (a_global)
    a_global->value = diff;
  return diff;
}