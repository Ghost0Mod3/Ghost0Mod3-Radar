#include <Arduino.h>

#include "app_ui.h"
#include "back_button.h"
#include "ble_scanner.h"
#include "classifier_settings.h"
#include "display.h"
#include "radar_settings.h"
#include "touch.h"

namespace
{
    constexpr uint16_t COLOR_BACKGROUND = 0x0000;
    constexpr uint16_t COLOR_GREEN = 0x07E0;
    constexpr uint16_t COLOR_DARK_GREEN = 0x0320;
    constexpr uint16_t COLOR_DIM_GREEN = 0x4208;

    constexpr int ROW_X = 8;
    constexpr int ROW_WIDTH = 304;
    constexpr int ROW_HEIGHT = 34;

    constexpr int ROW_ALPR_Y = 42;
    constexpr int ROW_HEART_Y = 82;
    constexpr int ROW_PHONE_Y = 122;
    constexpr int ROW_ALL_BLE_Y = 162;

    constexpr int TOGGLE_X = 246;
    constexpr int TOGGLE_WIDTH = 56;
    constexpr int TOGGLE_HEIGHT = 24;

    bool pointInside(
        uint16_t touchX,
        uint16_t touchY,
        int x,
        int y,
        int width,
        int height
    )
    {
        return
            touchX >= x &&
            touchX < x + width &&
            touchY >= y &&
            touchY < y + height;
    }

    bool rowContains(
        uint16_t touchX,
        uint16_t touchY,
        int rowY
    )
    {
        return pointInside(
            touchX,
            touchY,
            ROW_X,
            rowY,
            ROW_WIDTH,
            ROW_HEIGHT
        );
    }

    void drawToggle(
        int rowY,
        const char* label,
        bool enabled
    )
    {
        lcd.fillRoundRect(
            ROW_X,
            rowY,
            ROW_WIDTH,
            ROW_HEIGHT,
            5,
            COLOR_BACKGROUND
        );

        lcd.drawRoundRect(
            ROW_X,
            rowY,
            ROW_WIDTH,
            ROW_HEIGHT,
            5,
            COLOR_DARK_GREEN
        );

        lcd.setFont(&fonts::Font2);

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_left
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            label,
            ROW_X + 10,
            rowY + ROW_HEIGHT / 2
        );

        const int toggleY =
            rowY +
            (ROW_HEIGHT - TOGGLE_HEIGHT) / 2;

        const uint16_t toggleColor =
            enabled
                ? COLOR_GREEN
                : COLOR_DIM_GREEN;

        lcd.fillRoundRect(
            TOGGLE_X,
            toggleY,
            TOGGLE_WIDTH,
            TOGGLE_HEIGHT,
            TOGGLE_HEIGHT / 2,
            toggleColor
        );

        const int knobX =
            enabled
                ? TOGGLE_X + TOGGLE_WIDTH - 12
                : TOGGLE_X + 12;

        const int knobY =
            toggleY + TOGGLE_HEIGHT / 2;

        lcd.fillCircle(
            knobX,
            knobY,
            9,
            COLOR_BACKGROUND
        );

        lcd.drawCircle(
            knobX,
            knobY,
            9,
            COLOR_GREEN
        );

        lcd.setFont(&fonts::Font0);

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_center
        );

        lcd.setTextColor(
            enabled
                ? COLOR_BACKGROUND
                : COLOR_GREEN,
            toggleColor
        );

        lcd.drawString(
            enabled ? "ON" : "OFF",
            enabled
                ? TOGGLE_X + 16
                : TOGGLE_X + TOGGLE_WIDTH - 17,
            knobY
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::top_left
        );
    }

    void applyFilterChange()
    {
        /*
         * Delete targets classified under the old settings.
         * New advertisements will use the current settings.
         */
        bleScannerClearTargets();

        radarSettingsDraw();
        touchReset();
    }
}

void radarSettingsDraw()
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
        "RADAR SETTINGS",
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

    drawToggle(
        ROW_ALPR_Y,
        "ALPR MATCHES",
        classifierSettings.showAlpr
    );

    drawToggle(
        ROW_HEART_Y,
        "HEART RATE",
        classifierSettings.showHeartRate
    );

    drawToggle(
        ROW_PHONE_Y,
        "PHONE HINTS",
        classifierSettings.showPhones
    );

    drawToggle(
        ROW_ALL_BLE_Y,
        "ALL BLE",
        classifierSettings.showCustom
    );

    backButtonDraw();

    lcd.setTextDatum(
        lgfx::textdatum_t::top_left
    );
}

void radarSettingsUpdate(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
)
{
    if(!justPressed)
    {
        return;
    }

    Serial.print("Radar Settings touch: X=");
    Serial.print(touchX);
    Serial.print(" Y=");
    Serial.println(touchY);

    if(
        backButtonPressed(
            justPressed,
            touchX,
            touchY
        )
    )
    {
        Serial.println("Radar Settings: BACK");

        touchReset();
        appUiGoBack();
        return;
    }

    if(
        rowContains(
            touchX,
            touchY,
            ROW_ALPR_Y
        )
    )
    {
        classifierSettings.showAlpr =
            !classifierSettings.showAlpr;

        Serial.print("ALPR matches: ");
        Serial.println(
            classifierSettings.showAlpr
                ? "ON"
                : "OFF"
        );

        applyFilterChange();
        return;
    }

    if(
        rowContains(
            touchX,
            touchY,
            ROW_HEART_Y
        )
    )
    {
        classifierSettings.showHeartRate =
            !classifierSettings.showHeartRate;

        Serial.print("Heart rate: ");
        Serial.println(
            classifierSettings.showHeartRate
                ? "ON"
                : "OFF"
        );

        applyFilterChange();
        return;
    }

    if(
        rowContains(
            touchX,
            touchY,
            ROW_PHONE_Y
        )
    )
    {
        classifierSettings.showPhones =
            !classifierSettings.showPhones;

        Serial.print("Phone hints: ");
        Serial.println(
            classifierSettings.showPhones
                ? "ON"
                : "OFF"
        );

        applyFilterChange();
        return;
    }

    if(
        rowContains(
            touchX,
            touchY,
            ROW_ALL_BLE_Y
        )
    )
    {
        classifierSettings.showCustom =
            !classifierSettings.showCustom;

        Serial.print("All BLE: ");
        Serial.println(
            classifierSettings.showCustom
                ? "ON"
                : "OFF"
        );

        applyFilterChange();
        return;
    }
}