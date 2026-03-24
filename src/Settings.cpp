#include "Settings.h"

#include <SimpleIni.h>

void Settings::Load() {
  CSimpleIniA ini;
  ini.SetUnicode();

  auto err = ini.LoadFile(L"Data/SKSE/Plugins/OpenAnimationReplacer-RaySense.ini");
  if (err < 0) {
    SKSE::log::warn("Settings: Failed to read INI file, using defaults.");
    return;
  }

  enableLog              = ini.GetBoolValue ("Debug",  "bEnableLog",              false);
  enableDebugDraw        = ini.GetBoolValue ("Debug",  "bEnableDebugDraw",        false);
  furnitureExitCooldown  = static_cast<float>(ini.GetDoubleValue("Tweaks", "fFurnitureExitCooldown",  1.0));
  obstacleWalkDistance   = static_cast<float>(ini.GetDoubleValue("Tweaks", "fObstacleWalkDistance",   230.0));
  obstacleSprintDistance = static_cast<float>(ini.GetDoubleValue("Tweaks", "fObstacleSprintDistance", 330.0));
  jumpBonus              = static_cast<float>(ini.GetDoubleValue("Tweaks", "fJumpBonus",              80.0));
  enableJumpBonus        = ini.GetBoolValue ("Tweaks", "bEnableJumpBonus",        true);

  SKSE::log::info(
      "Settings loaded | enableLog={} | enableDebugDraw={} | furnitureCooldown={:.2f} | "
      "obstacleWalk={:.0f} | obstacleSprint={:.0f} | "
      "jumpBonus={:.1f} | enableJumpBonus={}",
      enableLog, enableDebugDraw, furnitureExitCooldown, obstacleWalkDistance,
      obstacleSprintDistance, jumpBonus, enableJumpBonus);
}
