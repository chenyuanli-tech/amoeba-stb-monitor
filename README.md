# Amoeba STB Monitor — ZTE B860H V1

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
