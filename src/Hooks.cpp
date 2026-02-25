#include "Hooks.h"
#include "PCH.h"
#include "RaySenseLogic.h"

namespace Hooks {
void PlayerHook::Install() {
  REL::Relocation<std::uintptr_t> vtable{RE::PlayerCharacter::VTABLE[0]};

  // Update Hook (0xAD)
  _Update = vtable.write_vfunc(0xAD, reinterpret_cast<std::uintptr_t>(Update));

  // Jump Hook (0x14)
  _Jump = vtable.write_vfunc(0x14, reinterpret_cast<std::uintptr_t>(Jump));

  SKSE::log::info(
      "PlayerHook: Hooks installed at 0xAD (Update) and 0x14 (Jump)");
}

void PlayerHook::Update(RE::PlayerCharacter *a_this, float a_delta) {
  // Call original first
  using func_t = void (*)(RE::PlayerCharacter *, float);
  reinterpret_cast<func_t>(_Update)(a_this, a_delta);

  // Run our logic
  RaySenseLogic::GetSingleton()->OnUpdate(a_this, a_delta);
}

void PlayerHook::Jump(RE::PlayerCharacter *a_this) {
  // Run our logic BEFORE original Jump (ensuring Global is set before animation
  // event)
  auto *logic = RaySenseLogic::GetSingleton();

  // Quick safety check
  if (a_this && a_this->Is3DLoaded() && !a_this->IsDead()) {
    if (auto *state = a_this->AsActorState();
        !(a_this->IsOnMount() || (state && state->IsSwimming()))) {
      logic->OnJump(a_this);
    }
  }

  // Call original
  using func_t = void (*)(RE::PlayerCharacter *);
  reinterpret_cast<func_t>(_Jump)(a_this);

  // Apply jump bonus if obstacle detected
  if (a_this && a_this->Is3DLoaded() && logic->IsObstacleDetected()) {
    if (auto *ctrl = a_this->GetCharController()) {
      RE::hkVector4 currentVel;
      ctrl->GetLinearVelocityImpl(currentVel);

      // Havok uses meters, so we scale the bonus
      float scale = RE::bhkWorld::GetWorldScale();
      currentVel.quad.m128_f32[2] += logic->GetJumpBonus() * scale;

      ctrl->SetLinearVelocityImpl(currentVel);
    }
  }
}
} // namespace Hooks
