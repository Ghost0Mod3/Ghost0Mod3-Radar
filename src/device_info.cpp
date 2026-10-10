#include <Arduino.h>
#include <WiFi.h>

#include "app_ui.h"
#include "back_button.h"
#include "ble_scanner.h"
#include "device_info.h"
#include "display.h"
#include "touch.h"

namespace
{
    constexpr uint16_t COLOR_BACKGROUND = 0x0000;
    constexpr uint16_t COLOR_GREEN = 0x07E0;
    constexpr uint16_t COLOR_DARK_GREEN = 0x0320;

    void drawInfoLine(
        int y,
        const char* label,
        const String& value
    )
    {
        lcd.setFont(&fonts::Font0);

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_left
        );

        lcd.setTextColor(
            COLOR_DARK_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            label,
            18,
            y
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_right
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            value,
            lcd.width() - 18,
            y
        );
    }
}

void deviceInfoDraw()
{
    lcd.fillScreen(COLOR_BACKGROUND);

    lcd.setFont(&fonts::Font2);
    lcd.setTextDatum(
        lgfx::textdatum_t::top_center
    );

    lcd.setTextColor(
        COLOR_GREEN,
        COLOR_BACKGROUND
    );

    lcd.drawString(
        "DEVICE INFO",
        lcd.width() / 2,
        7
    );

    lcd.drawLine(
        50,
        31,
        lcd.width() - 50,
        31,
        COLOR_DARK_GREEN
    );

    drawInfoLine(
        50,
        "DEVICE",
        "GHOST0MOD3"
    );

    drawInfoLine(
        70,
        "FIRMWARE",
        "v0.2"
    );

    drawInfoLine(
        90,
        "DISPLAY",
        String(lcd.width()) +
        "x" +
        String(lcd.height())
    );

    drawInfoLine(
        110,
        "BLE",
        bleScannerIsRunning()
            ? "SCANNING"
            : "STOPPED"
    );

    drawInfoLine(
        130,
        "WI-FI",
        WiFi.status() == WL_CONNECTED
            ? "CONNECTED"
            : "DISCONNECTED"
    );

    drawInfoLine(
        150,
        "FREE HEAP",
        String(ESP.getFreeHeap()) +
        " BYTES"
    );

    drawInfoLine(
        170,
        "FLASH",
        String(ESP.getFlashChipSize()) +
        " BYTES"
    );

    backButtonDraw();

    lcd.setTextDatum(
        lgfx::textdatum_t::top_left
    );
}

void deviceInfoUpdate(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
)
{
    if(
        backButtonPressed(
            justPressed,
            touchX,
            touchY
        )
    )
    {
        touchReset();
        appUiGoBack();
    }
}