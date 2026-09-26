# Scripting System Documentation
<small>please note that I DID generate this documentation using AI (Claude Sonnet 4.5). I have however read it over a few times and i see no problems</small>

## ⚠️ Security Warning

**Dangerous Commands** (DEBUG MODE ONLY):
- `send` - Sends keyboard input (automation detection risk)
- `!read` - Reads game memory (can crash/trigger anti-cheat)  
- `!write` - Writes to game memory (can crash/trigger anti-cheat)

**When loading scripts containing these commands, you will see a warning message.**

These commands are only available when running in DEBUG mode and should only be used by developers who understand the risks.

## Overview

The CS2 External ESP includes a powerful scripting system with:
- **Bracket-scoped macros** for better organization
- **GUI access control** with `@` prefix
- **Color customization** with RGBA values
- **Variables and arithmetic** for dynamic configurations
- **Named key bindings** (use `SPACE`, `F1` instead of numbers)
- **Auto-reload** when script file is modified

## Quick Start

### Basic Macro
```
macro @my_config {
    set esp.box true
    set esp.health true
}

bind F1 @my_config
```

### GUI Access Control
```
macro @user_visible {     # Shows in GUI
    set esp.box true
}

macro internal_helper {    # Hidden from GUI, keybind-only
    set world.radar.enabled true
}
```

## Script Syntax

### Comments
```
# This is a comment
```

### Macros

**Bracket Syntax (Recommended):**
```
macro @name {
    command
    command
}
```

**With Stop Key:**
```
macro @bhop {
    send SPACE 50 999999
} SPACE

# Press SPACE to stop the macro
```

**Old Style (Still Works):**
```
macro name
    command
    command
```

**@ Prefix Rules:**
- With `@`: Visible in GUI and keybinds
- Without `@`: Keybind-only, hidden from GUI

**Stop Key (Optional):**
- Add a key name after the closing `}` to make that key stop the macro
- Useful for infinite loops or long-running sequences
- Example: `} ESC` makes ESC key stop the macro
- Without stop key, macro runs until completion

### Variables
```
var radius 1000
var doubled $radius*2
set world.radar.range $doubled
```

### Arithmetic
```
var result $a+$b      # Addition
var result $a-$b      # Subtraction
var result $a*$b      # Multiplication
var result $a/$b      # Division
```

## Commands

### set
Set configuration value (supports arithmetic).
```
set esp.box true
set world.radar.range 1500
set world.radar.range $var*100
```

### setcolor
Set RGBA color (0.0 to 1.0).
```
setcolor esp.color.box_enemy 1.0 0.0 0.0 1.0
```

### var
Create/update variable.
```
var name value
var result $a+$b
```

### get
Retrieve config value into variable.
```
get esp.box current_state
echo Current state: $current_state
```

### echo
Print to console (debug mode only).
```
echo Hello World
var x 42
echo Value: $x
```

### run
Execute another macro by name.
```
run helper_macro
run setup_colors
```

### send (⚠️ DEBUG ONLY)
Send keyboard input to the game. **Only available in debug mode.**
```
send KEY [duration_ms] [repeat_count]

# Examples:
send SPACE           # Single space press (50ms default)
send SPACE 100       # Hold space for 100ms
send SPACE 50 10     # Press space 10 times (bhop)
send W 1000          # Hold W for 1 second

# Bhop with stop key:
macro @bhop {
    send SPACE 50 999999
} SPACE

bind B bhop
# Press B to start, SPACE to stop
```

**WARNING**: Can trigger anti-cheat detection!

### !read (⚠️ DEBUG ONLY)
Read memory address into variable. **Only available in debug mode.**
```
!read variable_name 0xABCD1234
```

**WARNING**: Direct memory access - use with extreme caution!

### !write (⚠️ DEBUG ONLY)
Write variable value to memory address. **Only available in debug mode.**
```
var myvalue 100
!write myvalue 0xABCD1234
```

**WARNING**: Can crash the game or trigger anti-cheat!

### bind
```
echo Hello World
var x 42                                                    
echo Value: $x
```

### bind
Bind macro to key.
```
bind F1 @macro_name
bind SPACE @toggle
bind A internal_macro
```

**Key Names:** `A-Z`, `0-9`, `F1-F12`, `SPACE`, `ENTER`, `ESC`, `INSERT`, `DELETE`, `HOME`, `END`, `PGUP`, `PGDN`, `LEFT`, `RIGHT`, `UP`, `DOWN`, `SHIFT`, `CTRL`, `ALT`, `TAB`, `BACKSPACE`, `NUM0-NUM9`, `MOUSE1-MOUSE5`

**Numeric codes also work:** `bind 112 @macro` (F1 = 112)

## Configuration Variables

### ESP Settings
`enabled`, `esp.team`, `esp.box`, `esp.skeleton`, `esp.health`, `esp.health_number`, `esp.armor`, `esp.spotted`, `esp.bomb`, `esp.tracers`, `esp.head_tracker`

### ESP Flags
`esp.flags.name`, `esp.flags.ping`, `esp.flags.money`, `esp.flags.weapon`, `esp.flags.ammo`, `esp.flags.reloading`, `esp.flags.scoped`, `esp.flags.defusing`, `esp.flags.flashed`, `esp.flags.has_c4`

### World Settings
`world.spectators.enabled`, `world.spectators.detailed`, `world.spectators.self_only`, `world.bomb.location`, `world.bomb.timer`, `world.bomb.hud`, `world.crosshair.enabled`, `world.radar.enabled`, `world.radar.no_rotate`, `world.radar.range`

### Other Settings
`settings.watermark`, `settings.streamproof`, `settings.vsync`, `settings.free_cpu`

### Colors
`esp.color.box_team`, `esp.color.box_enemy`, `esp.color.skeleton_team`, `esp.color.skeleton_enemy`, `esp.color.tracker_team`, `esp.color.tracker_enemy`, `esp.color.tracer_team`, `esp.color.tracer_enemy`, `esp.bomb_color`

**Flag Colors:** `esp.color.flags.flashed_team/enemy`, `esp.color.flags.reloading_team/enemy`, `esp.color.flags.defusing_team/enemy`, `esp.color.flags.scoped_team/enemy`, `esp.color.flags.c4_team/enemy`

## Examples

### Complete Configuration
```
macro @competitive {
    set esp.box true
    set esp.health true
    set esp.spotted true
    setcolor esp.color.box_enemy 1.0 0.0 0.0 1.0
}
```

### Color Presets
```
macro @red_theme {
    setcolor esp.color.box_enemy 1.0 0.0 0.0 1.0
    setcolor esp.color.skeleton_enemy 1.0 0.2 0.2 1.0
}

macro @neon_theme {
    setcolor esp.color.box_enemy 1.0 0.0 1.0 1.0
    setcolor esp.color.skeleton_enemy 0.0 1.0 0.5 1.0
}
```

### Variables & Math
```
macro @dynamic_radar {
    var base 1000
    var multiplier 2
    set world.radar.range $base*$multiplier
    echo Radar set to: $base*$multiplier
}
```

### Internal Helpers
```
# User-facing (GUI visible)
macro @full_setup {
    run enable_visuals
    run enable_world
    run apply_colors
}

# Internal (GUI hidden)
macro enable_visuals {
    set esp.box true
    set esp.skeleton true
}

macro enable_world {
    set world.radar.enabled true
}

macro apply_colors {
    setcolor esp.color.box_enemy 1.0 0.0 0.0 1.0
}
```

### Keybinds
```
bind F1 @competitive    # GUI macro
bind F2 enable_visuals  # Internal macro
bind SPACE @full_setup  # Quick access
```

## GUI Usage

### Macros Tab
- Click macro buttons to execute
- Only `@` prefixed macros appear
- Hover for command preview
- Manual input field for typing `@macro_name`

### Manual Execution
1. Type `@macro_name` in input field
2. Click Execute
3. Only works with `@` prefixed macros

## Auto-Reload

The script file is monitored and reloads automatically when modified:
1. Edit `scripts.txt`
2. Save changes
3. Changes apply immediately
4. New macros/keybinds available instantly

Or click "Reload Scripts Now" in GUI.

## Best Practices

1. **Use @ for user actions** - Main operations
2. **Skip @ for helpers** - Internal functions
3. **Use key names** - More readable than numbers
4. **Add comments** - Document your macros
5. **Group related macros** - Organize by category
6. **Use variables** - For reusable values
7. **Test incrementally** - Change one thing at a time

## Troubleshooting

**Macro not in GUI?**
- Check for `@` prefix in definition
- Reload scripts manually
- Check console for errors

**Keybind not working?**
- Verify key name spelling
- Try numeric code temporarily
- Check macro name is correct

**Colors not changing?**
- Use 0.0 to 1.0 range (not 0-255)
- Check variable name is correct
- Verify it's a color variable

## Quick Reference

```
# Macros
macro @name { commands }        # GUI visible
macro name { commands }          # GUI hidden

# Commands
set variable value               # Set config
setcolor var r g b a            # Set color
var name value                  # Create variable
get config_var var_name         # Retrieve value
echo message                    # Print to console
run macro_name                  # Execute macro
bind KEY macro_name             # Bind key

# DEBUG ONLY (⚠️ Dangerous)
send KEY [duration] [repeat]    # Send input
!read var address               # Read memory
!write var address              # Write memory

# Variables
$variable                       # Use variable
$a+$b                          # Arithmetic
```

## Script File Location

`scripts.txt` in the application directory.
