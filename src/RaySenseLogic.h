#pragma once

#include "PCH.h"
#include <atomic>
#include <cmath>

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
  float GetFrontDiff() const {
    return _frontDiff.load(std::memory_order_relaxed);
  }
  float GetLeftDiff() const {
    return _leftDiff.load(std::memory_order_relaxed);
  }
  float GetRightDiff() const {
    return _rightDiff.load(std::memory_order_relaxed);
  }
  float GetObstacleDist() const {
    return _obstacleVaultDist.load(std::memory_order_relaxed);
  }
  float GetWallFrontDist() const {
    return _wallFrontDist.load(std::memory_order_relaxed);
  }
  float GetWallFrontLDist() const {
    return _wallFrontLDist.load(std::memory_order_relaxed);
  }
  float GetWallFrontRDist() const {
    return _wallFrontRDist.load(std::memory_order_relaxed);
  }
  float GetWallLeftDist() const {
    return _wallLeftDist.load(std::memory_order_relaxed);
  }
  float GetWallRightDist() const {
    return _wallRightDist.load(std::memory_order_relaxed);
  }
  float GetPlayerHeight() const {
    return _playerHeight.load(std::memory_order_relaxed);
  }
  float GetSurfaceType() const {
    return _surfaceType.load(std::memory_order_relaxed);
  }
  float GetPlatformType() const {
    return _platformType.load(std::memory_order_relaxed);
  }
  std::uint32_t GetObstacleTypeFront() const {
    return _obstacleTypeFront.load(std::memory_order_relaxed);
  }
  std::uint32_t GetObstacleTypeLeft() const {
    return _obstacleTypeLeft.load(std::memory_order_relaxed);
  }
  std::uint32_t GetObstacleTypeRight() const {
    return _obstacleTypeRight.load(std::memory_order_relaxed);
  }

private:
  static constexpr float CAP_HEIGHT = 4000.0f;
  static constexpr float OBSTACLE_JUMP_BONUS =
      80.0f; // Adjusted value for natural feel

  float UpdateObstacleDetection(RE::PlayerCharacter *a_player);
  std::uint32_t UpdateObstacleType(RE::PlayerCharacter *a_player,
                                   const RE::NiPoint3 &a_direction);
  float UpdateVerticality(RE::TESGlobal *a_global,
                          RE::PlayerCharacter *a_player,
                          const RE::NiPoint3 &a_pos,
                          const RE::NiPoint3 &a_offset,
                          const RE::NiPoint3 &a_vel, float a_predictionTime,
                          float a_slantAngle = 7.0f);

  void UpdateSurfaceInfo(RE::PlayerCharacter *a_player);

  enum class SurfaceType : std::uint32_t {
    kDefault = 0,
    kGrass = 1,
    kSnow = 2,
    kIce = 3,
    kWater = 4,
    kWood = 5,
    kStone = 6,
    kDirt = 7,
    kSand = 8,
    kGravel = 9
  };

  enum class PlatformType : std::uint32_t {
    kNone = 0,
    kMoving = 1,
    kActor = 2
  };

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
  RE::TESGlobal *_wallFrontGlobal{nullptr};
  RE::TESGlobal *_wallFrontLGlobal{nullptr};
  RE::TESGlobal *_wallFrontRGlobal{nullptr};
  RE::TESGlobal *_wallLeftGlobal{nullptr};
  RE::TESGlobal *_wallRightGlobal{nullptr};
  RE::TESGlobal *_obstacleTypeFrontGlobal{nullptr};
  RE::TESGlobal *_obstacleTypeLeftGlobal{nullptr};
  RE::TESGlobal *_obstacleTypeRightGlobal{nullptr};
  RE::TESGlobal *_verticalityPlayerGlobal{nullptr};
  RE::TESGlobal *_surfaceTypeGlobal{nullptr};
  RE::TESGlobal *_platformTypeGlobal{nullptr};
  RE::TESGlobal *_rawMaterialIDGlobal{nullptr};
  RE::TESGlobal *_rawLayerIDGlobal{nullptr};
  RE::NiPoint3 _lastUpdatePos;
  float _lastUpdateAngle{0.0f};
  bool _initialized{false};

  // Internal storage for values (Thread-safe for OAR)
  std::atomic<float> _frontDiff{0.0f};
  std::atomic<float> _leftDiff{0.0f};
  std::atomic<float> _rightDiff{0.0f};
  std::atomic<float> _obstacleVaultDist{0.0f};
  std::atomic<float> _wallFrontDist{0.0f};
  std::atomic<float> _wallFrontLDist{0.0f};
  std::atomic<float> _wallFrontRDist{0.0f};
  std::atomic<float> _wallLeftDist{0.0f};
  std::atomic<float> _wallRightDist{0.0f};
  std::atomic<float> _playerHeight{0.0f};
  std::atomic<float> _surfaceType{0.0f};
  std::atomic<float> _platformType{0.0f};
  std::atomic<std::uint32_t> _obstacleTypeFront{0};
  std::atomic<std::uint32_t> _obstacleTypeLeft{0};
  std::atomic<std::uint32_t> _obstacleTypeRight{0};
  // Helper for RayCasting to reduce duplication
  bool PerformRayCast(RE::PlayerCharacter *a_player,
                      const RE::NiPoint3 &a_start, const RE::NiPoint3 &a_end,
                      RE::hkpWorldRayCastOutput &a_output);

  bool PerformWaterRayCast(RE::PlayerCharacter *a_player,
                           const RE::NiPoint3 &a_start,
                           const RE::NiPoint3 &a_end,
                           RE::hkpWorldRayCastOutput &a_output);

  static bool IsFinite(const RE::NiPoint3 &a_vec) {
    return std::isfinite(a_vec.x) && std::isfinite(a_vec.y) &&
           std::isfinite(a_vec.z);
  }
};
