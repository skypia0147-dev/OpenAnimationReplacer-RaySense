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

// Condition to check FRONT wall distance
class WallFrontCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "RaySense_Wall_Front"sv;
  WallFrontCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks distance to the wall in front."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// Condition to check FRONT LEFT wall distance
class WallFrontLCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "RaySense_Wall_Front_L"sv;
  WallFrontLCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks distance to the wall on the front-left."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// Condition to check FRONT RIGHT wall distance
class WallFrontRCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "RaySense_Wall_Front_R"sv;
  WallFrontRCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks distance to the wall on the front-right."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// Condition to check LEFT wall distance
class WallLeftCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "RaySense_Wall_Left"sv;
  WallLeftCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks distance to the wall on the left."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// Condition to check RIGHT wall distance
class WallRightCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "RaySense_Wall_Right"sv;
  WallRightCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks distance to the wall on the right."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// Condition to check FRONT obstacle FormType
class ObstacleTypeFrontCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "Obstacle_Type_Front"sv;
  ObstacleTypeFrontCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks FormType of front obstacle."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// Condition to check LEFT obstacle FormType
class ObstacleTypeLeftCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "Obstacle_Type_Left"sv;
  ObstacleTypeLeftCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks FormType of left obstacle."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};

// Condition to check RIGHT obstacle FormType
class ObstacleTypeRightCondition : public Conditions::CustomCondition {
public:
  constexpr static inline std::string_view CONDITION_NAME =
      "Obstacle_Type_Right"sv;
  ObstacleTypeRightCondition();
  RE::BSString GetName() const override { return CONDITION_NAME.data(); }
  RE::BSString GetDescription() const override {
    return "Checks FormType of right obstacle."sv.data();
  }
  constexpr REL::Version GetRequiredVersion() const override {
    return {1, 0, 0};
  }
  RE::BSString GetArgument() const override;
  RE::BSString GetCurrent(RE::TESObjectREFR *a_refr) const override;

protected:
  bool EvaluateImpl(RE::TESObjectREFR *a_refr, RE::hkbClipGenerator *a_cg,
                    void *a_sm) const override;
  Conditions::IComparisonConditionComponent *comparisonComponent;
  Conditions::INumericConditionComponent *valueComponent;
};
} // namespace OARConditions
