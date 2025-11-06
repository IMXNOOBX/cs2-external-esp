#pragma once

constexpr int MAX_ENTITIES = 1024;
constexpr int MAX_PLAYERS = 64;

class Cache {
public:
    ~Cache() = default;
    Cache(const Cache&) = delete;
    Cache(Cache&&) = delete;
    Cache& operator=(const Cache&) = delete;
    Cache& operator=(Cache&&) = delete;

    static bool Init();
private:
    Cache() {};

    static Cache& GetInstance()
    {
        static Cache i{};
        return i;
    }

    bool InitImpl();
};