#pragma once

#include "API/OpenAnimationReplacerAPI-Conditions.h"
#include "RaySenseLogic.h"

namespace OARConditions {
using namespace OAR_API::Conditions;

// Condition to check verticality differences (Front, Left, Right, PlayerHeight)
class VerticalityCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "RaySense_Verticality"sv;

  VerticalityCondition();

  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks verticality difference for a specified sensor."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }

  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr,
                    RE::hkbClipGenerator *a_clipGenerator,
                    void *a_subMod) const override;

  Conditions::INumericConditionComponent
      *sensorIndexComponent; // 0: Front, 1: Left, 2: Right, 3: PlayerHeight
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// Condition to check obstacle distance
class ObstacleCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "RaySense_Obstacle"sv;

  ObstacleCondition();

  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks distance to the front obstacle."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }

  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr,
                    RE::hkbClipGenerator *a_clipGenerator,
                    void *a_subMod) const override;

  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// Condition to check front obstacle FormType
class ObstacleTypeCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "RaySense_ObstacleType"sv;

  ObstacleTypeCondition();

  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks the FormType of the object in front of the player."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }

  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr,
                    RE::hkbClipGenerator *a_clipGenerator,
                    void *a_subMod) const override;

  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};
} // namespace OARConditions
