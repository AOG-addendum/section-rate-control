# Project Overview: esp32-section-rate

This document explains the project based on the code itself.

## What it is

`esp32-section-rate` is ESP32 (Arduino/PlatformIO) firmware for a peripheral control module used with **AgOpenGPS**, an open-source precision-agriculture guidance system. The module combines two functions in one board:

1. **Section control** — turning sprayer/planter boom sections on and off.
2. **Rate control** — closed-loop control of a valve or motor to maintain a target application (flow) rate, using a flow meter for feedback.

It communicates with AgOpenGPS's AgIO app and with a separate "RateController" companion app over UDP using AgOpenGPS's PGN (Parameter Group Number) binary protocol.

## Directory structure

```
esp32-section-rate/
├── platformio.ini              # PlatformIO config: esp32dev board, Arduino framework, deps
├── firmware_file_name.py       # build hook naming output binary "section_rate_71_firmware_<ver>"
├── no_ota_eeprom.csv / ota_small_spiffs.csv   # ESP32 partition table variants
├── .gitmodules                  # submodules: tools/EspArduinoExceptionDecoder, lib/DOGL
├── lib/
│   ├── json/json.hpp             # vendored nlohmann::json
│   └── DOGL/                     # graphic LCD driver (submodule)
└── src/                          # flat source layout, no include/ folder
    ├── main.cpp / main.hpp        # entry point, global config/state, pin setup
    ├── network.cpp                 # WiFi STA/AP connection management
    ├── sections.cpp                 # section relay control via MCP23017 + UDP
    ├── Rate.cpp                     # flow-meter pulse counting / UPM calculation
    ├── ManualRate.cpp                # manual rate/valve control task
    ├── RCteensy.cpp                  # PGN parsing helpers, CRC, 20Hz rate task
    ├── Comm.cpp                      # UDP send/receive of AgOpenGPS/RateController PGNs
    ├── PID.cpp                       # PID / motor control algorithms
    ├── Motor.cpp                     # drives valve/motor PWM + direction pins
    ├── Analog.cpp                    # ADS1115 analog read (pressure sensor)
    ├── jsonFunctions.cpp/.hpp         # SPIFFS-backed JSON config load/save
    ├── ESP32_UI.cpp                   # ESPUI web UI (config, diagnostics, OTA)
    ├── diagnostics.cpp                 # 1Hz task updating ESPUI diagnostic labels
    ├── idleStats.cpp                   # FreeRTOS idle/heap stats
    ├── display.cpp/.hpp                 # DOGL 128x64 LCD driver / readouts
    └── AOG_bmp.cpp/.hpp                  # AgOpenGPS splash bitmap for LCD
```

## Entry point (`src/main.cpp`)

`setup()`:
- Starts LCD splash task, Serial (115200), mounts SPIFFS, loads persisted config (`sectionRate.json`, `pidConfig.json`).
- Selects GPIO pin mapping based on `pcbVersion` (Rev C/D/E boards).
- Reads a physical Auto/Manual selector GPIO to decide operating mode.
- `Wire.begin()` (I2C), sets up flow-sensor interrupt pins, configures a PWM channel (`ledcSetup`) for the valve/motor.
- Brings up WiFi (`initWiFi()`) and the ESPUI web server (`initESPUI()`), optionally OTA.
- Opens UDP socket(s) for RateController/AgIO communication.
- Configures an **MCP23017** I2C GPIO expander (address `0x20`) — Port A as output driving section relays, Port B (in manual mode) read for physical section switches.
- Branches: automatic mode wires up AgOpenGPS section UDP listener + rate control; manual mode turns all sections off and reads physical switches/buttons instead.
- Starts LCD display, idle-stats, and diagnostics FreeRTOS tasks.

`loop()` is minimal — it just polls whether the physical Auto/Manual switch changed since boot; if so, it forces all sections off and calls `ESP.restart()` to reboot into the new mode. Essentially all real work happens in separate FreeRTOS tasks, not in `loop()`.

## Hardware / peripherals

- **MCU**: ESP32 dev board (PlatformIO `esp32dev`, Arduino framework).
- **Section relays**: MCP23017 I2C GPIO expander, Port A drives up to 8 relays from a bitmask; Port B senses physical switches in manual mode.
- **Rate/flow sensing**: interrupt-driven flow-meter pulse counting (`Rate.cpp`), forward/reverse direction pins, PWM output (via `ledcWrite`) to a valve or motor.
- **Analog/pressure**: ADS1115 I2C ADC (`Analog.cpp`).
- **Display**: DOGL 128x64 graphic LCD showing rate/section status, plus an AgOpenGPS splash bitmap.
- **Physical controls**: Auto/Manual selector, rate up/down buttons, LCD menu buttons.
- **Networking**: ESP32 WiFi — station mode to join an existing network, falling back to a SoftAP (`"Section Rate <MAC>"`) if it can't connect; heavy use of `AsyncUDP`.
- **Web UI**: ESPUI-based configuration/diagnostics page, plus AsyncElegantOTA for browser-based firmware updates.
- **Storage**: SPIFFS holding `sectionRate.json` (network/settings) and `pidConfig.json` (PID tuning), serialized via vendored `nlohmann::json`.

## What "section" and "rate" mean here

- **Section control**: In automatic mode, AgOpenGPS's AgIO sends PGN `32766` over UDP with an 8-bit section on/off mask, written directly to the MCP23017 relay expander (`sections.cpp`). In manual mode, physical switches on the expander's Port B are polled instead.
- **Rate control**: Closed-loop control of a flow-control valve/motor to hit a target application rate, expressed in "UPM" (units per minute) derived from flow-meter pulse timing (`Rate.cpp`) and driven through PID (`PID.cpp`) / motor control (`Motor.cpp`, PWM + direction pins). `Sensor.ControlType` selects strategy: 0 = standard valve, 1 = fast-close ("combo") valve, 2 = motor, 3 = motor + weight/scale. Rate/PID settings arrive over UDP from a separate "RateController" companion app.

## Networking / protocol (`network.cpp`, `Comm.cpp`, `sections.cpp`)

- `initWiFi()`: tries station mode against a saved SSID/password (5s timeout), else falls back to AP mode at `192.168.1.1`. Once a station IP is obtained, the last octet is forced to `.71` and a static IP is set relative to the DHCP gateway; broadcast destination for outgoing PGNs is derived as `.255`.
- UDP ports (configurable via ESPUI): `rcPortSendFrom` (6100), `rcPortListenTo` (28888), `rcPortSendTo` (29999), `aogPortListenTo` (8888), `aogPortSendTo` (9999).
- **Incoming PGNs**: `32766` section on/off bitmask, `32712` AgIO "Hello" (captures AgIO's IP), `32500` rate settings (target rate, meter cal, control flags), `32502` PID tuning, RC-app "Hello" for failover if AgIO Hello silence exceeds 4s.
- **Outgoing PGNs**: `32400` rate/quantity/PWM status (sent ~5Hz from a 20Hz task), `32401` pressure/RSSI/module version, periodic Hello heartbeat to AgOpenGPS (~2.5s), switch-state PGN to the RateController.
- All PGNs use a simple additive checksum (`CRC()` in `RCteensy.cpp`).

## Recent development focus (as of this writing)

The most recent commits concentrate on **networking robustness**: AgIO Hello heartbeat timing, UDP send-timer tuning, WiFi STA/AP fallback timing, removing the DNS server, and adding an ESPUI widget showing AgOpenGPS's live IP and time-since-last-Hello. Just before that: updating the Rate/PID protocol to "RCApp 4.0.0-beta" and hardening the physical Auto/Manual switch (forcing all sections off before rebooting on a switch change). This suggests the project is stabilizing ESP32 ↔ AgOpenGPS ↔ RateController communication ahead of a release build.

For up-to-date details, check `git log` and the source files referenced above directly, since this snapshot will drift as the code evolves.
