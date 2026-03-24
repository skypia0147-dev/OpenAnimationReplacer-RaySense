#pragma once

#include "RaySenseLogic.h"

namespace Hooks {
class PlayerHook {
public:
  static void Install();

private:
  static void Update(RE::PlayerCharacter *a_this, float a_delta);

  static inline std::uintptr_t _Update;
};

} // namespace Hooks
