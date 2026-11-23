#!/usr/bin/env bash
set -Eeuo pipefail

REPO="chenyuanli-tech/amoeba-stb-monitor"
ASSET="amoeba-b860h-i2c-installer.zip"
DTB_NAME="meson-gxl-s905x-b860h.dtb"
WORK="$(mktemp -d)"
BACKUP_DIR="/boot/amoeba-dtb-backup"
TARGET=""

cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT

if [[ "${EUID}" -ne 0 ]]; then
  echo "Jalankan sebagai root: sudo bash $0"
  exit 1
fi
for cmd in curl unzip find install cmp; do
  command -v "$cmd" >/dev/null || { echo "Perintah tidak tersedia: $cmd"; exit 1; }
done

echo "[1/6] Mengunduh rilis Amoeba DTB terbaru..."
curl -fL --retry 3 "https://github.com/${REPO}/releases/latest/download/${ASSET}" -o "$WORK/package.zip"
unzip -q "$WORK/package.zip" -d "$WORK/package"

echo "[2/6] Memeriksa DTB target..."
NEW_DTB="$(find "$WORK/package" -type f -name "$DTB_NAME" -print -quit)"
[[ -n "$NEW_DTB" ]] || { echo "File $DTB_NAME tidak ditemukan. Instalasi dibatalkan."; exit 1; }

echo "[3/6] Menentukan lokasi DTB aktif..."
for candidate in "/boot/dtb/amlogic/$DTB_NAME" "/boot/dtb/$DTB_NAME" "/boot/$DTB_NAME"; do
  if [[ -f "$candidate" ]]; then TARGET="$candidate"; break; fi
done
if [[ -z "$TARGET" ]]; then
  if [[ -d /boot/dtb/amlogic ]]; then TARGET="/boot/dtb/amlogic/$DTB_NAME"; else
    echo "Direktori DTB Armbian tidak ditemukan. Tidak ada perubahan."; exit 1
  fi
fi

echo "[4/6] Membuat backup..."
mkdir -p "$BACKUP_DIR"
STAMP="$(date +%Y%m%d-%H%M%S)"
if [[ -f "$TARGET" ]]; then cp -a "$TARGET" "$BACKUP_DIR/${DTB_NAME}.before-amoeba.${STAMP}"; fi

echo "[5/6] Memasang DTB Amoeba..."
install -D -m 0644 "$NEW_DTB" "$TARGET"
sync

echo "[6/6] Memverifikasi hasil..."
cmp -s "$NEW_DTB" "$TARGET" || { echo "Verifikasi gagal. Periksa backup di $BACKUP_DIR"; exit 1; }
echo "DTB terpasang: $TARGET"
echo "Backup tersimpan di: $BACKUP_DIR"
echo "Tidak reboot otomatis. Reboot hanya setelah verifikasi jalur pin fisik dan kesiapan rollback."
