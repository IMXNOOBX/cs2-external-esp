#pragma once
#include "assets/fonts/Icons.h"

class Menu {
public:
    ~Menu() = default;
    Menu(const Menu&) = delete;
    Menu(Menu&&) = delete;
    Menu& operator=(const Menu&) = delete;
    Menu& operator=(Menu&&) = delete;

    static bool Init();
    static void Render();

    static void RenderStartupHelp();

    static ImVec2 GetPos();
    static ImVec2 GetSize();
private:
    Menu() {};

    static Menu& GetInstance()
    {
        static Menu i{};
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
    
    ImFont* font_icons;
};

enum Tab {
    PLAYER,
    WORLD,
    SETTINGS
};

struct TabItem
{
    std::string label;
    Tab id;
};

static const TabItem tabs[] =
{
    { std::string(Icons::PERSON) + " Player",   Tab::PLAYER   },
    { std::string(Icons::GLOBE) + " World",    Tab::WORLD    },
    { std::string(Icons::SETTINGS) + " Settings", Tab::SETTINGS }
};