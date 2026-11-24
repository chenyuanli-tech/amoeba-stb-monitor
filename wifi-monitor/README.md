# Amoeba STB Mini Monitor — Wi-Fi OLED

Wi-Fi monitor for ZTE B860H V1 running Armbian. The ESP8266 drives the SSD1306 OLED locally; the STB exposes metrics over the LAN. No STB GPIO/I2C pins or DTB changes are required.

## Hardware

| OLED SSD1306 | NodeMCU V3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SCL | D1 / GPIO5 |
| SDA | D2 / GPIO4 |

Default I2C address: 0x3C (some modules use 0x3D).

## Install API on Armbian

Copy this wifi-monitor directory to the STB, then run:

    cd wifi-monitor
    sudo bash install-server.sh

Service listens on TCP port 8765. Endpoint:

    http://192.168.1.14:8765/api/v1/status

Python 3 standard library only. Metrics: CPU, RAM, thermal-zone temperature, storage at /mnt/amoeba, eth0/eth1 throughput and link state, IP, uptime, SSH and NetworkManager service state.

If UFW is enabled, permit only the trusted LAN, for example:

    sudo ufw allow from 192.168.1.0/24 to any port 8765 proto tcp

Do not expose port 8765 to the public internet. Optional bearer token: set AMOEBA_API_TOKEN in a systemd override and enter the same token in the firmware.

## Flash NodeMCU

Install Arduino IDE, ESP8266 board package, and libraries Adafruit GFX, Adafruit SSD1306, and ArduinoJson (6.x or 7.x). Open firmware/amoeba_stb_oled.ino, set WIFI_SSID, WIFI_PASSWORD, and API_URL, select NodeMCU 1.0 (ESP-12E Module), then flash over USB.

Default API URL uses 192.168.1.14. Change if the STB IP differs. Polling interval is 2 seconds; display pages rotate every 5 seconds.

## Test

    curl -s http://127.0.0.1:8765/api/v1/status

First network-rate sample is zero because it establishes the counter baseline. eth1 is shown as the primary WAN rate.
