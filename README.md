# esp32-section-rate
ESP32 (Arduino/PlatformIO) firmware for a combined section-control and rate-control module used with **AgOpenGPS**. It talks to AgOpenGPS's AgIO app and to a separate "RateController" companion app over UDP using AgOpenGPS's PGN binary protocol.

# Features
* Section control: drives up to 8 boom sections through an MCP23017 I2C relay expander, controlled directly from AgOpenGPS's section on/off PGN
* Rate control: closed-loop control of a valve or motor to hit a target application rate, using flow-meter pulse feedback and a tunable PID
* Supports standard valves, fast-close ("combo") valves, and motor (with or without scale/weight feedback) control types
* Manual mode: a physical Auto/Manual selector switches to reading physical section switches and rate up/down buttons instead of AgOpenGPS control
* WiFi station mode with automatic fallback to a SoftAP (`Section Rate <MAC>`) if it can't join the configured network
* ESPUI-based web configuration/diagnostics page, including live status of AgOpenGPS's IP and time since last heartbeat
* Browser-based OTA firmware updates via AsyncElegantOTA
* DOGL 128x64 graphic LCD showing rate/section status and an AgOpenGPS splash screen
* Settings and PID tuning persisted to SPIFFS (`sectionRate.json`, `pidConfig.json`)

# Caveats
* This firmware uses a custom flash-partition layout (`min_spiffs.csv` / the partition CSVs in this repo), so it must be flashed at least once with a full flasher as described below. A generic OTA example/tool will not work for the first flash.
* GPIO pin mapping depends on the `pcbVersion` selected in `main.cpp`/config (Rev C/D/E boards) — make sure this matches your hardware before flashing.
* Changing the physical Auto/Manual switch forces all sections off and reboots the board into the new mode.

# Hardware
* **MCU**: ESP32 dev board (PlatformIO `esp32dev`, Arduino framework)
* **Section relays**: MCP23017 I2C GPIO expander — Port A drives relays, Port B senses physical switches in manual mode
* **Rate/flow sensing**: interrupt-driven flow-meter pulse counting, forward/reverse direction pins, PWM output to a valve or motor
* **Analog/pressure**: ADS1115 I2C ADC
* **Display**: DOGL 128x64 graphic LCD (submodule)

# Installation
TL;DR: install [PlatformIO](https://platformio.org/), clone the repository with all submodules, build and upload it.

## Windows
### Install Prerequisites
1. Install [Visual Studio Code](https://visualstudio.microsoft.com/)
2. Inside VS Code:
   1. Click on "Extensions" on the left hand side
   2. Search for and install PlatformIO
   3. Open the PlatformIO home tab (if not opened automatically, use the menu)
      1. Go to "Platforms" (left side tile)
      2. Choose the "Embedded" tab
      3. Install the "Espressif 32" platform
3. Install the drivers needed for your ESP32 board's USB-to-serial chip (commonly CP210x); consult your board manufacturer's documentation.

### Downloading the repository
1. Open a folder in the explorer, preferably not too deep inside the drive. `C:\` or a folder under it should work.
2. Right click on it and choose "Git Bash Here".
3. Enter `git clone --recursive https://github.com/AOG-silrep/section-rate-control.git`

### Compiling
1. Open the created folder above by right clicking the folder and "Open with Visual Studio Code".
2. Click build (the tile with the tick); missing dependencies should be installed automatically.

### Upload to the ESP32
1. Connect the ESP32 over USB.
2. Click on upload (the icon with the arrow in the bottom blue bar).

Alternatively, once a first flash has been done, you can use the OTA update page in the WebUI to upload a new `firmware.bin`.

## Linux
Install `platformio` with the package management system of your distribution and find a guide to help you set it up for your particular editor/IDE.

To do it on the console, clone the repository with all the submodules (`git clone --recurse-submodules`), then change into the cloned folder and enter:
```
platformio run -t upload
```

This takes care of all the required libraries and uploads it to a connected ESP32.

## Configuration

All configuration is done in the WebUI. To connect to the created hotspot of the ESP32, using a mobile device is normally the simplest solution.

On first boot (or if it can't join a saved WiFi network), the module starts a hotspot named `Section Rate <MAC>` at `192.168.1.1`. Once connected to your own WiFi, the module's IP is set with a `.71` last octet on your network. Alternatively, connect the ESP32 over USB and open a serial monitor at 115200 baud — it prints connection info on boot.

After logging in to the WebUI, you can configure WiFi credentials, UDP ports for AgIO/RateController, rate/PID settings, and section behavior. Settings are saved to SPIFFS and persist across firmware uploads; only a full flash erase resets them to defaults.

## Updating the repository
As there are sometimes new submodules added, enter the following to update:
1. Go to the repository folder and open a terminal.
2. `git pull`
3. `git submodule sync`
4. `git submodule update --init --recursive`

Repeat as needed.
