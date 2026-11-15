// Most of this file was written with AI assistance

#pragma once

class Radar {
public:
    struct SharingState {
        bool active = false;
        std::string status;
        std::string url;
        std::string error;
    };

    static std::string GetUrl();
    static bool Init();
    static void Destroy();
    static bool Share();
    static bool IsSharing();
    static SharingState GetSharingState();
    static void StopSharing();
private:
    static Radar& GetInstance() {
        static Radar instance;
        return instance;
    }

    std::string token;
    ~Radar();
    bool InitImpl();
    void DestroyImpl();
    void Thread();
    const std::string& GetState();
    void RunSharing();
    std::string ReadSharingOutput(HANDLE output, HANDLE process);

    std::atomic<bool> running{ false };
    std::thread thread;
    std::string state = R"({"in_match":false,"map":"","players":[]})";
    std::chrono::steady_clock::time_point updated{};
    SOCKET listener = INVALID_SOCKET;
    std::atomic<bool> sharingStopped{ false };
    std::thread sharingThread;
    std::mutex sharingMutex;
    SharingState sharing;
};
