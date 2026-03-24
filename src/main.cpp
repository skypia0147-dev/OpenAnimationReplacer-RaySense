#include "DebugDraw.h"
#include "Hooks.h"
#include "OARConditions.h"
#include "RaySenseLogic.h"
#include "Settings.h"
#include <spdlog/sinks/basic_file_sink.h>

using namespace std::literals;

SKSEPluginInfo(.Version = {1, 0, 0, 0},
               .Name = "Open Animation Replacer - RaySense", .Author = "Smooth",
               .RuntimeCompatibility =
                   SKSE::VersionIndependence::AddressLibrary)

    namespace {
  void InitializeLog() {
    auto path = SKSE::log::log_directory();
    if (!path) {
      return;
    }

    *path /= "OpenAnimationReplacer-RaySense.log";
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        path->string(), true);

    auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%H:%M:%S.%e] [%n] [%l] %v"s);

    SKSE::log::info("Open Animation Replacer - RaySense log initialized");
  }

  void OnMessaging(SKSE::MessagingInterface::Message * a_msg) {
    switch (a_msg->type) {
    case SKSE::MessagingInterface::kPostLoad:
      SKSE::log::info("RaySenseVerticality: Post-Load message received.");
      // 1. RaySense_Verticality
      if (OAR_API::Conditions::AddCustomCondition<
              OARConditions::VerticalityCondition>() ==
          OAR_API::Conditions::APIResult::OK) {
        SKSE::log::info("RaySense: Registered Consolidated OAR Condition "
                        "'RaySense_Verticality'");
      }

      // 2. RaySense_Slope
      if (OAR_API::Conditions::AddCustomCondition<
              OARConditions::SlopeCondition>() ==
          OAR_API::Conditions::APIResult::OK) {
        SKSE::log::info(
            "RaySense: Registered Consolidated OAR Condition 'RaySense_Slope'");
      }

      // 3. RaySense_Wall
      if (OAR_API::Conditions::AddCustomCondition<
              OARConditions::WallCondition>() ==
          OAR_API::Conditions::APIResult::OK) {
        SKSE::log::info(
            "RaySense: Registered Consolidated OAR Condition 'RaySense_Wall'");
      }

      // 4. RaySense_Obstacle
      if (OAR_API::Conditions::AddCustomCondition<
              OARConditions::ObstacleCondition>() ==
          OAR_API::Conditions::APIResult::OK) {
        SKSE::log::info("RaySense: Registered Consolidated OAR Condition "
                        "'RaySense_Obstacle'");
      }
      break;
    case SKSE::MessagingInterface::kDataLoaded:
      RaySenseLogic::GetSingleton()->Install();
      Hooks::PlayerHook::Install();
      RaySense::DebugDraw::Install();
      break;
    }
  }
} // namespace

SKSEPluginLoad(const SKSE::LoadInterface *a_skse) {
  InitializeLog();
  Settings::GetSingleton()->Load();
  SKSE::log::info("RaySense - Verticality loaded");

  SKSE::Init(a_skse);

  auto messaging = SKSE::GetMessagingInterface();
  if (messaging) {
    messaging->RegisterListener(OnMessaging);
  }

  return true;
}
