#include "OARConditions.h"
#include <format>

namespace OARConditions {
// --- VerticalityCondition ---

VerticalityCondition::VerticalityCondition() {
  sensorIndexComponent = static_cast<Conditions::INumericConditionComponent *>(
      AddBaseComponent(Conditions::ConditionComponentType::kNumeric,
                       "Sensor (0:Front, 1:Left, 2:Right, 3:Player)"));
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
  }

  return RE::BSString(std::format("{} {} {}", sensorName,
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}

RE::BSString VerticalityCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";

  auto *logic = ShadowsLogic::GetSingleton();
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(a_refr));
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
  }
  return RE::BSString(std::to_string(static_cast<int>(val)).c_str());
}

bool VerticalityCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                        RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;

  auto *logic = ShadowsLogic::GetSingleton();
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
  }

  return comparisonComponent->GetComparisonResult(
      currentVal, valueComponent->GetNumericValue(a_refr));
}

// --- ObstacleCondition ---

ObstacleCondition::ObstacleCondition() {
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "Distance"));
}

RE::BSString ObstacleCondition::GetArgument() const {
  return RE::BSString(std::format("Obstacle {} {}",
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}

RE::BSString ObstacleCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  return RE::BSString(
      std::to_string(
          static_cast<int>(ShadowsLogic::GetSingleton()->GetObstacleDist()))
          .c_str());
}

bool ObstacleCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                     RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;

  float currentDist = ShadowsLogic::GetSingleton()->GetObstacleDist();
  // If 0, no obstacle detected. Should probably return false if looking for > 0
  if (currentDist <= 0.0f)
    return false;

  return comparisonComponent->GetComparisonResult(
      currentDist, valueComponent->GetNumericValue(a_refr));
}
} // namespace OARConditions
