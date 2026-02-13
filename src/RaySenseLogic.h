#pragma once

#include "PCH.h"

class RaySenseLogic {
public:
  static RaySenseLogic *GetSingleton() {
    static RaySenseLogic singleton;
    return &singleton;
  }

  void Install();
  void OnUpdate(RE::PlayerCharacter *a_player, float a_delta);
  void OnJump(RE::PlayerCharacter *a_player);

  bool IsObstacleDetected() const;
  float GetJumpBonus() const { return OBSTACLE_JUMP_BONUS; }

  // Getters for OAR Conditions (All rounded values)
  float GetFrontDiff() const { return _frontDiff; }
  float GetLeftDiff() const { return _leftDiff; }
  float GetRightDiff() const { return _rightDiff; }
  float GetObstacleDist() const { return _obstacleDist; }
  float GetPlayerHeight() const { return _playerHeight; }
  std::uint32_t GetFrontObstacleType() const { return _frontObstacleType; }

private:
  static constexpr float CAP_HEIGHT = 4000.0f;
  static constexpr float OBSTACLE_JUMP_BONUS =
      80.0f; // Adjusted value for natural feel

  float UpdateObstacleDetection(RE::PlayerCharacter *a_player);
  std::uint32_t UpdateFrontObstacleType(RE::PlayerCharacter *a_player);
  float UpdateVerticality(RE::TESGlobal *a_global,
                          RE::PlayerCharacter *a_player,
                          const RE::NiPoint3 &a_pos,
                          const RE::NiPoint3 &a_offset,
                          const RE::NiPoint3 &a_vel, float a_predictionTime);

  RaySenseLogic() = default;
  ~RaySenseLogic() = default;
  RaySenseLogic(const RaySenseLogic &) = delete;
  RaySenseLogic(const RaySenseLogic &&) = delete;
  RaySenseLogic &operator=(const RaySenseLogic &) = delete;
  RaySenseLogic &operator=(const RaySenseLogic &&) = delete;

  RE::TESGlobal *_verticalityFrontGlobal{nullptr};
  RE::TESGlobal *_verticalityLeftGlobal{nullptr};
  RE::TESGlobal *_verticalityRightGlobal{nullptr};
  RE::TESGlobal *_verticalityObstacleGlobal{nullptr};
  RE::TESGlobal *_verticalityPlayerGlobal{nullptr};
  RE::NiPoint3 _lastUpdatePos;
  float _lastUpdateAngle{0.0f};
  bool _initialized{false};

  // Internal storage for values
  float _frontDiff{0.0f};
  float _leftDiff{0.0f};
  float _rightDiff{0.0f};
  float _obstacleDist{0.0f};
  float _playerHeight{0.0f};
  std::uint32_t _frontObstacleType{0}; // kNone
  // Helper for RayCasting to reduce duplication
  bool PerformRayCast(RE::PlayerCharacter *a_player,
                      const RE::NiPoint3 &a_start, const RE::NiPoint3 &a_end,
                      RE::hkpWorldRayCastOutput &a_output);
};
