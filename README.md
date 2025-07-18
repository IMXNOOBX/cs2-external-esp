# 🎡 CS2 External ESP

Simple external ESP for Counter-Strike 2 using the GDI overlay to render esp boxes on top of CS2, highlighting your enemies and teammates including their health and name. If you want to check out Discord overlay rendering check the [discord-overlay-esp](https://github.com/IMXNOOBX/cs2-external-esp/tree/discord-overlay) tree.

### Make sure you 🌟 the project if you like it ❤

## Video Showcase

[![Cs2ESP](.github/showcase.png)](https://youtu.be/SV_lddIxQ5w)
## 🌳 Simple Use

<!-- * Releases have been removed in order to be allowed in UnknownCheats, so the binaries are published there. -->
> You can download the latest binaries from the build artifacts in the [**Actions**](https://github.com/IMXNOOBX/cs2-external-esp/actions/workflows/auto_build.yml) tab.

* The application has been updated compared to the one in UC, i suggest you to build it yourself or use the automatic builds from above.

1. Go to the UnknownCheats post [**cs2-external-esp**](https://www.unknowncheats.me/forum/counter-strike-2-a/600259-cs2-external-esp.html)
2. Click on the binary file you want to download and download it.
3. Open the binary file and CS2 (Doesn't matter which one goes first (If you are having issues with ESP not showing, wait to open till in game))
❗ Make sure your game is in full screen windowed

## 💧 Repository Update

* If you have updated the offsets, and want to share it with everyone follow these steps

1. Create a [pull request](https://github.com/IMXNOOBX/cs2-external-esp/pulls) and provide just the `offsets/offsets.json` updated file in the pull request. If there are any other files modified it will be denied.

2. Provide a valid image that the esp is working on the latest version on the game

3. The commit will be merged into the main branch once verified and all the users will be able to update it!

## ✔ Manually Update

* To manually update the ESP offsets I have included an offsets.json file which will be created once opened.

Currently there are two ways to get the latest offsets.
1. Manually updating them <br>
1.1a Go to this [UnknownCheats thread](https://www.unknowncheats.me/forum/counter-strike-2-a/576077-counter-strike-2-reversal-structs-offsets.html) and find the latest offsets posted by the community <br>
1.1b Or go to the [cs2-dumper](https://github.com/a2x/cs2-dumper) repository and find the latest offsets

1.2 You will find something like this

```cpp
#define dwLocalPlayer 0x1715418 // This is hexadecimal
```

1.3 You will have to translate it to decimal and put it in the offsets file next to the ESP executable like so, you can use [**this website**](https://www.rapidtables.com/convert/number/hex-to-decimal.html)

```json
{
    "dwLocalPlayer": 24204312, // To decimal
    ...
}	
```

2. Automatically updating them using a script <br>
2.1 Download the `update_offsets.py` script and `offsets.json` file from [the offsets folder](https://github.com/IMXNOOBX/cs2-external-esp/tree/main/offsets) in this repository <br>
2.2 Put the `update_offsets.py` script next to the `offsets.json` file found next to your ESP executable <br>
2.3 Run `update_offsets.py`, the offsets will be automatically written to `offsets.json`

## 📘 Developer Instructions

1. Build the program using **Visual Studio 2022**
	- Build: **`x64 - Release`**

2. Locate your binary file in the folder `<arch>/<configuration>`, e.g., `x64/Release`.

* ❕ In case the offsets get outdated (Every game update), you could check UnnamedZ03's repository for the updated ones [here](https://github.com/UnnamedZ03/CS2-external-base/blob/58466cd7feba2fbcf5ab49b0dbbdc7bcd6d7df58/source/CSSPlayer.hpp#L3-L15)

## 💫 Credits

* [UnnamedZ03](https://github.com/UnnamedZ03) for providing [offsets](https://www.unknowncheats.me/forum/3846642-post734.html) and guide with his [CS2-external-base](https://github.com/UnnamedZ03/CS2-external-base)
* [a2x](https://github.com/a2x) for his [offset dumper](https://github.com/a2x/cs2-dumper) and constant updates to it
* [ifBars](https://github.com/ifBars) for his [contributions](https://github.com/IMXNOOBX/cs2-external-esp/pull/37) to the project and ideas
* [Bekston](https://github.com/Bekston) for his [contributions](https://github.com/IMXNOOBX/cs2-external-esp/pull/20) to the project and ideas
* [Apxaey](https://github.com/Apxaey) for releasing an easy way to implement [handle hijacking](https://github.com/Apxaey/Handle-Hijacking-Anti-Cheat-Bypass) 
* All [contributors](https://github.com/IMXNOOBX/cs2-external-esp/graphs/contributors) who have helped improve the project!
* The UnknownCheats community for their research!

# 🔖 License & Copyright

This project is licensed under [**CC BY-NC 4.0**](https://creativecommons.org/licenses/by-nc/4.0/).

```diff
+ You are free to:
	• Share: Copy and redistribute the material in any medium or format.
	• Adapt: Remix, transform, and build upon the material.
+ Under the following terms:
	• Attribution: You must give appropriate credit, provide a link to the original source repository, and indicate if changes were made.
	• Non-Commercial: You may not use the material for commercial purposes.
- You are not allowed to:
	• Sell: This license forbids selling original or modified material for commercial purposes.
	• Sublicense: This license forbids sublicensing original or modified material.
```
### ©️ Copyright
The content of this project is ©️ by [IMXNOOBX](https://github.com/IMXNOOBX) and the respective contributors. See the [LICENSE.md](LICENSE.md) file for details.
