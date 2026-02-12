#include "Hooks.h"
#include "ShadowsLogic.h"
#include <spdlog/sinks/basic_file_sink.h>

using namespace std::literals;

namespace {
void InitializeLog() {
  auto path = SKSE::log::log_directory();
  if (!path) {
    return;
  }

  *path /= "RaySenseVerticality.log";
  auto sink =
      std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

  auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

  log->set_level(spdlog::level::info);
  log->flush_on(spdlog::level::info);

  spdlog::set_default_logger(std::move(log));
  spdlog::set_pattern("[%H:%M:%S.%e] [%n] [%l] %v"s);
}

void OnMessaging(SKSE::MessagingInterface::Message *a_msg) {
  switch (a_msg->type) {
  case SKSE::MessagingInterface::kDataLoaded:
    ShadowsLogic::GetSingleton()->Install();
    Hooks::PlayerHook::Install();
    break;
  }
}
} // namespace

SKSEPluginLoad(const SKSE::LoadInterface *a_skse) {
  InitializeLog();
  SKSE::log::info("RaySense - Verticality loaded");

  SKSE::Init(a_skse);

  auto messaging = SKSE::GetMessagingInterface();
  if (messaging) {
    messaging->RegisterListener(OnMessaging);
  }

  return true;
}
