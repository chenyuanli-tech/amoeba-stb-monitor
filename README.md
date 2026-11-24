# Amoeba STB Monitor — ZTE B860H V1

## Recommended: Wi-Fi OLED monitor (NodeMCU V3 + SSD1306)

For the NodeMCU V3 / 0.96-inch SSD1306 build, use the Wi-Fi implementation in [wifi-monitor](wifi-monitor/README.md). The ESP8266 drives the OLED over local I2C and reads metrics from the Armbian HTTP API on the B860H V1. It refreshes every 2 seconds and does not require STB GPIO wiring or DTB changes.

Quick start:
1. Copy the wifi-monitor directory to the STB.
2. Run sudo bash install-server.sh from that directory.
3. Set Wi-Fi credentials and the STB API URL in firmware/amoeba_stb_oled.ino, then flash the NodeMCU.

## Legacy experimental DTB path

The DTB patch below is an older experimental direct-I2C approach for an SH1106 OLED. It is separate from, and not required by, the recommended Wi-Fi SSD1306 implementation.

Experimental Armbian DTB patch to enable Meson GXL I2C_A for an SH1106 OLED.

## Build target
- Board: ZTE B860H V1 / Meson GXL S905X
- Kernel target: Ophub 6.12.114
- DTB: meson-gxl-s905x-b860h.dtb
- I2C_A SDA: GPIODV_24 (also UART_B TX)
- I2C_A SCL: GPIODV_25 (also UART_B RX)
- I2C clock: 100 kHz

## GitHub Actions
Run **Actions → Build Amoeba B860H I2C DTB → Run workflow**. On success, a prerelease is published with `amoeba-b860h-i2c-installer.zip`.

## Install from the STB
Only after a successful build and after verifying the physical board routing:

```bash
sudo apt-get update
sudo apt-get install -y curl unzip
curl -fL https://github.com/chenyuanli-tech/amoeba-stb-monitor/releases/latest/download/amoeba-b860h-i2c-installer.zip -o /tmp/amoeba-dtb.zip
mkdir -p /tmp/amoeba-dtb && unzip -o /tmp/amoeba-dtb.zip -d /tmp/amoeba-dtb
sudo bash /tmp/amoeba-dtb/install-amoeba-dtb.sh
```

Rollback:

```bash
sudo bash /tmp/amoeba-dtb/rollback-amoeba-dtb.sh
```

The installer backs up the existing DTB and does not reboot automatically.

## Safety
The SoC pinmux mapping is verified in the Ophub Linux source. It does **not** prove that the B860H board's X2 TTL connector is routed to these SoC pads. Do not wire OLED or reboot into the modified DTB until PCB routing, 3.3 V logic compatibility, and recovery method are verified.

## Kernel version caveat
The workflow requests exactly `6.12.114` and does not auto-select a newer kernel. If Ophub no longer exposes that source revision, the build must fail rather than silently building against a different kernel. Inspect the generated package before installation.
