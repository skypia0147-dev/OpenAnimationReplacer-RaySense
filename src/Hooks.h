#pragma once

#include "ShadowsLogic.h"

namespace Hooks {
class PlayerHook {
public:
  static void Install();

private:
  static void Update(RE::PlayerCharacter *a_this, float a_delta);
  static void Jump(RE::PlayerCharacter *a_this);

  static inline std::uintptr_t _Update;
  static inline std::uintptr_t _Jump;
};
} // namespace Hooks
