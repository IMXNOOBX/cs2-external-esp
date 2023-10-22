# 🎡 CS2 External ESP

Simple external esp using gdi overlay to render esp boxes on top of cs2 highlighting your enemies and teammates including their health, name and health number. If you want to check out discord overlay rendering check [discord-overlay-esp](https://github.com/IMXNOOBX/cs2-external-esp/tree/discord-overlay)

### Make sure you 🌟 the project if you like it ❤

## Video Showcase

[![Cs2ESP](https://cdn.discordapp.com/attachments/903283950267564094/1148569148318822461/image.png)](https://youtu.be/SV_lddIxQ5w)
## 🌳 Simple Use

* Releases have been removed in order to be allowed in unknowncheats so the binaries are published there
1. Go to unknwoncheats post [**cs2-external-esp**](https://www.unknowncheats.me/forum/counter-strike-2-a/600259-cs2-external-esp.html)
2. Click on the binary file you want to download and download it.
3. Open the binary file and cs2 (doesnt matter which one goes first)
❗ Make sure your game is in full screen windowed

## 💧 Repository Update

* If you have updated the offsets, and want to share it with everyone follow this  steps

1. Create a [pull request](https://github.com/IMXNOOBX/cs2-external-esp/pulls) and provided just the `offsets/offsets.json` updated file in the pull request. if there are any other files modified it will be denied

2. Provide a valid image that the esp is working on the latest version on the game

3. The commit will be merged into the main branch once verified and all the users will be able to update it!

## ✔ Manually Update

* To manually update the esp offsets i have included an offsets.json file which will be created once opened.

1. Two ways to get the latest offsets.
1.1 Go to this [unknowncheats thread](https://www.unknowncheats.me/forum/counter-strike-2-a/576077-counter-strike-2-reversal-structs-offsets.html) and find the latest offsets posted by the community
1.2 Or go to this repository [cs2-dumper](https://github.com/a2x/cs2-dumper) and find the latest offstes

2. You will find somethig like this

```cpp
#define dwLocalPlayer 0x1715418 // This is hexadecimal
```

3. You will have to translate it to decimal and put it in the offsets file next to the esp executable like so, you can use [**this website**](https://www.rapidtables.com/convert/number/hex-to-decimal.html)

```json
{
    "dwLocalPlayer": 24204312, // To decimal
    ...
}	
```

## 📘 Developer Instructions

1. Build the program using Visual Studio 2022
	- Build: **`x64 - Release`**

2. Locate your binary file in the folder `<arch>/<configuration>`, e.g., `x64/Release`.

* ❕ In case the offsets get outdated (Every game update), you could check UnnamedZ03's repository for the updated ones [here](https://github.com/UnnamedZ03/CS2-external-base/blob/58466cd7feba2fbcf5ab49b0dbbdc7bcd6d7df58/source/CSSPlayer.hpp#L3-L15)

## 💫 Credits

* [UnnamedZ03](https://github.com/UnnamedZ03) for providing [offsets](https://www.unknowncheats.me/forum/3846642-post734.html) and guide with his [CS2-external-base](https://github.com/UnnamedZ03/CS2-external-base)
* [a2x](https://github.com/a2x) for his [offset dumper](https://github.com/a2x/cs2-dumper) and constant updates to it
* [ifBars](https://github.com/ifBars) for his [contributions](https://github.com/IMXNOOBX/cs2-external-esp/pull/37) to the project and ideas
* [Bekston](https://github.com/Bekston) for his [contributions](https://github.com/IMXNOOBX/cs2-external-esp/pull/20) to the project and ideas
* [Apxaey](https://github.com/Apxaey) for releasing an easy way to implement [handle hijacking](https://github.com/Apxaey/Handle-Hijacking-Anti-Cheat-Bypass) 
* Unknowncheats comunity for their reseach!
