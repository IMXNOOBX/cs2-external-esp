#ifndef DISABLE_TIMERS

#include "Timer.hpp"

// Initialize static members
std::map<std::string, Timer::FunctionData> Timer::functionData;
std::mutex Timer::dataMutex;

Timer::Timer(const std::string& name) : name(name) {
    startTimePoint = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {
    auto endTimePoint = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(startTimePoint).time_since_epoch().count();
    auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(endTimePoint).time_since_epoch().count();
    auto duration = end - start;

    {
        std::lock_guard<std::mutex> lock(dataMutex);
        auto& data = functionData[name];
        data.totalTime += duration;
        data.invocations++;
        auto averageTime = data.totalTime / data.invocations;

        LOGF(WARNING, "{} ' took ' {}ms - Average {}ms - Invocations '{}'", name, duration, averageTime, data.invocations);
    }
}

#endif // DISABLE_TIMERS
