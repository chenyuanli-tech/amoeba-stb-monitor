#!/usr/bin/env bash
set -Eeuo pipefail
if [[ "${EUID}" -ne 0 ]]; then echo "Jalankan sebagai root."; exit 1; fi
DTB_NAME="meson-gxl-s905x-b860h.dtb"
BACKUP_DIR="/boot/amoeba-dtb-backup"
TARGET=""
for candidate in "/boot/dtb/amlogic/$DTB_NAME" "/boot/dtb/$DTB_NAME" "/boot/$DTB_NAME"; do
  [[ -f "$candidate" ]] && { TARGET="$candidate"; break; }
done
[[ -n "$TARGET" ]] || { echo "DTB target tidak ditemukan."; exit 1; }
BACKUP="$(find "$BACKUP_DIR" -maxdepth 1 -type f -name "${DTB_NAME}.before-amoeba.*" -printf '%T@ %p\n' 2>/dev/null | sort -nr | head -n1 | cut -d' ' -f2-)"
[[ -n "$BACKUP" && -f "$BACKUP" ]] || { echo "Backup tidak ditemukan."; exit 1; }
cp -a "$BACKUP" "$TARGET"
sync
echo "DTB asli dipulihkan ke $TARGET"
echo "Reboot jika diperlukan: reboot"
