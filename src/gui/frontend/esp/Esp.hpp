#pragma once

class Esp {
public:
    ~Esp() = default;
    Esp(const Esp&) = delete;
    Esp(Esp&&) = delete;
    Esp& operator=(const Esp&) = delete;
    Esp& operator=(Esp&&) = delete;

    static void Render();
private:
    Esp() {};

    static Esp& GetInstance()
    {
        static Esp i{};
        return i;
    }

    void RenderImpl();
};