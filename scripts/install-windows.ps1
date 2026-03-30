# install-windows.ps1
# Downloads the latest RC-20 Clone VST3 build from GitHub Actions and installs it.
#
# Requirements: GitHub CLI (gh) — https://cli.github.com
# Usage: Right-click → "Run with PowerShell" (or run from an elevated prompt)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ── Config ─────────────────────────────────────────────────────────────────────

$ArtifactName = "RC-20-Clone-Windows-VST3"
$PluginName   = "RC-20 Clone.vst3"
$Vst3Dir      = "C:\Program Files\Common Files\VST3"

# Derive repo from git remote so the script works on any fork
$Repo = git -C $PSScriptRoot remote get-url origin 2>$null |
        Select-String -Pattern "github\.com[:/](.+?)(?:\.git)?$" |
        ForEach-Object { $_.Matches[0].Groups[1].Value }

if (-not $Repo) {
    Write-Error "Could not determine GitHub repo from git remote. Set `$Repo manually."
}

# ── Preflight ──────────────────────────────────────────────────────────────────

if (-not (Get-Command gh -ErrorAction SilentlyContinue)) {
    Write-Error "GitHub CLI (gh) not found. Install from https://cli.github.com then run: gh auth login"
}

# Re-launch as Administrator if needed (writing to Program Files requires elevation)
if (-not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "Requesting administrator privileges..."
    Start-Process powershell "-ExecutionPolicy Bypass -File `"$PSCommandPath`"" -Verb RunAs
    exit
}

# ── Download ───────────────────────────────────────────────────────────────────

$TmpDir = Join-Path $env:TEMP "rc20-install-$([System.IO.Path]::GetRandomFileName())"
New-Item -ItemType Directory -Path $TmpDir | Out-Null

Write-Host "Fetching latest successful build from $Repo ..."

$RunId = gh run list `
    --repo $Repo `
    --branch main `
    --workflow build.yml `
    --status success `
    --limit 1 `
    --json databaseId `
    --jq ".[0].databaseId"

if (-not $RunId) {
    Remove-Item $TmpDir -Recurse -Force
    Write-Error "No successful build found on main. Check GitHub Actions."
}

Write-Host "Downloading artifact from run $RunId ..."
gh run download $RunId --repo $Repo --name $ArtifactName --dir $TmpDir

# ── Install ────────────────────────────────────────────────────────────────────

$Source = Join-Path $TmpDir $PluginName
$Dest   = Join-Path $Vst3Dir $PluginName

if (-not (Test-Path $Source)) {
    Remove-Item $TmpDir -Recurse -Force
    Write-Error "Expected '$PluginName' inside artifact but it was not found."
}

if (Test-Path $Dest) {
    Write-Host "Removing previous installation at $Dest ..."
    Remove-Item $Dest -Recurse -Force
}

Write-Host "Installing to $Dest ..."
Copy-Item $Source $Dest -Recurse

# ── Cleanup ────────────────────────────────────────────────────────────────────

Remove-Item $TmpDir -Recurse -Force

Write-Host ""
Write-Host "Installed: $Dest"
Write-Host "Rescan plugins in your DAW to pick up the new version."
