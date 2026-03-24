#include "OARConditions.h"
#include <cmath>
#include <format>

namespace OARConditions {

// --- 1. VerticalityCondition ---
VerticalityCondition::VerticalityCondition() {
  sensorIndexComponent = static_cast<Conditions::INumericConditionComponent *>(
      AddBaseComponent(Conditions::ConditionComponentType::kNumeric,
                       "Sensor(0:F, 1:L, 2:R, 3:Plyr, 4:Surf, 5:Plat)"));
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
  return RE::BSString(std::format("{}Verticality {} {}", sensorName,
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}

RE::BSString VerticalityCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  auto *logic = RaySenseLogic::GetSingleton();
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
  case 4:
    val = logic->GetSurfaceType();
    break;
  case 5:
    val = logic->GetPlatformType();
    break;
  }
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

// --- 2. SlopeCondition ---
SlopeCondition::SlopeCondition() {
  sensorIndexComponent = static_cast<Conditions::INumericConditionComponent *>(
      AddBaseComponent(Conditions::ConditionComponentType::kNumeric,
                       "Sensor(0:Surf, 1:F, 2:L, 3:R)"));
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent = static_cast<Conditions::INumericConditionComponent *>(
      AddBaseComponent(Conditions::ConditionComponentType::kNumeric, "Value"));
}

RE::BSString SlopeCondition::GetArgument() const {
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(nullptr));
  const char *sensorName = "Surface";
  switch (idx) {
  case 0:
    sensorName = "Surface";
    break;
  case 1:
    sensorName = "Front";
    break;
  case 2:
    sensorName = "Left";
    break;
  case 3:
    sensorName = "Right";
    break;
  }
  return RE::BSString(std::format("{}Slope {} {}", sensorName,
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}

RE::BSString SlopeCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  auto *logic = RaySenseLogic::GetSingleton();
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(a_refr));
  float val = 0.0f;
  switch (idx) {
  case 0:
    val = logic->GetSurfaceSlope();
    break;
  case 1:
    val = logic->GetMacroFrontSlope();
    break;
  case 2:
    val = logic->GetMacroLeftSlope();
    break;
  case 3:
    val = logic->GetMacroRightSlope();
    break;
  }
  return RE::BSString(std::to_string(static_cast<int>(val)).c_str());
}

bool SlopeCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                  RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  auto *logic = RaySenseLogic::GetSingleton();
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(a_refr));
  float currentVal = 0.0f;
  switch (idx) {
  case 0:
    currentVal = logic->GetSurfaceSlope();
    break;
  case 1:
    currentVal = logic->GetMacroFrontSlope();
    break;
  case 2:
    currentVal = logic->GetMacroLeftSlope();
    break;
  case 3:
    currentVal = logic->GetMacroRightSlope();
    break;
  }
  return comparisonComponent->GetComparisonResult(
      currentVal, valueComponent->GetNumericValue(a_refr));
}

// --- 3. WallCondition ---
WallCondition::WallCondition() {
  sensorIndexComponent = static_cast<Conditions::INumericConditionComponent *>(
      AddBaseComponent(Conditions::ConditionComponentType::kNumeric,
                       "Sensor(0:F, 1:FL, 2:FR, 3:L, 4:R)"));
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent =
      static_cast<Conditions::INumericConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kNumeric, "Distance"));
}

RE::BSString WallCondition::GetArgument() const {
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(nullptr));
  const char *sensorName = "Front";
  switch (idx) {
  case 0:
    sensorName = "Front";
    break;
  case 1:
    sensorName = "FrontL";
    break;
  case 2:
    sensorName = "FrontR";
    break;
  case 3:
    sensorName = "Left";
    break;
  case 4:
    sensorName = "Right";
    break;
  }
  return RE::BSString(std::format("{}Wall {} {}", sensorName,
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}

RE::BSString WallCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  auto *logic = RaySenseLogic::GetSingleton();
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(a_refr));
  float val = 0.0f;
  switch (idx) {
  case 0:
    val = logic->GetWallFrontDist();
    break;
  case 1:
    val = logic->GetWallFrontLDist();
    break;
  case 2:
    val = logic->GetWallFrontRDist();
    break;
  case 3:
    val = logic->GetWallLeftDist();
    break;
  case 4:
    val = logic->GetWallRightDist();
    break;
  }
  return RE::BSString(std::to_string(static_cast<int>(val)).c_str());
}

bool WallCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                 RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  auto *logic = RaySenseLogic::GetSingleton();
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(a_refr));
  float currentVal = 0.0f;
  switch (idx) {
  case 0:
    currentVal = logic->GetWallFrontDist();
    break;
  case 1:
    currentVal = logic->GetWallFrontLDist();
    break;
  case 2:
    currentVal = logic->GetWallFrontRDist();
    break;
  case 3:
    currentVal = logic->GetWallLeftDist();
    break;
  case 4:
    currentVal = logic->GetWallRightDist();
    break;
  }
  return comparisonComponent->GetComparisonResult(
      currentVal, valueComponent->GetNumericValue(a_refr));
}

// --- 4. ObstacleCondition ---
ObstacleCondition::ObstacleCondition() {
  sensorIndexComponent = static_cast<Conditions::INumericConditionComponent *>(
      AddBaseComponent(Conditions::ConditionComponentType::kNumeric,
                       "Sensor(0:Fnd, 1:Lnd, 2:Rnd, 3:VaultDist)"));
  comparisonComponent =
      static_cast<Conditions::IComparisonConditionComponent *>(AddBaseComponent(
          Conditions::ConditionComponentType::kComparison, "Comparison"));
  valueComponent = static_cast<Conditions::INumericConditionComponent *>(
      AddBaseComponent(Conditions::ConditionComponentType::kNumeric, "Value"));
}

RE::BSString ObstacleCondition::GetArgument() const {
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(nullptr));
  const char *sensorName = "FrontType";
  switch (idx) {
  case 0:
    sensorName = "FrontType";
    break;
  case 1:
    sensorName = "LeftType";
    break;
  case 2:
    sensorName = "RightType";
    break;
  case 3:
    sensorName = "VaultDist";
    break;
  }
  return RE::BSString(std::format("{}Obstacle {} {}", sensorName,
                                  comparisonComponent->GetArgument().c_str(),
                                  valueComponent->GetArgument().c_str())
                          .c_str());
}

RE::BSString ObstacleCondition::GetCurrent(RE::TESObjectREFR *a_refr) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return "0";
  auto *logic = RaySenseLogic::GetSingleton();
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(a_refr));
  float val = 0.0f;
  switch (idx) {
  case 0:
    val = static_cast<float>(logic->GetObstacleTypeFront());
    break;
  case 1:
    val = static_cast<float>(logic->GetObstacleTypeLeft());
    break;
  case 2:
    val = static_cast<float>(logic->GetObstacleTypeRight());
    break;
  case 3:
    val = logic->GetObstacleDist();
    break;
  }
  return RE::BSString(std::to_string(static_cast<int>(val)).c_str());
}

bool ObstacleCondition::EvaluateImpl(RE::TESObjectREFR *a_refr,
                                     RE::hkbClipGenerator *, void *) const {
  if (!a_refr || !a_refr->IsPlayerRef())
    return false;
  auto *logic = RaySenseLogic::GetSingleton();
  int idx = static_cast<int>(sensorIndexComponent->GetNumericValue(a_refr));
  float currentVal = 0.0f;
  switch (idx) {
  case 0:
    currentVal = static_cast<float>(logic->GetObstacleTypeFront());
    break;
  case 1:
    currentVal = static_cast<float>(logic->GetObstacleTypeLeft());
    break;
  case 2:
    currentVal = static_cast<float>(logic->GetObstacleTypeRight());
    break;
  case 3:
    currentVal = logic->GetObstacleDist();
    break;
  }
  return comparisonComponent->GetComparisonResult(
      currentVal, valueComponent->GetNumericValue(a_refr));
}

} // namespace OARConditions
