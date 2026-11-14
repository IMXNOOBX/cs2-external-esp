# Scripting

Scripts run on [Solis](https://github.com/IMXNOOBX/cs2-external-esp/tree/main/src/external/Solis).

The application loads every `.sl` file from the working directory and from a `scripts` folder next to it. The `scripts` folder is not created automatically. Each file is compiled separately, so scripts can be added or removed independently, but every file needs its own `main()`.

## Quick start

`movement.sl`:

```solis
import host;

func main() {
    host.bind("F1", "btn_toggle_box");
}

func btn_toggle_box() {
    host.set_bool("esp.box", !host.get_bool("esp.box"));
}
```

`main()` runs when the script loads, so that's where bindings belong. Each script keeps one Solis VM for as long as it is loaded, so **global variables keep their values between calls**: a script can count presses or remember a toggle with a top-level `var`, no `host.set_*` round trip needed. Unloading or reloading a script starts it again from a fresh `main()`. `btn_toggle_box` appears as a `toggle_box` button in the **Scripts** tab.

## Buttons

Any function without parameters whose name starts with `btn_` is shown in the GUI:

```solis
func btn_reset() {
    host.reset_theme();
}
```

The `btn_` prefix is removed from the button label. Functions without the prefix are not shown, but they can still be used with `host.bind` and `host.hold`.

## Multiple scripts

Each file is independent. A script can look like this:

```solis
import host;

func main() {
    host.bind("F1", "toggle");
    host.hold("SPACE", "jump");
}

func toggle() {
    host.set_bool("esp.skeleton", !host.get_bool("esp.skeleton"));
}

func jump() {
    host.send("SPACE", 50, 10);
}
```

The functions passed to `host.bind` and `host.hold` refer to functions in the same script. In the **Scripts** tab, every script is listed with its relative name (`movement` or `scripts/movement`) and its `btn_` functions.

Click a script name to open the file in the default editor. Use **Load** / **Unload** to enable a script, and **Refresh** to rescan the folders for new or removed files.

## Host API

Every function below is provided by the application through the `host` module. Add `import host;` at the top of the script and call it with the `host.` prefix, e.g. `host.set_bool("esp.box", true)`. The signatures are checked when the script compiles, so a wrong argument count or type is a compile error rather than a runtime surprise.

| Function | Description |
| --- | --- |
| `set_bool(name, value)` | Set a boolean config value. |
| `set_int(name, value)` | Set an integer config value. |
| `set_float(name, value)` | Set a float config value. |
| `set_color(name, r, g, b, a)` | Set an RGBA color from `0.0` to `1.0`. |
| `get_bool(name)` / `get_int(name)` / `get_float(name)` | Read a config value. |
| `get_color(name)` | Read a color as `"r,g,b,a"`. |
| `reset_theme()` | Restore the default UI colors. |
| `echo(message)` | Print a message to the console. |
| `print(message)` | Builtin alias of `echo`; available without the `host.` prefix. |
| `bind(key, function)` | Bind a key to a function in the same script. |
| `hold(key, function)` | Run a function while a key is held. |
| `client_base()` / `engine_base()` | Module base addresses. |
| `menu_open()`, `menu_x()`, `menu_y()` | Menu state and position. |

`host.bind` and `host.hold` are only accepted from `main()`.

The `math` and `strings` modules are available with `import math;` and `import strings;`.

### Keyboard input (debug builds)

```solis
host.send(key, duration_ms, repeat_count);
host.send("SPACE", 50, 10);
host.send("MOUSE1", 50, 10);
```

### Memory access (debug builds)

```solis
let value = host.read_int(0xABCD1234);
host.write_int(0xABCD1234, value + 1);
```

This directly accesses game memory and can crash the game or trigger anti-cheat detection.

## Host module

Scripts see the application through the generated `host` module. The application builds it from the natives it registers (names and signatures), so there is no `prelude.sl` to write, generate, or keep in sync. A `prelude.sl` left over from an older version is ignored and can be deleted.

Functions that only a script can define, such as its own structs, are not part of `host`; declare those in the script as usual.

## Configuration values

```text
# ESP
esp.team  esp.box  esp.skeleton  esp.health  esp.health_number
esp.armor  esp.spotted  esp.tracers  esp.head_tracker

# ESP flags
esp.flags.name  esp.flags.ping  esp.flags.money  esp.flags.weapon
esp.flags.ammo  esp.flags.reloading  esp.flags.scoped
esp.flags.defusing  esp.flags.flashed  esp.flags.has_c4

# World
world.spectators.enabled  world.spectators.detailed  world.spectators.self_only
world.bomb.location  world.bomb.timer  world.crosshair.enabled
world.radar.enabled  world.radar.no_rotate

# Other
settings.watermark  settings.streamproof  settings.vsync  settings.free_cpu
```

ESP colors use names such as `esp.color.box_team`, `esp.color.box_enemy`, `esp.color.skeleton_team`, `esp.color.skeleton_enemy`, `esp.color.tracer_team`, and `esp.bomb_color`. Flag colors follow the pattern `esp.color.flags.<flag>_<team|enemy>`. UI colors can also be passed to `set_color`.

For theme colors, see [`THEMES.md`](THEMES.md).

## Key names

Use `A-Z`, `0-9`, `F1-F12`, `SPACE`, `ENTER`, `ESC`, `INSERT`, `DELETE`, `HOME`, `END`, `PGUP`, `PGDN`, `LEFT`, `RIGHT`, `UP`, `DOWN`, `SHIFT`, `CTRL`, `ALT`, `TAB`, `BACKSPACE`, `NUMPAD0-NUMPAD9`, or `MOUSE1-MOUSE5`.

Numeric virtual-key codes are also accepted, for example `bind("112", "btn_setup")`.

## Troubleshooting

- **Compile errors:** the console prints `file:line:column` diagnostics after a failed load. Other scripts are not affected.
- **Button is missing:** make sure the function takes no parameters and starts with `btn_`.
- **Keybind does not work:** use a function from the same script, and check the key name.
- **Color does not change:** use RGBA values between `0.0` and `1.0` and verify the color variable name.
