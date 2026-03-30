#!/usr/bin/env bash
# install-macos.sh
# Downloads the latest RC-20 Clone build from GitHub Actions and installs
# both the VST3 and AU component.
#
# Requirements: GitHub CLI (gh) — https://cli.github.com
# Usage: bash scripts/install-macos.sh

set -euo pipefail

# ── Config ─────────────────────────────────────────────────────────────────────

ARTIFACT_VST3="RC-20-Clone-macOS-VST3"
ARTIFACT_AU="RC-20-Clone-macOS-AU"
PLUGIN_VST3="RC-20 Clone.vst3"
PLUGIN_AU="RC-20 Clone.component"
VST3_DIR="/Library/Audio/Plug-Ins/VST3"
AU_DIR="/Library/Audio/Plug-Ins/Components"

# Derive repo from git remote so the script works on any fork
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO=$(git -C "$SCRIPT_DIR" remote get-url origin 2>/dev/null \
    | sed -E 's|.*github\.com[:/]||; s|\.git$||')

if [[ -z "$REPO" ]]; then
    echo "Error: could not determine GitHub repo from git remote." >&2
    exit 1
fi

# ── Preflight ──────────────────────────────────────────────────────────────────

if ! command -v gh &>/dev/null; then
    echo "Error: GitHub CLI (gh) not found." >&2
    echo "Install: brew install gh  then run: gh auth login" >&2
    exit 1
fi

# ── Download ───────────────────────────────────────────────────────────────────

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

echo "Fetching latest successful build from $REPO ..."

RUN_ID=$(gh run list \
    --repo "$REPO" \
    --branch main \
    --workflow build.yml \
    --status success \
    --limit 1 \
    --json databaseId \
    --jq ".[0].databaseId")

if [[ -z "$RUN_ID" ]]; then
    echo "Error: no successful build found on main. Check GitHub Actions." >&2
    exit 1
fi

echo "Downloading artifacts from run $RUN_ID ..."
gh run download "$RUN_ID" --repo "$REPO" --name "$ARTIFACT_VST3" --dir "$TMP_DIR/vst3-raw"
gh run download "$RUN_ID" --repo "$REPO" --name "$ARTIFACT_AU"   --dir "$TMP_DIR/au-raw"

# upload-artifact@v4 strips the bundle directory name (Contents/ lands at the
# artifact root instead of RC-20 Clone.vst3/Contents/).  Reconstruct the wrapper.
mkdir -p "$TMP_DIR/vst3/$PLUGIN_VST3"
cp -r "$TMP_DIR/vst3-raw/." "$TMP_DIR/vst3/$PLUGIN_VST3/"
mkdir -p "$TMP_DIR/au/$PLUGIN_AU"
cp -r "$TMP_DIR/au-raw/." "$TMP_DIR/au/$PLUGIN_AU/"

# ── Install ────────────────────────────────────────────────────────────────────

install_plugin() {
    local src="$1"
    local dest_dir="$2"
    local plugin_name
    plugin_name=$(basename "$src")
    local dest="$dest_dir/$plugin_name"

    if [[ ! -e "$src" ]]; then
        echo "Error: expected '$plugin_name' in artifact but it was not found." >&2
        exit 1
    fi

    echo "Installing $plugin_name to $dest_dir ..."
    sudo mkdir -p "$dest_dir"
    sudo rm -rf "$dest"
    sudo cp -r "$src" "$dest"

    # Remove quarantine flag so Gatekeeper doesn't block unsigned binaries
    sudo xattr -rd com.apple.quarantine "$dest" 2>/dev/null || true
}

install_plugin "$TMP_DIR/vst3/$PLUGIN_VST3" "$VST3_DIR"
install_plugin "$TMP_DIR/au/$PLUGIN_AU"     "$AU_DIR"

# ── Done ───────────────────────────────────────────────────────────────────────

echo ""
echo "Installed:"
echo "  $VST3_DIR/$PLUGIN_VST3"
echo "  $AU_DIR/$PLUGIN_AU"
echo ""
echo "Rescan plugins in your DAW to pick up the new version."
