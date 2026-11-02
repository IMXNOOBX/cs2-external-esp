# Scripting

Scripts run on [Solis](https://github.com/IMXNOOBX/cs2-external-esp/tree/main/src/external/Solis).

The application loads every `.sl` file from the working directory and from a `scripts` folder next to it. The `scripts` folder is not created automatically. Each file is compiled separately, so scripts can be added or removed independently, but every file needs its own `main()`.

## Quick start

`movement.sl`:

```solis
func main() {
    bind("F1", "btn_toggle_box");
}

func btn_toggle_box() {
    set_bool("esp.box", !get_bool("esp.box"));
}
```

`main()` runs when the script loads, so that's where bindings belong. `btn_toggle_box` appears as a `toggle_box` button in the **Scripts** tab.

## Buttons

Any function without parameters whose name starts with `btn_` is shown in the GUI:

```solis
func btn_reset() {
    reset_theme();
}
```

The `btn_` prefix is removed from the button label. Functions without the prefix are not shown, but they can still be used with `bind` and `hold`.

## Multiple scripts

Each file is independent. A script can look like this:

```solis
func main() {
    bind("F1", "toggle");
    hold("SPACE", "jump");
}

func toggle() {
    set_bool("esp.skeleton", !get_bool("esp.skeleton"));
}

func jump() {
    send("SPACE", 50, 10);
}
```

`bind` and `hold` refer to functions in the same script. In the **Scripts** tab, every script is listed with its relative name (`movement` or `scripts/movement`) and its `btn_` functions.

Click a script name to open the file in the default editor. Use **Load** / **Unload** to enable a script, and **Refresh** to rescan the folders for new or removed files.

## Host API

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
| `print(message)` | Alias of `echo`. |
| `bind(key, function)` | Bind a key to a function in the same script. |
| `hold(key, function)` | Run a function while a key is held. |
| `client_base()` / `engine_base()` | Module base addresses. |
| `menu_open()`, `menu_x()`, `menu_y()` | Menu state and position. |

`bind` and `hold` are only accepted from `main()`.

The `math` and `strings` modules are available with `import math;` and `import strings;`.

### Keyboard input (debug builds)

```solis
send(key, duration_ms, repeat_count);
send("SPACE", 50, 10);
send("MOUSE1", 50, 10);
```

### Memory access (debug builds)

```solis
let value = read_int(0xABCD1234);
write_int(0xABCD1234, value + 1);
```

This directly accesses game memory and can crash the game or trigger anti-cheat detection.

## Generated prelude

Solis requires host functions to be declared with `extern func`. A `prelude.sl` containing those declarations is generated in every folder that has scripts, and is overwritten whenever a script loads. Don't edit it.

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

Use `A-Z`, `0-9`, `F1-F12`, `SPACE`, `ENTER`, `ESC`, `INSERT`, `DELETE`, `HOME`, `END`, `PGUP`, `PGDN`, `LEFT`, `RIGHT`, `UP`, `DOWN`, `SHIFT`, `CTRL`, `ALT`, `TAB`, `BACKSPACE`, `NUM0-NUM9`, or `MOUSE1-MOUSE5`.

Numeric virtual-key codes are also accepted, for example `bind("112", "btn_setup")`.

## Troubleshooting

- **Compile errors:** the console prints `file:line:column` diagnostics after a failed load. Other scripts are not affected.
- **Button is missing:** make sure the function takes no parameters and starts with `btn_`.
- **Keybind does not work:** use a function from the same script, and check the key name.
- **Color does not change:** use RGBA values between `0.0` and `1.0` and verify the color variable name.
