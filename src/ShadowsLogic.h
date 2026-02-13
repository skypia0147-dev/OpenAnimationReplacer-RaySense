#pragma once

#include "PCH.h"

class ShadowsLogic {
public:
  static ShadowsLogic *GetSingleton() {
    static ShadowsLogic singleton;
    return &singleton;
  }

  void Install();
  void OnUpdate(RE::PlayerCharacter *a_player, float a_delta);
  void OnJump(RE::PlayerCharacter *a_player);

  bool IsObstacleDetected() const { return _obstacleDist > 0.0f; }

  // OAR/Condition용 Getter
  float GetFrontDiff() const { return _frontDiff; }
  float GetLeftDiff() const { return _leftDiff; }
  float GetRightDiff() const { return _rightDiff; }
  float GetObstacleDist() const { return _obstacleDist; }
  float GetObstacleType() const { return _obstacleType; }
  float GetPlayerHeight() const { return _playerHeight; }
  float GetVerticalityType() const { return _verticalityType; }

private:
  static constexpr float CAP_HEIGHT = 4000.0f;

  // [핵심] 물 높이 정보를 포함하여 정확히 판별하는 함수
  float ResolveObjectType(const RE::hkpWorldRayCastOutput &a_rayOutput,
                          const RE::NiPoint3 &a_hitPos,
                          RE::TESObjectCELL *a_cell);

  float UpdateObstacleDetection(RE::PlayerCharacter *a_player);
  float UpdateVerticality(RE::TESGlobal *a_global,
                          RE::PlayerCharacter *a_player,
                          const RE::NiPoint3 &a_pos,
                          const RE::NiPoint3 &a_offset,
                          const RE::NiPoint3 &a_vel, float a_predictionTime);

  // 헬퍼: 셀에서 물 높이 가져오기
  float GetCellWaterHeight(RE::TESObjectCELL *a_cell,
                           const RE::NiPoint3 &a_pos);

  ShadowsLogic() = default;
  ~ShadowsLogic() = default;
  ShadowsLogic(const ShadowsLogic &) = delete;
  ShadowsLogic(const ShadowsLogic &&) = delete;
  ShadowsLogic &operator=(const ShadowsLogic &) = delete;
  ShadowsLogic &operator=(const ShadowsLogic &&) = delete;

  RE::TESGlobal *_verticalityFrontGlobal{nullptr};
  RE::TESGlobal *_verticalityLeftGlobal{nullptr};
  RE::TESGlobal *_verticalityRightGlobal{nullptr};
  RE::TESGlobal *_verticalityObstacleGlobal{nullptr};
  RE::TESGlobal *_verticalityObstacleTypeGlobal{nullptr};
  RE::TESGlobal *_verticalityPlayerGlobal{nullptr};
  RE::TESGlobal *_verticalityTypeGlobal{nullptr};

  RE::NiPoint3 _lastPosition;
  bool _initialized{false};

  float _frontDiff{0.0f};
  float _leftDiff{0.0f};
  float _rightDiff{0.0f};
  float _obstacleDist{0.0f};
  float _obstacleType{0.0f};
  float _playerHeight{0.0f};
  float _verticalityType{0.0f};
};