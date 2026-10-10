#include <Arduino.h>

#include "app_ui.h"
#include "back_button.h"
#include "display.h"
#include "system_settings.h"
#include "touch.h"

namespace
{
    // Ghost0Mod3 green-on-black theme
    constexpr uint16_t COLOR_BACKGROUND = 0x0000;
    constexpr uint16_t COLOR_GREEN = 0x07E0;
    constexpr uint16_t COLOR_DARK_GREEN = 0x0320;

    // Tile dimensions
    constexpr int TILE_WIDTH = 142;
    constexpr int TILE_HEIGHT = 62;

    // Tile positions
    constexpr int LEFT_TILE_X = 12;
    constexpr int RIGHT_TILE_X = 166;

    constexpr int TOP_TILE_Y = 54;
    constexpr int BOTTOM_TILE_Y = 128;

    bool tilePressed(
        bool justPressed,
        uint16_t touchX,
        uint16_t touchY,
        int tileX,
        int tileY
    )
    {
        if(!justPressed)
        {
            return false;
        }

        return
            touchX >= tileX &&
            touchX < tileX + TILE_WIDTH &&
            touchY >= tileY &&
            touchY < tileY + TILE_HEIGHT;
    }

    void drawSettingsTile(
        int x,
        int y,
        const char* title,
        const char* subtitle
    )
    {
        /*
         * Clear the area before drawing.
         */
        lcd.fillRoundRect(
            x,
            y,
            TILE_WIDTH,
            TILE_HEIGHT,
            6,
            COLOR_BACKGROUND
        );

        /*
         * Bright-green outer border.
         */
        lcd.drawRoundRect(
            x,
            y,
            TILE_WIDTH,
            TILE_HEIGHT,
            6,
            COLOR_GREEN
        );

        /*
         * Dark-green inner border.
         */
        lcd.drawRoundRect(
            x + 2,
            y + 2,
            TILE_WIDTH - 4,
            TILE_HEIGHT - 4,
            5,
            COLOR_DARK_GREEN
        );

        /*
         * Main title.
         */
        lcd.setFont(
            &fonts::Font2
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_center
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            title,
            x + TILE_WIDTH / 2,
            y + 22
        );

        /*
         * Subtitle.
         */
        lcd.setFont(
            &fonts::Font0
        );

        lcd.setTextColor(
            COLOR_DARK_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            subtitle,
            x + TILE_WIDTH / 2,
            y + 44
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::top_left
        );
    }
}

void systemSettingsDraw()
{
    lcd.fillScreen(
        COLOR_BACKGROUND
    );

    /*
     * Page heading.
     */
    lcd.setFont(
        &fonts::Font2
    );

    lcd.setTextDatum(
        lgfx::textdatum_t::top_center
    );

    lcd.setTextColor(
        COLOR_GREEN,
        COLOR_BACKGROUND
    );

    lcd.drawString(
        "SYSTEM SETTINGS",
        lcd.width() / 2,
        7
    );

    /*
     * Divider beneath heading.
     */
    lcd.drawLine(
        50,
        31,
        lcd.width() - 50,
        31,
        COLOR_DARK_GREEN
    );

    /*
     * Top-left:
     * Display and brightness controls.
     */
    drawSettingsTile(
        LEFT_TILE_X,
        TOP_TILE_Y,
        "DISPLAY",
        "BRIGHTNESS"
    );

    /*
     * Top-right:
     * Wi-Fi network controls.
     */
    drawSettingsTile(
        RIGHT_TILE_X,
        TOP_TILE_Y,
        "WI-FI",
        "NETWORK"
    );

    /*
     * Bottom-left:
     * Sound controls.
     */
    drawSettingsTile(
        LEFT_TILE_X,
        BOTTOM_TILE_Y,
        "SOUND",
        "AUDIO"
    );

    /*
     * Bottom-right:
     * Device status and information.
     */
    drawSettingsTile(
        RIGHT_TILE_X,
        BOTTOM_TILE_Y,
        "DEVICE INFO",
        "STATUS"
    );

    /*
     * Standard reusable Back button.
     */
    backButtonDraw();

    lcd.setTextDatum(
        lgfx::textdatum_t::top_left
    );
}

void systemSettingsUpdate(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
)
{
    if(!justPressed)
    {
        return;
    }

    Serial.print(
        "System Settings touch: X="
    );

    Serial.print(touchX);

    Serial.print(" Y=");
    Serial.println(touchY);

    /*
     * Handle the reusable Back button before
     * checking the larger settings tiles.
     */
    if(
        backButtonPressed(
            justPressed,
            touchX,
            touchY
        )
    )
    {
        Serial.println(
            "System Settings: BACK"
        );

        touchReset();
        appUiGoBack();
        return;
    }

    /*
     * Display Settings.
     */
    if(
        tilePressed(
            justPressed,
            touchX,
            touchY,
            LEFT_TILE_X,
            TOP_TILE_Y
        )
    )
    {
        Serial.println(
            "Opening Display Settings."
        );

        appUiSetScreen(
            AppScreen::DisplaySettings
        );

        return;
    }

    /*
     * Wi-Fi Settings.
     */
    if(
        tilePressed(
            justPressed,
            touchX,
            touchY,
            RIGHT_TILE_X,
            TOP_TILE_Y
        )
    )
    {
        Serial.println(
            "Opening Wi-Fi Settings."
        );

        appUiSetScreen(
            AppScreen::WiFiSettings
        );

        return;
    }
}
