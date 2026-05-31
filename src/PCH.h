#pragma once

// CommonLibF4VR
#include "F4SE/F4SE.h"
#include "RE/Fallout.h"

// Standard
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <list>
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

// Windows
#include <Windows.h>
#include <spdlog/sinks/basic_file_sink.h>

// Macros
#define DLLEXPORT __declspec(dllexport)

// Logging aliases
namespace logger = F4SE::log;

// Version info
#include "Version.h"

// Shared diagnostics/parser helpers
#include "PatchDiagnostics.h"

// VR compatibility layer (missing enums, fmt formatters, helper functions)
#include "VRCompat.h"

using namespace std::literals;
