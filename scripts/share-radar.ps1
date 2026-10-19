param(
    [Parameter(Mandatory = $true)]
    [string]$RadarUrl
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
[Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)

try {
    try {
        $response = Invoke-WebRequest -Uri ($RadarUrl + 'state') -UseBasicParsing -TimeoutSec 3 -MaximumRedirection 0
        $state = $response.Content | ConvertFrom-Json
        if ($null -eq $state.in_match -or $null -eq $state.players -or $null -eq $state.map) {
            throw 'Unexpected radar response.'
        }
    } catch {
        throw 'Start the app and enable Browser radar under World > Radar before sharing.'
    }

    $cache = $PSScriptRoot
    $executable = Join-Path $cache 'cloudflared.exe'
    if (!(Test-Path -LiteralPath $executable -PathType Leaf)) {
        if (![Environment]::Is64BitOperatingSystem) {
            throw 'The automatic download requires 64-bit Windows.'
        }
        Write-Output 'Downloading cloudflared...'
        $temporary = $null
        try {
            $release = Invoke-RestMethod -Uri 'https://api.github.com/repos/cloudflare/cloudflared/releases/latest' -TimeoutSec 20
            $asset = $release.assets | Where-Object name -EQ 'cloudflared-windows-amd64.exe' | Select-Object -First 1
            if ($asset.digest -notmatch '^sha256:[a-fA-F0-9]{64}$' -or
                $asset.browser_download_url -notmatch '^https://github\.com/cloudflare/cloudflared/releases/download/[^/]+/cloudflared-windows-amd64\.exe$') {
                throw 'The release has no verifiable Windows download.'
            }
            [IO.Directory]::CreateDirectory($cache) | Out-Null
            $temporary = Join-Path $cache ('cloudflared-' + [Guid]::NewGuid().ToString() + '.download')
            Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $temporary -UseBasicParsing -TimeoutSec 120
            Write-Output 'Verifying download...'
            if ((Get-FileHash -LiteralPath $temporary -Algorithm SHA256).Hash -ine $asset.digest.Substring(7)) {
                throw 'The downloaded file failed its SHA-256 check.'
            }
            Move-Item -LiteralPath $temporary -Destination $executable
        } catch {
            $response = $_.Exception.Response

            if ($null -ne $response -and [int]$response.StatusCode -eq 403) {
                throw "Github denied the download (403). Try again later or install cloudflared manually, see docs/radar-sharing.md."
            }

            throw "Could not download cloudflared: $($_.Exception.Message) See docs/radar-sharing.md"
        } finally {
            if ($temporary -and (Test-Path -LiteralPath $temporary)) {
                Remove-Item -LiteralPath $temporary
            }
        }
    }

    Write-Output 'Connecting to Cloudflare...'

    # Keep the local server's Host check intact.
    & $executable tunnel --output json --loglevel info --url http://127.0.0.1:27080 --http-host-header 127.0.0.1:27080
    if ($LASTEXITCODE -ne 0) {
        throw "Cloudflare tunnel exited (code $LASTEXITCODE)."
    }
} catch {
    Write-Output ('ERROR: ' + $_.Exception.Message)
    exit 1
}
