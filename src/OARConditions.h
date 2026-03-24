#pragma once

#include "API/OpenAnimationReplacerAPI-Conditions.h"
#include "RaySenseLogic.h"

namespace OARConditions {
using namespace OAR_API::Conditions;

// 1. RaySense_Verticality (Height differences and basics)
class VerticalityCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "RaySense_Verticality"sv;
  VerticalityCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Check verticality diff (0:F, 1:L, 2:R, 3:Plyr, 4:Surf, 5:Plat)."sv
        .data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::INumericConditionComponent *sensorIndexComponent;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// 2. RaySense_Slope (Surface and Macro angles)
class SlopeCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME = "RaySense_Slope"sv;
  SlopeCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Check slope (0:Surf, 1:F, 2:L, 3:R)."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::INumericConditionComponent *sensorIndexComponent;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// 3. RaySense_Wall (Horizontal distances to walls)
class WallCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME = "RaySense_Wall"sv;
  WallCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Check wall dist (0:F, 1:FL, 2:FR, 3:L, 4:R)."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::INumericConditionComponent *sensorIndexComponent;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// 4. RaySense_Obstacle (Type and distance)
class ObstacleCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "RaySense_Obstacle"sv;
  ObstacleCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Check obstacle (0-2:Front/L/R Type, 3:Vault Dist)."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::INumericConditionComponent *sensorIndexComponent;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

} // namespace OARConditions
