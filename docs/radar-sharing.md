# Share the radar

Enable **Browser radar** under **World > Radar**, then click **Share radar**.

Once the link appears, click **Copy share link** and send it to whoever you want to share with. Anyone with the link can view your radar through Cloudflare.

This uses [Cloudflare Quick Tunnels](https://developers.cloudflare.com/tunnel/get-started/quick-tunnels/), intended for temporary testing, with no uptime guarantee.

## cloudflared

Sharing is powered by Cloudflare's `cloudflared` binary. Release archives ship the latest Windows build as `cloudflared.exe` next to the program, so nothing is downloaded or installed at runtime:

```
cs2-external-esp.exe
cloudflared.exe
```

The program never fetches or installs anything. It only starts `cloudflared.exe` when you click **Share radar**, so if you do not use that feature you can delete the file. Sharing then reports the missing binary in the menu, and the rest of the program is unaffected.

## Already installed system-wide?

A copy next to the program is optional. If you installed cloudflared yourself, the program uses it. Resolution order:

1. `cloudflared.exe` beside `cs2-external-esp.exe`
2. `cloudflare\cloudflared.exe` beside it
3. `%ProgramFiles(x86)%\cloudflared\cloudflared.exe` — official MSI and `winget install --id Cloudflare.cloudflared`
4. `%ProgramFiles%\cloudflared\cloudflared.exe`
5. `%LOCALAPPDATA%\Microsoft\WinGet\Links\cloudflared.exe` — older winget portable installs
6. Any folder on `PATH` containing `cloudflared.exe` — Chocolatey, Scoop, or a manual install

The current working directory is never searched. If you install cloudflared while the program is already running, restart it so the new `PATH` becomes visible.

## Manual install

Download `cloudflared-windows-amd64.exe` from https://github.com/cloudflare/cloudflared/releases, rename it to `cloudflared.exe`, and place it either beside the executable or in a `cloudflare` folder beside it:

```
cs2-external-esp.exe
cloudflared.exe
cloudflare/
    cloudflared.exe
```
