<div class="ascii-logo">
<pre> 
                
                !@@!@@@@@@@@!
              @@#############@@
            @@#################@@
           @#####################@     000
          @######     #####   ####@    0000
          @######     #####   ####@    000
          @######     #####   ####@   000
      ----@#######################@***00
     *****@#########       #######@ ***
   000    @#########       #######@
  0000    @#######################@
   000    @#######################@
           @#####################@
            @@#################@@
             @@###############@@
            !!!##!!!##!!!##!!!11
          !!!##!!!##!!!##!!!##!!!11

</pre>
</div>
# Ghost0Mod3
------------
Ghost0Mod3 is a modular ESP32 touchscreen application platform built on a 2.8-inch CYD-style board. The current prototype includes a BLE radar application, separate radar and system settings, touch navigation, Wi-Fi setup, and saved Wi-Fi credentials.

> **Project status:** Active working prototype

## Current Features

### Home Launcher
-----------------
- Green text and borders on a black background
- BLE Radar launcher
- System Settings launcher
- Placeholder tiles for future applications
- Touch-based navigation

### BLE Radar
----------------
- Continuous Bluetooth Low Energy scanning
- Animated radar sweep
- RSSI-based target positioning
- Stable visual target angle derived from the BLE address
- Up to 12 active targets
- Removal of stale targets
- Configurable minimum RSSI threshold
- Current threshold: `-75 dBm`

### BLE Classification
-----------------------
The radar can classify advertisements as:

- Possible Flock match
- Possible Axon match
- Heart Rate Service device
- Possible Apple device
- Possible Samsung device
- Possible Google or Pixel device
- Possible Motorola device
- Possible OnePlus device
- Possible Xiaomi or Redmi device
- Possible Huawei device
- Generic BLE device

Classification labels are hints only. A detected name, address prefix, or advertised service does not prove the exact identity of a device.

### Radar Settings
------------------
- ALPR matches
- Heart-rate devices
- Phone hints
- All BLE advertisements
- Clearing existing radar targets whenever a filter changes

### System Settings
--------------------
The System Settings menu is separate from Radar Settings and contains:

- Display Settings
- Wi-Fi Settings
- Sound Settings
- Device Information

### Wi-Fi Settings
------------------
- Scan nearby 2.4 GHz Wi-Fi networks
- Display SSID, RSSI, and security type
- Select an access point
- On-screen password keyboard
- Connect and disconnect controls
- Save credentials with ESP32 Preferences/NVS
- Automatic reconnection after restart
- Forget saved network

Supported network modes:

- Open
- WPA2-Personal
- WPA3-Personal
- WPA2/WPA3 transition mode

Not supported by the current personal-password interface:

- Legacy WPA
- WPA/WPA2 legacy mixed mode
- WEP
- Enterprise authentication
- Unknown security modes

### Navigation

- Reusable Back button component
- Stack-based navigation history
- Home is the root page
- Every secondary page returns to the previous page

## Important Limitations

### Radar Direction

The radar does not measure physical direction.

Each target angle is generated from its BLE address so that the dot remains visually stable. The distance from the center is based on RSSI strength.

### RSSI Distance

RSSI is not an exact distance measurement. Walls, antennas, device transmit power, reflections, and orientation can change the displayed signal strength.

### Phone Detection

A phone may not appear simply because Bluetooth is enabled. The phone must be actively transmitting a discoverable BLE advertisement.

### Wi-Fi Credentials

Wi-Fi credentials are stored in ESP32 Preferences/NVS. They survive restart and power loss, but they are not encrypted by the application.

## Hardware

Current test hardware:

- ESP32-WROOM-32E CYD-style board
- 2.8-inch ILI9341 TFT
- 240 x 320 native panel
- 320 x 240 landscape application layout
- XPT2046 resistive touchscreen
- 4 MB flash
- Wi-Fi and Bluetooth Low Energy

## Pin Configuration

### Display

| Function | GPIO |
|---|---:|
| TFT SCLK | 14 |
| TFT MOSI | 13 |
| TFT MISO | 12 |
| TFT DC | 2 |
| TFT CS | 15 |
| TFT Reset | Not connected |
| Backlight | 21 |

### Touchscreen

| Function | GPIO |
|---|---:|
| Touch SCLK | 25 |
| Touch MOSI | 32 |
| Touch MISO | 39 |
| Touch CS | 33 |
| Touch IRQ | Disabled, polling mode |

## Libraries

The project currently uses:

- Arduino framework for ESP32
- LovyanGFX
- NimBLE-Arduino
- ESP32 WiFi library
- ESP32 Preferences library

Example `platformio.ini` dependencies:

```ini
[env:cyd]
platform = espressif32
board = esp32dev
framework = arduino

monitor_speed = 115200
upload_speed = 460800

lib_deps =
    lovyan03/LovyanGFX
    h2zero/NimBLE-Arduino
```

The WiFi and Preferences libraries are supplied by the ESP32 Arduino framework.

## Project Structure

```text
Ghost0Mod3-Radar/
├── platformio.ini
├── README.md
├── include/
│   ├── app_ui.h
│   ├── back_button.h
│   ├── ble_scanner.h
│   ├── classifier_settings.h
│   ├── device_classifier.h
│   ├── device_info.h
│   ├── display.h
│   ├── display_settings.h
│   ├── home_screen.h
│   ├── radar.h
│   ├── radar_settings.h
│   ├── sound_settings.h
│   ├── system_settings.h
│   ├── touch.h
│   ├── wifi_credential_store.h
│   └── wifi_settings.h
└── src/
    ├── app_ui.cpp
    ├── back_button.cpp
    ├── ble_scanner.cpp
    ├── classifier_settings.cpp
    ├── device_classifier.cpp
    ├── device_info.cpp
    ├── display.cpp
    ├── display_settings.cpp
    ├── home_screen.cpp
    ├── main.cpp
    ├── radar.cpp
    ├── radar_settings.cpp
    ├── sound_settings.cpp
    ├── system_settings.cpp
    ├── touch.cpp
    ├── wifi_credential_store.cpp
    └── wifi_settings.cpp
```

## Build and Upload

### Visual Studio Code

1. Open the project folder in Visual Studio Code.
2. Save all source files.
3. Run **PlatformIO: Clean** after large structural changes.
4. Run **PlatformIO: Build**.
5. Run **PlatformIO: Upload**.
6. Open the PlatformIO Serial Monitor at `115200` baud.

### PowerShell

From the project folder:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run
```

Upload:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --target upload
```

Open the Serial Monitor:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" device monitor --baud 115200
```

Press `Ctrl+C` to close the Serial Monitor.

## Application Flow

```text
Boot
  ↓
Ghost0Mod3 splash
  ↓
Home launcher
  ├── BLE Radar
  │     └── Radar Settings
  └── System Settings
        ├── Display Settings
        ├── Wi-Fi Settings
        ├── Sound Settings
        └── Device Info
```

## Touchscreen Notes

The touchscreen reports a mirrored horizontal axis on the current board. `touch.cpp` corrects it with logic equivalent to:

```cpp
touchX = lcd.width() - 1 - touchX;
```

The first press is tracked with `justPressed` so a held finger does not repeatedly activate a control.

## Wi-Fi Test Procedure

1. Open **Home**.
2. Open **System Settings**.
3. Open **Wi-Fi**.
4. Tap **Scan**.
5. Confirm nearby networks appear with RSSI and security labels.
6. Choose an Open, WPA2, WPA3, or WPA2/WPA3 network.
7. Enter the password when required.
8. Tap **Connect**.
9. Confirm the connected page shows the SSID, IP address, RSSI, and saved status.
10. Restart the board and verify automatic reconnection.
11. Use **Forget Network** to disconnect and delete stored credentials.

## Recommended Test Checklist

- [ ] Project builds without compiler or linker errors
- [ ] Firmware uploads successfully
- [ ] Splash screen appears
- [ ] Home page appears
- [ ] BLE Radar opens
- [ ] Radar Back button returns Home
- [ ] Radar Settings opens and returns correctly
- [ ] Filter changes clear current BLE targets
- [ ] System Settings opens from Home
- [ ] Display Settings opens and returns correctly
- [ ] Wi-Fi Settings scans nearby networks
- [ ] WPA2 connection succeeds
- [ ] WPA3 connection succeeds when a compatible access point is available
- [ ] Saved Wi-Fi reconnects after restart
- [ ] Forget Network clears saved credentials
- [ ] Sound Settings opens and returns correctly
- [ ] Device Info displays current status

## Planned Work

- Real brightness control using PWM
- Configurable screen timeout
- Preferences-backed system settings
- Actual sound or buzzer output after the board audio pin is confirmed
- Improved Wi-Fi scan status and connection diagnostics
- Optional hidden-network entry
- Additional launcher applications
- Versioned settings migration
- Better handling when BLE and Wi-Fi run simultaneously

## Development Rules

- Keep Home settings separate from Radar settings.
- Use the reusable Back button on every secondary page.
- Add every new screen to the `AppScreen` enum.
- Route new screens through `appUiSetScreen()` and `appUiGoBack()`.
- Keep hardware responsibilities in dedicated modules.
- Save credentials only after a Wi-Fi connection succeeds.
- Build and test after each major module change.

## Version

```text
Ghost0Mod3 working prototype
Firmware milestone: v0.2
```

## License

No license has been selected yet. Add a `LICENSE` file before publicly distributing or accepting outside contributions.
