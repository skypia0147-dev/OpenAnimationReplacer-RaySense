#include "OARConditions.h"
#include <cmath>
#include <format>

namespace OARConditions {
// --- VerticalityCondition ---

VerticalityCondition::VerticalityCondition() {
  sensorIndexComponent = static_cast<Conditions::INumericConditionComponent *>(
      AddBaseComponent(Conditions::ConditionComponentType::kNumeric,
                       "Sensor(0:F, 1:L, 2:R, 3:P, 4:S, 5:Pl)"));
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent = static_cast<Conditions::INumericConditionComponent *>(
      AddBaseComponent(Conditions::ConditionComponentType::kNumeric, "Value"));
}

RE::BSString VerticalityCondition::GetArgument() const {
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(nullptr));
  const char *sensorName = "Unknown";
  switch (idx) {
  case 0:
    sensorName = "Front";
    break;
  case 1:
    sensorName = "Left";
    break;
  case 2:
    sensorName = "Right";
    break;
  case 3:
    sensorName = "Player";
    break;
  case 4:
    sensorName = "Surface";
    break;
  case 5:
    sensorName = "Platform";
    break;
  }

  return RE::BSString(std::format("{} {} {}", sensorName,
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}

RE::BSString VerticalityCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";

  auto *logic = RaySenseLogic::GetSingleton();
  float fIdx = sensorIndexComponent->GetNumericValue(a_refr);
  if (!std::isfinite(fIdx))
    return "0";

  int idx = static_cast<int>(fIdx);
  float val = 0.0f;
  switch (idx) {
  case 0:
    val = logic->GetFrontDiff();
    break;
  case 1:
    val = logic->GetLeftDiff();
    break;
  case 2:
    val = logic->GetRightDiff();
    break;
  case 3:
    val = logic->GetPlayerHeight();
    break;
  case 4:
    val = logic->GetSurfaceType();
    break;
  case 5:
    val = logic->GetPlatformType();
    break;
  }

  if (!std::isfinite(val))
    return "0";
  return RE::BSString(std::to_string(static_cast<int>(val)).c_str());
}

bool VerticalityCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                        RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;

  auto *logic = RaySenseLogic::GetSingleton();
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(a_refr));
  float currentVal = 0.0f;
  switch (idx) {
  case 0:
    currentVal = logic->GetFrontDiff();
    break;
  case 1:
    currentVal = logic->GetLeftDiff();
    break;
  case 2:
    currentVal = logic->GetRightDiff();
    break;
  case 3:
    currentVal = logic->GetPlayerHeight();
    break;
  case 4:
    currentVal = logic->GetSurfaceType();
    break;
  case 5:
    currentVal = logic->GetPlatformType();
    break;
  }

  return comparisonComponent->GetComparisonResult(
      currentVal, valueComponent->GetNumericValue(a_refr));
}

// --- ObstacleCondition (Vaultable) ---
ObstacleCondition::ObstacleCondition() {
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "Distance"));
}
RE::BSString ObstacleCondition::GetArgument() const {
  return RE::BSString(std::format("ObstacleVault {} {}",
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}
RE::BSString ObstacleCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  float dist = RaySenseLogic::GetSingleton()->GetObstacleDist();
  if (!std::isfinite(dist))
    return "0";
  return RE::BSString(std::to_string(static_cast<int>(dist)).c_str());
}
bool ObstacleCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                     RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  float dist = RaySenseLogic::GetSingleton()->GetObstacleDist();
  return comparisonComponent->GetComparisonResult(
      dist, valueComponent->GetNumericValue(a_refr));
}

// --- WallFrontCondition ---
WallFrontCondition::WallFrontCondition() {
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "Distance"));
}
RE::BSString WallFrontCondition::GetArgument() const {
  return RE::BSString(std::format("WallFront {} {}",
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}
RE::BSString WallFrontCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  float dist = RaySenseLogic::GetSingleton()->GetWallFrontDist();
  if (!std::isfinite(dist))
    return "0";
  return RE::BSString(std::to_string(static_cast<int>(dist)).c_str());
}
bool WallFrontCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                      RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  float dist = RaySenseLogic::GetSingleton()->GetWallFrontDist();
  return comparisonComponent->GetComparisonResult(
      dist, valueComponent->GetNumericValue(a_refr));
}

// --- WallFrontLCondition ---
WallFrontLCondition::WallFrontLCondition() {
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "Distance"));
}
RE::BSString WallFrontLCondition::GetArgument() const {
  return RE::BSString(std::format("WallFrontL {} {}",
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}
RE::BSString WallFrontLCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  float dist = RaySenseLogic::GetSingleton()->GetWallFrontLDist();
  if (!std::isfinite(dist))
    return "0";
  return RE::BSString(std::to_string(static_cast<int>(dist)).c_str());
}
bool WallFrontLCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                       RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  float dist = RaySenseLogic::GetSingleton()->GetWallFrontLDist();
  return comparisonComponent->GetComparisonResult(
      dist, valueComponent->GetNumericValue(a_refr));
}

// --- WallFrontRCondition ---
WallFrontRCondition::WallFrontRCondition() {
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "Distance"));
}
RE::BSString WallFrontRCondition::GetArgument() const {
  return RE::BSString(std::format("WallFrontR {} {}",
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}
RE::BSString WallFrontRCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  float dist = RaySenseLogic::GetSingleton()->GetWallFrontRDist();
  if (!std::isfinite(dist))
    return "0";
  return RE::BSString(std::to_string(static_cast<int>(dist)).c_str());
}
bool WallFrontRCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                       RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  float dist = RaySenseLogic::GetSingleton()->GetWallFrontRDist();
  return comparisonComponent->GetComparisonResult(
      dist, valueComponent->GetNumericValue(a_refr));
}

// --- WallLeftCondition ---
WallLeftCondition::WallLeftCondition() {
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "Distance"));
}
RE::BSString WallLeftCondition::GetArgument() const {
  return RE::BSString(std::format("WallLeft {} {}",
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}
RE::BSString WallLeftCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  float dist = RaySenseLogic::GetSingleton()->GetWallLeftDist();
  if (!std::isfinite(dist))
    return "0";
  return RE::BSString(std::to_string(static_cast<int>(dist)).c_str());
}
bool WallLeftCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                     RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  float dist = RaySenseLogic::GetSingleton()->GetWallLeftDist();
  return comparisonComponent->GetComparisonResult(
      dist, valueComponent->GetNumericValue(a_refr));
}

// --- WallRightCondition ---
WallRightCondition::WallRightCondition() {
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "Distance"));
}
RE::BSString WallRightCondition::GetArgument() const {
  return RE::BSString(std::format("WallRight {} {}",
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}
RE::BSString WallRightCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  float dist = RaySenseLogic::GetSingleton()->GetWallRightDist();
  if (!std::isfinite(dist))
    return "0";
  return RE::BSString(std::to_string(static_cast<int>(dist)).c_str());
}
bool WallRightCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                      RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  float dist = RaySenseLogic::GetSingleton()->GetWallRightDist();
  return comparisonComponent->GetComparisonResult(
      dist, valueComponent->GetNumericValue(a_refr));
}

// --- ObstacleTypeFrontCondition ---
ObstacleTypeFrontCondition::ObstacleTypeFrontCondition() {
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "FormType ID"));
}
RE::BSString ObstacleTypeFrontCondition::GetArgument() const {
  return RE::BSString(std::format("ObstacleTypeFront {} {}",
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}
RE::BSString
ObstacleTypeFrontCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  std::uint32_t type = RaySenseLogic::GetSingleton()->GetObstacleTypeFront();
  return RE::BSString(std::to_string(static_cast<int>(type)).c_str());
}
bool ObstacleTypeFrontCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                              RE::hkbClipGenerator *,
                                              void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  return comparisonComponent->GetComparisonResult(
      static_cast<float>(RaySenseLogic::GetSingleton()->GetObstacleTypeFront()),
      valueComponent->GetNumericValue(a_refr));
}

// --- ObstacleTypeLeftCondition ---
ObstacleTypeLeftCondition::ObstacleTypeLeftCondition() {
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "FormType ID"));
}
RE::BSString ObstacleTypeLeftCondition::GetArgument() const {
  return RE::BSString(std::format("ObstacleTypeLeft {} {}",
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}
RE::BSString
ObstacleTypeLeftCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  std::uint32_t type = RaySenseLogic::GetSingleton()->GetObstacleTypeLeft();
  return RE::BSString(std::to_string(static_cast<int>(type)).c_str());
}
bool ObstacleTypeLeftCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                             RE::hkbClipGenerator *,
                                             void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  return comparisonComponent->GetComparisonResult(
      static_cast<float>(RaySenseLogic::GetSingleton()->GetObstacleTypeLeft()),
      valueComponent->GetNumericValue(a_refr));
}

// --- ObstacleTypeRightCondition ---
ObstacleTypeRightCondition::ObstacleTypeRightCondition() {
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "FormType ID"));
}
RE::BSString ObstacleTypeRightCondition::GetArgument() const {
  return RE::BSString(std::format("ObstacleTypeRight {} {}",
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}
RE::BSString
ObstacleTypeRightCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  std::uint32_t type = RaySenseLogic::GetSingleton()->GetObstacleTypeRight();
  return RE::BSString(std::to_string(static_cast<int>(type)).c_str());
}
bool ObstacleTypeRightCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                              RE::hkbClipGenerator *,
                                              void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  return comparisonComponent->GetComparisonResult(
      static_cast<float>(RaySenseLogic::GetSingleton()->GetObstacleTypeRight()),
      valueComponent->GetNumericValue(a_refr));
}
} // namespace OARConditions
