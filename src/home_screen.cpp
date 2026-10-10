#include <Arduino.h>

#include "display.h"
#include "home_screen.h"

namespace
{
    // Ghost0Mod3 color theme
    constexpr uint16_t COLOR_BACKGROUND = 0x0000;
    constexpr uint16_t COLOR_GREEN = 0x07E0;
    constexpr uint16_t COLOR_DARK_GREEN = 0x0320;
    constexpr uint16_t COLOR_DIM_GREEN = 0x4208;

    // App tile layout
    constexpr int TILE_WIDTH = 142;
    constexpr int TILE_HEIGHT = 62;

    constexpr int LEFT_TILE_X = 12;
    constexpr int RIGHT_TILE_X = 166;

    constexpr int TOP_TILE_Y = 54;
    constexpr int BOTTOM_TILE_Y = 128;

    void drawAppTile(
        int x,
        int y,
        const char* title,
        const char* subtitle,
        bool available
    )
    {
        const uint16_t borderColor =
            available
                ? COLOR_GREEN
                : COLOR_DIM_GREEN;

        const uint16_t textColor =
            available
                ? COLOR_GREEN
                : COLOR_DIM_GREEN;

        lcd.fillRoundRect(
            x,
            y,
            TILE_WIDTH,
            TILE_HEIGHT,
            6,
            COLOR_BACKGROUND
        );

        lcd.drawRoundRect(
            x,
            y,
            TILE_WIDTH,
            TILE_HEIGHT,
            6,
            borderColor
        );

        // Add an inner border to the active app.
        if(available)
        {
            lcd.drawRoundRect(
                x + 2,
                y + 2,
                TILE_WIDTH - 4,
                TILE_HEIGHT - 4,
                5,
                COLOR_DARK_GREEN
            );
        }

        lcd.setFont(&fonts::Font2);

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_center
        );

        lcd.setTextColor(
            textColor,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            title,
            x + TILE_WIDTH / 2,
            y + 22
        );

        lcd.setFont(&fonts::Font0);

        lcd.setTextColor(
            available
                ? COLOR_DARK_GREEN
                : COLOR_DIM_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            subtitle,
            x + TILE_WIDTH / 2,
            y + 44
        );
    }
}

void homeScreenDraw()
{
    lcd.fillScreen(COLOR_BACKGROUND);

    // Main heading
    lcd.setFont(&fonts::Font2);

    lcd.setTextDatum(
        lgfx::textdatum_t::top_center
    );

    lcd.setTextColor(
        COLOR_GREEN,
        COLOR_BACKGROUND
    );

    lcd.drawString(
        "GHOST0MOD3",
        lcd.width() / 2,
        8
    );

    // Small divider beneath the title
    lcd.drawLine(
        52,
        34,
        lcd.width() - 52,
        34,
        COLOR_DARK_GREEN
    );

    lcd.setFont(&fonts::Font0);

    lcd.setTextColor(
        COLOR_DARK_GREEN,
        COLOR_BACKGROUND
    );

    lcd.drawString(
        "SELECT APPLICATION",
        lcd.width() / 2,
        40
    );

    // Top-left: working radar application
    drawAppTile(
        LEFT_TILE_X,
        TOP_TILE_Y,
        "BLE RADAR",
        "READY",
        true
    );

    // Top-right: future application
    drawAppTile(
        RIGHT_TILE_X,
        TOP_TILE_Y,
        "APP 2",
        "COMING SOON",
        false
    );

    // Bottom-left: future application
    drawAppTile(
        LEFT_TILE_X,
        BOTTOM_TILE_Y,
        "APP 3",
        "COMING SOON",
        false
    );

    // Bottom-right: settings launcher
    drawAppTile(
        RIGHT_TILE_X,
        BOTTOM_TILE_Y,
        "SETTINGS",
        "FILTERS",
        true
    );

    // Footer
    lcd.setFont(&fonts::Font0);

    lcd.setTextDatum(
        lgfx::textdatum_t::bottom_center
    );

    lcd.setTextColor(
        COLOR_DARK_GREEN,
        COLOR_BACKGROUND
    );

    lcd.drawString(
        "WORKING PROTOTYPE v0.1",
        lcd.width() / 2,
        lcd.height() - 5
    );

    lcd.setTextDatum(
        lgfx::textdatum_t::top_left
    );
}