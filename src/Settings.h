#pragma once

#include "PCH.h"

class Settings {
public:
  static Settings *GetSingleton() {
    static Settings singleton;
    return &singleton;
  }

  void Load();

  // [Debug]
  bool enableLog{false};
  bool enableDebugDraw{false};

  // [Tweaks]
  float furnitureExitCooldown{1.0f};
  float obstacleWalkDistance{230.0f};
  float obstacleSprintDistance{330.0f};
  float jumpBonus{80.0f};
  bool enableJumpBonus{true};

private:
  Settings() = default;
  ~Settings() = default;
  Settings(const Settings &) = delete;
  Settings &operator=(const Settings &) = delete;
};
