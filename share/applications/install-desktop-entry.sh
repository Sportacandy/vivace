#!/usr/bin/env bash
# Installs a desktop menu entry + file associations for this extracted Vivace
# tarball. Unlike the NSIS/IFW installers, a plain .tar.gz has no install
# step to template the real install path into vivace.desktop ahead of time,
# so this resolves its own current location and substitutes it in instead.
# Safe to re-run any time, including after moving the extracted directory.
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$script_dir/../.." && pwd)"

target_dir="$HOME/.local/share/applications"
mkdir -p "$target_dir"
sed "s|__INSTALL_DIR__|$root|g" "$script_dir/vivace.desktop" > "$target_dir/vivace.desktop"

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$target_dir"
fi

echo "Installed $target_dir/vivace.desktop (pointing at $root)."
echo "Vivace should now appear in your application menu and be pinnable to your taskbar/dock."
