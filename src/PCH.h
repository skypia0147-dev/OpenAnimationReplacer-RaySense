#pragma once


#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>


#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <spdlog/sinks/basic_file_sink.h>

// D3D headers (must follow CommonLibSSE to avoid REX conflicts)
#include <d3d11.h>
#include <dxgi.h>

using namespace std::literals;
