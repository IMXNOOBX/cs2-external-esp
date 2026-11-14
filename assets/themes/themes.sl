import host;

func main() {
    host.echo("[theme] loaded");
    print("[theme] buttons: theme_midnight, theme_light, theme_matrix, theme_reset");
}

func btn_theme_midnight() {
    host.reset_theme();

    host.set_color("windowbg", 0.04, 0.05, 0.09, 1.0);
    host.set_color("childbg", 0.06, 0.07, 0.12, 1.0);
    host.set_color("popupbg", 0.06, 0.07, 0.12, 1.0);
    host.set_color("border", 0.16, 0.20, 0.34, 1.0);
    host.set_color("text", 0.86, 0.90, 1.0, 1.0);
    host.set_color("textdisabled", 0.45, 0.50, 0.64, 1.0);
    host.set_color("button", 0.12, 0.16, 0.28, 1.0);
    host.set_color("buttonhovered", 0.18, 0.24, 0.42, 1.0);
    host.set_color("buttonactive", 0.24, 0.32, 0.56, 1.0);
    host.set_color("checkmark", 0.35, 0.70, 1.0, 1.0);
    host.set_color("slidergrab", 0.35, 0.70, 1.0, 1.0);
    host.set_color("header", 0.10, 0.13, 0.24, 1.0);
    host.set_color("tab", 0.10, 0.13, 0.24, 1.0);
    host.set_color("tabactive", 0.18, 0.24, 0.42, 1.0);

    print("[theme] midnight theme applied");
}

func btn_theme_light() {
    host.reset_theme();

    host.set_color("windowbg", 0.94, 0.94, 0.94, 1.0);
    host.set_color("childbg", 0.90, 0.90, 0.90, 1.0);
    host.set_color("popupbg", 0.94, 0.94, 0.94, 1.0);
    host.set_color("border", 0.72, 0.72, 0.72, 1.0);
    host.set_color("text", 0.10, 0.10, 0.10, 1.0);
    host.set_color("textdisabled", 0.45, 0.45, 0.45, 1.0);
    host.set_color("button", 0.82, 0.82, 0.82, 1.0);
    host.set_color("buttonhovered", 0.76, 0.76, 0.76, 1.0);
    host.set_color("buttonactive", 0.68, 0.68, 0.68, 1.0);
    host.set_color("checkmark", 0.10, 0.45, 0.85, 1.0);
    host.set_color("slidergrab", 0.10, 0.45, 0.85, 1.0);
    host.set_color("header", 0.85, 0.85, 0.85, 1.0);
    host.set_color("tab", 0.85, 0.85, 0.85, 1.0);
    host.set_color("tabactive", 0.74, 0.74, 0.74, 1.0);

    print("[theme] light theme applied");
}

func btn_theme_matrix() {
    host.reset_theme();

    host.set_color("windowbg", 0.01, 0.03, 0.01, 1.0);
    host.set_color("childbg", 0.02, 0.05, 0.02, 1.0);
    host.set_color("border", 0.05, 0.30, 0.10, 1.0);
    host.set_color("text", 0.35, 1.0, 0.45, 1.0);
    host.set_color("textdisabled", 0.16, 0.55, 0.24, 1.0);
    host.set_color("button", 0.03, 0.16, 0.06, 1.0);
    host.set_color("buttonhovered", 0.06, 0.26, 0.10, 1.0);
    host.set_color("buttonactive", 0.10, 0.38, 0.15, 1.0);
    host.set_color("checkmark", 0.20, 1.0, 0.35, 1.0);
    host.set_color("slidergrab", 0.20, 1.0, 0.35, 1.0);
    host.set_color("header", 0.03, 0.14, 0.06, 1.0);
    host.set_color("separator", 0.08, 0.35, 0.13, 1.0);

    print("[theme] matrix theme applied");
}

func btn_theme_reset() {
    host.reset_theme();
    print("[theme] theme reset to defaults");
}
