# Share the radar

Enable **Browser radar** under **World > Radar**, then click **Share radar**. An animated loading bar shows startup progress. The URL is read from this tunnel's startup banner and shown after a connection is registered. Use **Copy share link** to share it. Anyone with that link can view the radar; traffic passes through Cloudflare.

On first use, the script downloads cloudflared from Cloudflare's official GitHub release, verifies its SHA-256 checksum, and saves it as `cloudflare\cloudflared.exe` beside the app. Later runs reuse that file. No account, API key or administrator rights are needed.

Sharing runs in the background. Click **Stop sharing** to stop it. Disabling Browser radar or closing the app also stops sharing. Each run creates a new URL. **Regenerate link** restarts the tunnel and replaces the current link.

Build **Release | x64** after applying the UI changes. The build places `share-radar.ps1` in the `cloudflare` folder beside the executable. Keep that folder with the app; it must be writable for the first download. The script can also be run directly from PowerShell.

This uses [Cloudflare Quick Tunnels](https://developers.cloudflare.com/tunnel/get-started/quick-tunnels/), intended for temporary testing, with no uptime guarantee.
