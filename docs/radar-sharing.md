# Share the radar

Enable **Browser radar** under **World > Radar**, then click **Share radar**.

Once the link appears, click **Copy share link** and send it to whoever you want to share with. Anyone with the link can view your radar through Cloudflare.

This uses [Cloudflare Quick Tunnels](https://developers.cloudflare.com/tunnel/get-started/quick-tunnels/), intended for temporary testing, with no uptime guarantee.

## Manual install

If the automated installer fails, or you are being rate limited, you can manually install the cloudflared binary from https://github.com/cloudflare/cloudflared/releases.

Download `cloudflared-windows-amd64.exe`, rename it to `cloudflared.exe`, and place it in the `cloudflare` folder beside the executable.

```
cs2-external-esp.exe
cloudflare/
    share-radar.ps1
    cloudflared.exe
```
