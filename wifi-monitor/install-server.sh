#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEST="/opt/amoeba-stb-monitor"
SERVICE="amoeba-monitor-api"
if [[ $EUID -ne 0 ]]; then echo "Run with sudo: sudo bash install-server.sh"; exit 1; fi
install -d -m 0755 "$DEST"
install -m 0644 "$ROOT/server/monitor_api.py" "$DEST/monitor_api.py"
getent group amoeba-monitor >/dev/null || groupadd --system amoeba-monitor
id amoeba-monitor >/dev/null 2>&1 || useradd --system --gid amoeba-monitor --home-dir "$DEST" --no-create-home --shell /usr/sbin/nologin amoeba-monitor
install -m 0644 "$ROOT/systemd/amoeba-monitor-api.service" "/etc/systemd/system/${SERVICE}.service"
systemctl daemon-reload
systemctl enable --now "$SERVICE"
echo "API installed. Test: curl http://127.0.0.1:8765/api/v1/status"
echo "If firewall is enabled, allow TCP/8765 from your LAN only."
