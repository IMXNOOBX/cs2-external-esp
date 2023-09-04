# 🎡 CS2 External ESP

Simple external esp using discord's overlay to render a box on top of cs2 highlighting your enemies and teammates including their health.

## Video Showcase

[![Cs2ESP](https://cdn.discordapp.com/attachments/903283950267564094/1148225159443005440/image.png)](https://youtu.be/SV_lddIxQ5w)
## 🌳 Simple Use

1. Go to the [**releases**](https://github.com/IMXNOOBX/cs2-external-esp/releases) tab.
2. Find the latest release and download it
3. Open the binary file and cs2 (doesnt matter which one goes first)

* ❗ You must have discord overlay enabled! Settings > Game Overlay > Enable
* ❗ Make sure you have enabled the overlay in for that game in Registered Games
* ❗ Make you have `-insecure` in csgo launch parameters for the overlay to work

## 📘 Developer Instructions

1. Build the program using Visual Studio 2022
	- Build: **`x64 - Release`**

2. Locate your binary file in the folder `<arch>/<configuration>`, e.g., `x64/Release`.

* ❕ In case the offsets get outdated (Every game update), you could check UnnamedZ03's repository for the updated ones [here](https://github.com/UnnamedZ03/CS2-external-base/blob/58466cd7feba2fbcf5ab49b0dbbdc7bcd6d7df58/source/CSSPlayer.hpp#L3-L15)

## 💫 Credits

* SamuelTulach's [OverlayCord](https://github.com/SamuelTulach/OverlayCord) for making the discord overlay hijack.
* [UnnamedZ03](https://github.com/UnnamedZ03) for providing [offsets](https://www.unknowncheats.me/forum/3846642-post734.html) and guide with his [CS2-external-base](https://github.com/UnnamedZ03/CS2-external-base)
* Unknowncheats comunity for their reseach!
