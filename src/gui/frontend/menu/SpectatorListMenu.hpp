#pragma once

class SpectatorListMenu {
public:
    ~SpectatorListMenu() = default;
    SpectatorListMenu(const SpectatorListMenu&) = delete;
    SpectatorListMenu(SpectatorListMenu&&) = delete;
    SpectatorListMenu& operator=(const SpectatorListMenu&) = delete;
    SpectatorListMenu& operator=(SpectatorListMenu&&) = delete;

    static bool Init();
    static void Render();

    static void RenderStartupHelp();

    static ImVec2 GetPos();
    static ImVec2 GetSize();
private:
    SpectatorListMenu() {};

    static SpectatorListMenu& GetInstance()
    {
        static SpectatorListMenu i{};
        return i;
    }

    bool InitImpl();
    void RenderImpl();
    void RenderStartupHelpImpl();

    void SetupStyles();
private:
    bool isSetup = true;

    ImVec2 pos;
    ImVec2 size;
};

enum Tab {
    PLAYER,
    WORLD,
    SETTINGS
};

struct TabItem
{
    const char* label;
    Tab id;
};

static constexpr TabItem tabs[] =
{
    { "Player",   Tab::PLAYER   },
    { "World",    Tab::WORLD    },
    { "Settings", Tab::SETTINGS }
};