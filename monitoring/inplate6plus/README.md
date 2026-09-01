# Inkplate 6Plus — Solar / Battery / Grid Monitor

E-paper monitoring display for a home solar power system, built for the
**e-radionica.com Inkplate 6Plus** (ESP32, 1024×758 px, 3-bit e-paper,
frontlight, RTC, touchscreen, wake-up button).

The board boots, connects to WiFi, synchronizes the RTC via NTP, pulls the
current state from [ioBroker](https://www.iobroker.net/) over HTTP, draws one
full-page update, and goes into deep sleep for 60 minutes. Pressing the
wake-up button (GPIO36) starts a refresh immediately.

Displayed: solar panel (MPPT) power / yield, battery (BMV) status, grid
consumption (Tasmota / Shelly) with power and yield graphs, inverter state,
WiFi RSSI and battery level.

## Required software (exact versions)

| Component | Version | Notes |
|---|---|---|
| Board package: Inkplate Boards (e-radionica) | **8.1.0** | Arduino IDE: Boards Manager URL below. arduino-cli: `board install Inkplate_Boards:esp32` |
| ESP32 Arduino core | **3.3.5** | Bundled with board package 8.1.0 |
| Library: InkplateLibrary (Soldered/e-radionica) | **11.1.4** | https://github.com/SolderedElectronics/Inkplate-Arduino-library |
| Library: RTClib | **2.1.4** | DateTime / TimeSpan handling |
| Library: Time | **1.6.1** | `time.h` / `makeTime()` / NTP helpers |
| arduino-cli (optional) | 1.5.1 | Any recent version works; Arduino IDE is fine too |

`WiFi`, `HTTPClient`, `driver/rtc_io.h` and `nvs.h` are part of the ESP32
core — no extra installation needed.

Boards Manager URL (Arduino IDE):

```
https://github.com/SolderedElectronics/Dasduino-Board-Definitions-for-Arduino-IDE/raw/master/package_Dasduino_Boards_index.json
```

Board selection: **e-radionica.com Inkplate 6Plus**
FQBN (arduino-cli): `Inkplate_Boards:esp32:Inkplate6plus`

> **Note:** There are two variants of the Inkplate 6 Plus, listed as separate
> board entries in the Arduino board menu:
> **e-radionica.com Inkplate 6Plus** (`Inkplate6plus`, used by this project)
> and **Soldered Inkplate 6Plus** (`Inkplate6plusV2`). Select the FQBN that
> matches the physical board you have.

## Configuration

| File | Purpose |
|---|---|
| `Config.h` | Defaults: ioBroker host/port, update interval, version string |
| `ConfigOverride.h` | **Your** WiFi credentials and ioBroker settings. Not tracked by git — create it locally (start from `Config.h` and adjust). Remove the `#include "ConfigOverride.h"` line in `inplate6plus.ino` if you keep everything in `Config.h`. |

## Build & flash (arduino-cli)

```bash
arduino-cli compile --fqbn "Inkplate_Boards:esp32:Inkplate6plus" .
arduino-cli upload  -p COM4 --fqbn "Inkplate_Boards:esp32:Inkplate6plus" .
```

(Adjust the serial port; it is COM4 on this machine.)

## Project layout

| File | Contents |
|---|---|
| `inplate6plus.ino` | Main: boot → WiFi → NTP → battery/SHT30/ioBroker → draw → deep sleep |
| `Display.h` | All drawing: layout, graphs, symbols |
| `Data.h` | Data structures, ioBroker entity definitions |
| `IoBroker.h` | HTTP queries to the ioBroker REST API (port 8087) |
| `EPD.h` | Display init / shutdown (frontlight, touchscreen, deep sleep) |
| `EPDWifi.h` | WiFi connect / disconnect |
| `RTCTime.h` | NTP sync → internal RTC |
| `Utils.h` | Date/time and WiFi signal helpers |
| `Battery.h`, `SHT30.h` | Board battery voltage / temperature, SHT30 sensor |
| `Config.h`, `ConfigOverride.h` | Configuration (see above) |
| `Icons.h` | Embedded icon bitmaps |

## Deep sleep

Wakeup is timer-based (`esp_sleep_enable_timer_wakeup`, currently 60 min) plus
the wake-up button on GPIO36. The ESP32 timer is 32-bit µs: the single-shot
maximum is ~4294 s (~71.6 min). Longer intervals require re-entering deep
sleep after each wake-up.

## License

GPLv3 — see the license headers in the source files. Based on the original
"inkplate solar monitor" by SFini (2022).
