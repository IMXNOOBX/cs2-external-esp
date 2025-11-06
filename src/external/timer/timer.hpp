#ifndef TIMER_HPP
#define TIMER_HPP

#ifdef DISABLE_TIMERS
#define TIMER(name) 
#else
#define TIMER(name) Timer timer(name)

class Timer
{
public:
    Timer(const  std::string& name);
    ~Timer();

private:
    struct FunctionData {
        long long totalTime = 0;
        int invocations = 0;
    };

    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTimePoint;

    static  std::map <std::string, FunctionData> functionData;
    static std::mutex dataMutex;
};
#endif // DISABLE_TIMERS

#endif // TIMER_HPP
