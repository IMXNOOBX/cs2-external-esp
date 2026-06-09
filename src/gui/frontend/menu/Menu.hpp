#pragma once
#include "assets/fonts/Icons.h"

enum Tab {
    PLAYER,
    WORLD,
    SETTINGS
};

struct TabItem
{
    Tab id;
    std::string label;
    std::string icon;
};

static const TabItem tabs[] =
{
    { Tab::PLAYER,      "Player",   Icons::PERSON },
    { Tab::WORLD,       "World",    Icons::GLOBE },
    { Tab::SETTINGS,    "Settings", Icons::SETTINGS }
};

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