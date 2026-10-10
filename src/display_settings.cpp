#include <Arduino.h>

#include "app_ui.h"
#include "back_button.h"
#include "display.h"
#include "display_settings.h"
#include "touch.h"

namespace
{
    constexpr uint16_t COLOR_BACKGROUND = 0x0000;
    constexpr uint16_t COLOR_GREEN = 0x07E0;
    constexpr uint16_t COLOR_DARK_GREEN = 0x0320;

    constexpr int ROW_X = 12;
    constexpr int ROW_WIDTH = 296;
    constexpr int ROW_HEIGHT = 52;

    constexpr int BRIGHTNESS_Y = 55;
    constexpr int TIMEOUT_Y = 119;

    bool rowPressed(
        bool justPressed,
        uint16_t touchX,
        uint16_t touchY,
        int rowY
    )
    {
        return
            justPressed &&
            touchX >= ROW_X &&
            touchX < ROW_X + ROW_WIDTH &&
            touchY >= rowY &&
            touchY < rowY + ROW_HEIGHT;
    }

    void drawRow(
        int y,
        const char* title,
        const char* value
    )
    {
        lcd.drawRoundRect(
            ROW_X,
            y,
            ROW_WIDTH,
            ROW_HEIGHT,
            6,
            COLOR_GREEN
        );

        lcd.drawRoundRect(
            ROW_X + 2,
            y + 2,
            ROW_WIDTH - 4,
            ROW_HEIGHT - 4,
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
            title,
            ROW_X + 12,
            y + ROW_HEIGHT / 2
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_right
        );

        lcd.drawString(
            value,
            ROW_X + ROW_WIDTH - 12,
            y + ROW_HEIGHT / 2
        );
    }
}

void displaySettingsDraw()
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
        "DISPLAY SETTINGS",
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

    drawRow(
        BRIGHTNESS_Y,
        "BRIGHTNESS",
        "100%"
    );

    drawRow(
        TIMEOUT_Y,
        "SCREEN TIMEOUT",
        "60 SEC"
    );

    backButtonDraw();

    lcd.setTextDatum(
        lgfx::textdatum_t::top_left
    );
}

void displaySettingsUpdate(
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
        return;
    }

    if(
        rowPressed(
            justPressed,
            touchX,
            touchY,
            BRIGHTNESS_Y
        )
    )
    {
        Serial.println(
            "Brightness control selected."
        );

        touchReset();
        return;
    }

    if(
        rowPressed(
            justPressed,
            touchX,
            touchY,
            TIMEOUT_Y
        )
    )
    {
        Serial.println(
            "Screen timeout selected."
        );

        touchReset();
    }
}