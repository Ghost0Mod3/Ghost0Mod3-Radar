#include <Arduino.h>

#include "app_ui.h"
#include "back_button.h"
#include "display.h"
#include "sound_settings.h"
#include "touch.h"

namespace
{
    constexpr uint16_t COLOR_BACKGROUND = 0x0000;
    constexpr uint16_t COLOR_GREEN = 0x07E0;
    constexpr uint16_t COLOR_DARK_GREEN = 0x0320;
    constexpr uint16_t COLOR_DIM_GREEN = 0x4208;

    constexpr int ROW_X = 12;
    constexpr int ROW_WIDTH = 296;
    constexpr int ROW_HEIGHT = 48;

    constexpr int UI_SOUND_Y = 55;
    constexpr int RADAR_SOUND_Y = 115;

    bool uiSoundsEnabled = false;
    bool radarAlertsEnabled = false;

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

    void drawSoundRow(
        int y,
        const char* title,
        bool enabled
    )
    {
        lcd.drawRoundRect(
            ROW_X,
            y,
            ROW_WIDTH,
            ROW_HEIGHT,
            6,
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

        lcd.setTextColor(
            enabled
                ? COLOR_GREEN
                : COLOR_DIM_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            enabled ? "ON" : "OFF",
            ROW_X + ROW_WIDTH - 12,
            y + ROW_HEIGHT / 2
        );
    }
}

void soundSettingsDraw()
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
        "SOUND SETTINGS",
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

    drawSoundRow(
        UI_SOUND_Y,
        "UI SOUNDS",
        uiSoundsEnabled
    );

    drawSoundRow(
        RADAR_SOUND_Y,
        "RADAR ALERTS",
        radarAlertsEnabled
    );

    backButtonDraw();

    lcd.setTextDatum(
        lgfx::textdatum_t::top_left
    );
}

void soundSettingsUpdate(
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
            UI_SOUND_Y
        )
    )
    {
        uiSoundsEnabled =
            !uiSoundsEnabled;

        soundSettingsDraw();
        touchReset();
        return;
    }

    if(
        rowPressed(
            justPressed,
            touchX,
            touchY,
            RADAR_SOUND_Y
        )
    )
    {
        radarAlertsEnabled =
            !radarAlertsEnabled;

        soundSettingsDraw();
        touchReset();
    }
}