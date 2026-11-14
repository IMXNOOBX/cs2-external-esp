# UI Theme System

<small>Theme documentation generated with AI assistance and reviewed for accuracy.</small>

The project now supports custom UI themes through the **scripting API**. Themes have been merged into the scripting system, so there is no separate theme format or theme loader.

For scripting syntax and commands, see the [`./SCRIPTING.md`](./SCRIPTING.md).

# Preset theme

[`assets/themes/themes.sl`](assets/themes/themes.sl) contains the midnight, light and matrix presets plus a reset. Release builds also ship it as `scripts/themes.sl`, so it is available in the **Scripts** tab out of the box.

Copy the file to the working directory or the `scripts` folder to use it in a source build.

## Creating a Theme

Themes are scripts built on `host.set_color`. Put each theme in its own `.sl` file in the working directory or in the `scripts` folder:

```solis
import host;

func main() {
    // Runs when the script loads.
}

func btn_my_theme() {
    host.reset_theme();

    host.set_color("windowbg", 0.05, 0.05, 0.05, 1.0);
    host.set_color("text", 1.0, 1.0, 1.0, 1.0);
    host.set_color("button", 0.2, 0.2, 0.2, 1.0);
    host.set_color("buttonhovered", 0.3, 0.3, 0.3, 1.0);
    host.set_color("checkmark", 0.0, 1.0, 0.0, 1.0);
}
```

The `btn_` prefix makes it appear as a button in the **Scripts** tab. You can also bind it to a key in `main`:

```solis
func main() {
    host.bind("F1", "btn_my_theme");
}
```

Using `host.reset_theme()` at the start is recommended so the theme always starts from the default UI and does not inherit colors from a previous theme.

## Color Format

```text
host.set_color("COLOR", R, G, B, A)
```

Colors use RGBA values from `0.0` to `1.0`.

For example:

```solis
host.set_color("button", 1.0, 0.0, 0.0, 1.0);
```

# Available Colors

The following UI color names are available to themes.

## Text

* `text` — Main text color
* `textdisabled` — Disabled/grayed-out text
* `textselectedbg` — Background for selected text

## Windows & Backgrounds

* `windowbg` — Main window background
* `childbg` — Child window background
* `popupbg` — Popup window background
* `menubarbg` — Menu bar background

## Borders

* `border` — Border color
* `bordershadow` — Border shadow, usually transparent

## Frames

* `framebg` — Frame background
* `framebghovered` — Frame background when hovered
* `framebgactive` — Frame background when active/clicked

## Title Bar

* `titlebg` — Title bar background when unfocused
* `titlebgactive` — Title bar background when focused
* `titlebgcollapsed` — Title bar when collapsed

## Scrollbar

* `scrollbarbg` — Scrollbar track background
* `scrollbargrab` — Scrollbar handle
* `scrollbargrabhovered` — Scrollbar handle when hovered
* `scrollbargrabactive` — Scrollbar handle when dragged

## Interactive Elements

* `checkmark` — Checkbox checkmark and radio button dot
* `slidergrab` — Slider handle
* `slidergrabactive` — Slider handle when dragged
* `button` — Button background
* `buttonhovered` — Button when hovered
* `buttonactive` — Button when clicked

## Headers & Collapsibles

* `header` — Collapsing header background
* `headerhovered` — Header when hovered
* `headeractive` — Header when clicked

## Separators

* `separator` — Separator line
* `separatorhovered` — Separator when hovered
* `separatoractive` — Separator when dragged

## Resize Grip

* `resizegrip` — Window resize handle
* `resizegriphovered` — Resize handle when hovered
* `resizegripactive` — Resize handle when dragged

## Tabs

* `tab` — Inactive tab
* `tabhovered` — Tab when hovered
* `tabactive` — Active tab
* `tabunfocused` — Inactive tab while the window is unfocused
* `tabunfocusedactive` — Active tab while the window is unfocused

## Docking

* `dockingpreview` — Docking preview overlay
* `dockingemptybg` — Empty docking area background

## Plots & Graphs

* `plotlines` — Plot line color
* `plotlineshovered` — Plot line when hovered
* `plothistogram` — Histogram bar color
* `plothistogramhovered` — Histogram bar when hovered

## Tables

* `tableheaderbg` — Table header background
* `tablebordersstrong` — Strong table borders
* `tableborderslight` — Light table borders
* `tablerowbg` — Table row background
* `tablerowbgalt` — Alternating table row background

## Drag & Drop

* `dragdroptarget` — Drag-and-drop target highlight

## Navigation

* `navhighlight` — Navigation highlight
* `navwindowinghighlight` — Window navigation highlight
* `navwindowingdimbg` — Dimmed background during window navigation

## Modal Windows

* `modalwindowdimbg` — Dimmed background for modal windows


You don't need to define every color. Any color not changed by your theme keeps its current value.

## Testing & Sharing

Themes live in their own `.sl` file, in the working directory or the `scripts` folder. Unload and load the theme script to apply changes.

To share a theme, share the `.sl` file and have the user drop it into either location.

For the host API, scripts, and keybinds, see the [`./SCRIPTING.md`](./SCRIPTING.md).
