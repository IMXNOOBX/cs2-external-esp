#pragma once

class Radar {
public:
    struct SharingState {
        bool active = false;
        std::string status;
        std::string url;
        std::string error;
    };

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

    ~Radar();
    bool InitImpl();
    void DestroyImpl();
    void Thread();
    const std::string& GetState();
    void ReadSharingOutput();
    void CloseSharing(bool clear);

    std::atomic<bool> running{ false };
    std::thread thread;
    std::string state = R"({"in_match":false,"map":"","players":[]})";
    std::chrono::steady_clock::time_point updated{};
    SOCKET listener = INVALID_SOCKET;
    HANDLE sharingProcess = nullptr;
    HANDLE sharingJob = nullptr;
    HANDLE sharingOutput = nullptr;
    std::thread sharingThread;
    std::mutex sharingMutex;
    SharingState sharing;
};
