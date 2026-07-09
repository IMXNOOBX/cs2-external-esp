# UI Theme System
<small>please note that I DID generate this documentation using AI (Claude Sonnet 4.5). I have however read it over a few times and i see no problems</small>

The CS2 External ESP supports custom UI themes through simple text files. Themes are **sandboxed** - they can only modify colors and cannot execute any code, making them safe to share and download.

## Quick Start

1. Place `.txt` theme files in the `assets/themes/` folder
2. Restart the application or reload themes from Settings tab
3. Select your theme from the dropdown

## Creating a Theme

### Theme File Format

Theme files are simple text files with key-value pairs. Example:

```
# My Custom Theme
# Lines starting with # are comments

name My Awesome Theme
author YourName
description A cool dark theme with red accents

# Window background (RGBA values from 0.0 to 1.0)
windowbg 0.1 0.1 0.1 1.0

# Buttons
button 0.8 0.0 0.0 1.0
buttonhovered 1.0 0.0 0.0 1.0
buttonactive 0.6 0.0 0.0 1.0

# Text
text 1.0 1.0 1.0 1.0
```

### Metadata Fields

```
name Theme Name Here
author Your Name
description Short description of the theme
```

### Color Format

```
colorname red green blue alpha
```

- Color values are floats from `0.0` to `1.0`
- Values outside this range are automatically clamped
- Color names are case-insensitive

## Available Colors

### Text
- `text` - Main text color
- `textdisabled` - Disabled/grayed out text
- `textselectedbg` - Background for selected text

### Windows & Backgrounds
- `windowbg` - Main window background
- `childbg` - Child window background
- `popupbg` - Popup window background
- `menubarbg` - Menu bar background

### Borders
- `border` - Border color
- `bordershadow` - Border shadow (usually transparent)

### Frames (Input boxes, checkboxes, etc.)
- `framebg` - Frame background
- `framebghovered` - Frame background when hovered
- `framebgactive` - Frame background when active/clicked

### Title Bar
- `titlebg` - Title bar background (unfocused)
- `titlebgactive` - Title bar background (focused)
- `titlebgcollapsed` - Title bar when collapsed

### Scrollbar
- `scrollbarbg` - Scrollbar track background
- `scrollbargrab` - Scrollbar handle
- `scrollbargrabhovered` - Scrollbar handle when hovered
- `scrollbargrabactive` - Scrollbar handle when dragged

### Interactive Elements
- `checkmark` - Checkbox checkmark, radio button dot
- `slidergrab` - Slider handle
- `slidergrabactive` - Slider handle when dragged
- `button` - Button background
- `buttonhovered` - Button when hovered
- `buttonactive` - Button when clicked

### Headers & Collapsibles
- `header` - Collapsing header background
- `headerhovered` - Header when hovered
- `headeractive` - Header when clicked

### Separators
- `separator` - Separator line
- `separatorhovered` - Separator when hovered
- `separatoractive` - Separator when dragged

### Resize Grip
- `resizegrip` - Window resize handle
- `resizegriphovered` - Resize handle when hovered
- `resizegripactive` - Resize handle when dragged

### Tabs
- `tab` - Inactive tab
- `tabhovered` - Tab when hovered
- `tabactive` - Active tab
- `tabunfocused` - Inactive tab (window unfocused)
- `tabunfocusedactive` - Active tab (window unfocused)

### Docking
- `dockingpreview` - Docking preview overlay
- `dockingemptybg` - Empty docking area background

### Plots & Graphs
- `plotlines` - Plot line color
- `plotlineshovered` - Plot line when hovered
- `plothistogram` - Histogram bar color
- `plothistogramhovered` - Histogram bar when hovered

### Tables
- `tableheaderbg` - Table header background
- `tablebordersstrong` - Strong table borders
- `tableborderslight` - Light table borders
- `tablerowbg` - Table row background
- `tablerowbgalt` - Alternating table row background

### Drag & Drop
- `dragdroptarget` - Drag & drop target highlight

### Navigation
- `navhighlight` - Navigation highlight
- `navwindowinghighlight` - Window navigation highlight
- `navwindowingdimbg` - Dimmed background during window navigation

### Modal
- `modalwindowdimbg` - Dimmed background for modal windows

## Tips

### Color Conversion

If you have RGB values in 0-255 range, divide by 255:
- RGB(255, 128, 0) → `1.0 0.5 0.0`
- RGB(51, 51, 51) → `0.2 0.2 0.2`

### Hex to Float

| Hex | Decimal | Float |
|-----|---------|-------|
| 00  | 0       | 0.0   |
| 40  | 64      | 0.25  |
| 80  | 128     | 0.5   |
| C0  | 192     | 0.75  |
| FF  | 255     | 1.0   |

### Quick Theme Creation

1. Open the application
2. In Settings → Themes, click "Export Current Theme"
3. Edit the generated file in `assets/themes/your_theme.txt`
4. Reload themes and select your edited theme

### Theme Testing

- You don't need to restart the app to test changes
- Just reload themes from the Settings tab
- Make small color adjustments and reload to see changes

## Example Themes

### Minimal Dark
```
name Minimal Dark
windowbg 0.0 0.0 0.0 1.0
text 1.0 1.0 1.0 1.0
button 0.2 0.2 0.2 1.0
buttonhovered 0.3 0.3 0.3 1.0
checkmark 0.0 1.0 0.0 1.0
```

### Neon Green
```
name Neon Green
windowbg 0.0 0.05 0.0 1.0
text 0.0 1.0 0.0 1.0
button 0.0 0.5 0.0 1.0
buttonhovered 0.0 0.7 0.0 1.0
checkmark 0.0 1.0 0.0 1.0
framebg 0.0 0.2 0.0 1.0
```

### Red Alert
```
name Red Alert
windowbg 0.1 0.0 0.0 1.0
text 1.0 0.9 0.9 1.0
button 0.8 0.0 0.0 1.0
buttonhovered 1.0 0.0 0.0 1.0
checkmark 1.0 0.0 0.0 1.0
border 0.8 0.0 0.0 0.5
```

## Safety

Theme files are **completely safe**:
- ✅ Only parse color values (4 floats per line)
- ✅ Cannot execute code
- ✅ Cannot access filesystem
- ✅ Cannot modify game memory
- ✅ Values are automatically clamped to safe ranges
- ✅ Invalid lines are skipped with warnings
- ✅ Sandboxed from main scripting system

You can safely download and use themes from others without security concerns.

## Sharing Themes

To share your theme:
1. Create your theme in `assets/themes/my_theme.txt`
2. Add proper name, author, and description metadata
3. Share the `.txt` file

To use someone else's theme:
1. Download the `.txt` file
2. Place it in `assets/themes/` folder
3. Reload themes from Settings tab
4. Select the new theme

## Troubleshooting

**Theme not appearing in list:**
- Make sure the file has a `.txt` extension
- Check that it's in the `assets/themes/` folder
- Make sure it has a `name` field

**Colors not applying:**
- Check console for warnings about invalid color names
- Ensure RGBA values are in 0.0-1.0 range
- Color names must match exactly (case-insensitive)

**Theme looks wrong:**
- Some colors depend on others for contrast
- Try setting both foreground and background colors
- Check the default themes for reference

## Advanced

### Complete Theme Template

See `assets/themes/default_dark.txt` for a complete theme with all 55 colors defined.

### Partial Themes

You don't need to define all colors - undefined colors keep their current values:

```
# Quick accent color change
name Blue Accent
checkmark 0.0 0.5 1.0 1.0
slidergrab 0.0 0.5 1.0 1.0
button 0.0 0.3 0.7 1.0
```

### Color Consistency

For a cohesive theme, use consistent color families:
- Button states: use same hue, vary brightness
- Text: ensure contrast with backgrounds (4.5:1 ratio minimum)
- Accents: use sparingly for important elements

## Resources

- [ImGui Color Demo](https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp) - Shows all color slots
- [Color Contrast Checker](https://webaim.org/resources/contrastchecker/) - Ensure readable text
- [Coolors.co](https://coolors.co/) - Generate color palettes
