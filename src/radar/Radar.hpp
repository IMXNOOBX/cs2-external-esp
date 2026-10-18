#pragma once

class Radar {
public:
    static bool Init();
    static void Destroy();
private:
    static Radar& GetInstance() {
        static Radar instance;
        return instance;
    }

    ~Radar();
    bool InitImpl();
    void DestroyImpl();
    void Thread();
    const std::string& GetState();

    std::atomic<bool> running{ false };
    std::thread thread;
    std::string state = R"({"in_match":false,"map":"","players":[]})";
    std::chrono::steady_clock::time_point updated{};
    SOCKET listener = INVALID_SOCKET;
};
