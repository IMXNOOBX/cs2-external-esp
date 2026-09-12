# Scripting

<small>Script documentation generated with AI assistance and reviewed for accuracy.</small>

Scripts are loaded from `scripts.esp` in the application directory. The file is reloaded automatically when it changes; it can also be reloaded from the **Macros** tab.

## Quick start

```text
# Setups basic esp features on click or when pressing F1
macro @setup { 
    set esp.box true
    set esp.health true
    setcolor esp.color.box_enemy 1.0 0.0 0.0 1.0
}

bind F1 setup
```

Macros beginning with `@` are shown in the GUI. The prefix is only part of the declaration: the stored macro name is `setup`, so use `setup` (without `@`) when running it or binding it to a key. Other macros are hidden helpers that can still be called by another macro or keybind.

## Syntax

### Macros

```text
macro @name {
    command
    command
}
```

Use `hold KEY macro_name` to make a macro run while a key is physically held. The macro starts when the key is pressed and releasing it stops repeated `send` input. Define the macro before its `hold` declaration.

```text
# Pressing and holding SPACE runs this invisible macro.
macro hold_jump {
    send SPACE 50 999999
}

hold SPACE hold_jump
```
Use `#` for comments. The older unbracketed macro format is still supported.

### Variables and expressions

```text
var radius 1000
var doubled $radius*2
set world.radar.range $doubled
```

Supported arithmetic operators are `+`, `-`, `*`, and `/`. Use `$name` to substitute a variable.

## Commands

| Command | Description | Example |
| --- | --- | --- |
| `set` | Set a configuration value. | `set esp.box true` |
| `setcolor` | Set an RGBA color from `0.0` to `1.0`. | `setcolor esp.color.box_enemy 1 0 0 1` |
| `resettheme` | Restore the default UI colors. | `resettheme` |
| `var` | Create or update a variable. | `var range 1500` |
| `get` | Read a configuration value into a variable. | `get esp.box current` |
| `echo` | Print a message to the console. | `echo Range: $range` |
| `run` | Run another macro. | `run setup` |
| `bind` | Bind a macro to a key; omit `@` from the macro name. | `bind F1 setup` |
| `hold` | Run a macro while a physical key is held; define the macro first. | `hold SPACE hold_jump` |

For UI themes and the full list of UI color names, see [`THEMES.md`](THEMES.md).

### Global variables

The following read-only values are updated automatically and can be used with `$name`:

```text
$client.base    # client.dll base address, in hexadecimal
$engine2.base   # engine2.dll base address, in hexadecimal
$menu.open          # true while the menu is open
$menu.pos.x         # menu X position
$menu.pos.y         # menu Y position
```

For example:

```text
echo Menu position: $menu.pos.x $menu.pos.y
```

Global variables cannot be changed with `var`; their values are refreshed by the application.

### Keyboard input

`send` is available in debug builds only and sends keyboard or mouse-button input:

```text
send KEY [duration_ms] [repeat_count]
send SPACE 50 10
send MOUSE1 50 10
```

### Memory commands  (debug builds only)

`!read` and `!write` are experimental commands under testing and are available in debug builds only. They directly access game memory and can crash the game or trigger anti-cheat detection.

```text
!read variable_name 0xABCD1234

var value 100
!write value 0xABCD1234
```

Use these commands only for development/testing when you understand the risks. Scripts containing dangerous commands produce a warning when loaded.

## Configuration values

Common values include:

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
world.radar.enabled  world.radar.no_rotate  world.radar.range

# Other
settings.watermark  settings.streamproof  settings.vsync  settings.free_cpu
```

ESP colors use names such as `esp.color.box_team`, `esp.color.box_enemy`, `esp.color.skeleton_team`, `esp.color.skeleton_enemy`, `esp.color.tracer_team`, and `esp.bomb_color`. Flag colors follow the pattern `esp.color.flags.<flag>_<team|enemy>`.

## GUI

Open the **Macros** tab to:

- run GUI-visible (`@`) macros;
- hover a macro to preview its commands;
- enter an `@macro_name` manually and click **Execute**;
- reload the script or open it in the default text editor.

## Key names

Use `A-Z`, `0-9`, `F1-F12`, `SPACE`, `ENTER`, `ESC`, `INSERT`, `DELETE`, `HOME`, `END`, `PGUP`, `PGDN`, `LEFT`, `RIGHT`, `UP`, `DOWN`, `SHIFT`, `CTRL`, `ALT`, `TAB`, `BACKSPACE`, `NUM0-NUM9`, or `MOUSE1-MOUSE5`.

Numeric virtual-key codes are also accepted, for example `bind 112 setup` for F1.

## Troubleshooting

- **Macro is missing from the GUI:** add `@` to its name, then reload the script.
- **Keybind does not work:** use the macro name without the `@` GUI marker (for example, `bind F1 setup` for `macro @setup`), then check the key name. Numeric key codes can be used as a fallback.
- **Color does not change:** use RGBA values between `0.0` and `1.0` and verify the color variable name.

For theme packs and color customization, see [`THEMES.md`](THEMES.md).
