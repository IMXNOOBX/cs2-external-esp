#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

// If you see dependency errors here, you didn't read README.md properly
#include <Logger.hpp>

#include <iostream>
#include <windows.h>

#include <map>
#include <array>
#include <memory>
#include <stack>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <future>
#include <cstddef>
#include <fstream>
#include <filesystem>
#include <functional>

#include <imgui.h>
#include <string_view>

#include <nlohmann/json.hpp>

using namespace al;

#include "core/logger/LogHelper.hpp"
#include "core/engine/types/Types.hpp"
#include "config/Current.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;
