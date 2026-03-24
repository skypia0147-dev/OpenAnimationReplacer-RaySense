#include "Hooks.h"
#include "PCH.h"
#include "RaySenseLogic.h"

namespace Hooks {
void PlayerHook::Install() {
  REL::Relocation<std::uintptr_t> vtable{RE::PlayerCharacter::VTABLE[0]};

  // Update Hook (0xAD)
  _Update = vtable.write_vfunc(0xAD, reinterpret_cast<std::uintptr_t>(Update));

  SKSE::log::info("PlayerHook: Update hook installed at 0xAD.");
}

void PlayerHook::Update(RE::PlayerCharacter *a_this, float a_delta) {
  using func_t = void (*)(RE::PlayerCharacter *, float);
  reinterpret_cast<func_t>(_Update)(a_this, a_delta);
  RaySenseLogic::GetSingleton()->OnUpdate(a_this, a_delta);
}

} // namespace Hooks
