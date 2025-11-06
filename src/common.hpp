#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include <Logger.hpp>
#include <Windows.h>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <imgui.h>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <stack>
#include <string_view>
#include <thread>
#include <vector>

using namespace al;

#include "core/logger/LogHelper.hpp"
#include "core/engine/math/Math.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;