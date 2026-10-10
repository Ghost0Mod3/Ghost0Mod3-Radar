#include <Arduino.h>

#include "app_ui.h"
#include "back_button.h"
#include "device_info.h"
#include "display.h"
#include "display_settings.h"
#include "home_screen.h"
#include "radar.h"
#include "radar_settings.h"
#include "sound_settings.h"
#include "system_settings.h"
#include "touch.h"
#include "wifi_settings.h"

void wifiSettingsDraw();
void wifiSettingsUpdate(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
);
void soundSettingsDraw();
void soundSettingsUpdate(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
);
void deviceInfoDraw();
void deviceInfoUpdate(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
);

namespace
{
    /*
     * Currently active screen.
     */
    AppScreen currentScreen =
        AppScreen::Home;

    /*
     * Navigation history.
     *
     * This allows nested navigation such as:
     *
     * Home
     * -> System Settings
     * -> Display Settings
     * -> Back
     * -> System Settings
     * -> Back
     * -> Home
     */
    constexpr size_t NAVIGATION_DEPTH = 8;

    AppScreen navigationStack[NAVIGATION_DEPTH] =
    {
        AppScreen::Home
    };

    size_t navigationCount = 0;

    /*
     * Ghost0Mod3 green-on-black theme.
     */
    constexpr uint16_t COLOR_BACKGROUND = 0x0000;
    constexpr uint16_t COLOR_GREEN = 0x07E0;
    constexpr uint16_t COLOR_DARK_GREEN = 0x0320;

    /*
     * Home-screen tile layout.
     *
     * These values must match home_screen.cpp.
     */
    constexpr int TILE_WIDTH = 142;
    constexpr int TILE_HEIGHT = 62;

    constexpr int LEFT_TILE_X = 12;
    constexpr int RIGHT_TILE_X = 166;

    constexpr int TOP_TILE_Y = 54;
    constexpr int BOTTOM_TILE_Y = 128;

    /*
     * Radar Settings button.
     *
     * The reusable Back button is in the bottom-left.
     */
    constexpr int RADAR_SETTINGS_X = 230;
    constexpr int RADAR_SETTINGS_Y = 202;
    constexpr int RADAR_SETTINGS_WIDTH = 86;
    constexpr int RADAR_SETTINGS_HEIGHT = 34;

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

        return pointInside(
            touchX,
            touchY,
            tileX,
            tileY,
            TILE_WIDTH,
            TILE_HEIGHT
        );
    }

    bool radarSettingsButtonPressed(
        bool justPressed,
        uint16_t touchX,
        uint16_t touchY
    )
    {
        if(!justPressed)
        {
            return false;
        }

        return pointInside(
            touchX,
            touchY,
            RADAR_SETTINGS_X,
            RADAR_SETTINGS_Y,
            RADAR_SETTINGS_WIDTH,
            RADAR_SETTINGS_HEIGHT
        );
    }

    void drawRadarSettingsButton()
    {
        /*
         * Clear the button area.
         */
        lcd.fillRoundRect(
            RADAR_SETTINGS_X,
            RADAR_SETTINGS_Y,
            RADAR_SETTINGS_WIDTH,
            RADAR_SETTINGS_HEIGHT,
            5,
            COLOR_BACKGROUND
        );

        /*
         * Bright-green outer border.
         */
        lcd.drawRoundRect(
            RADAR_SETTINGS_X,
            RADAR_SETTINGS_Y,
            RADAR_SETTINGS_WIDTH,
            RADAR_SETTINGS_HEIGHT,
            5,
            COLOR_GREEN
        );

        /*
         * Dark-green inner border.
         */
        lcd.drawRoundRect(
            RADAR_SETTINGS_X + 2,
            RADAR_SETTINGS_Y + 2,
            RADAR_SETTINGS_WIDTH - 4,
            RADAR_SETTINGS_HEIGHT - 4,
            4,
            COLOR_DARK_GREEN
        );

        lcd.setFont(&fonts::Font2);

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_center
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            "SETTINGS",
            RADAR_SETTINGS_X +
                RADAR_SETTINGS_WIDTH / 2,
            RADAR_SETTINGS_Y +
                RADAR_SETTINGS_HEIGHT / 2
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::top_left
        );
    }

    void pushNavigation(
        AppScreen screen
    )
    {
        /*
         * If the stack is full, remove the oldest entry.
         */
        if(navigationCount >= NAVIGATION_DEPTH)
        {
            for(
                size_t index = 1;
                index < NAVIGATION_DEPTH;
                index++
            )
            {
                navigationStack[index - 1] =
                    navigationStack[index];
            }

            navigationCount =
                NAVIGATION_DEPTH - 1;
        }

        navigationStack[navigationCount] =
            screen;

        navigationCount++;
    }

    void showScreen(
        AppScreen screen
    )
    {
        currentScreen = screen;

        clearScreen();
        touchReset();

        switch(screen)
        {
            case AppScreen::Home:
                homeScreenDraw();

                Serial.println(
                    "Screen changed: HOME"
                );
                break;

            case AppScreen::Radar:
                radarInit();

                Serial.println(
                    "Screen changed: RADAR"
                );
                break;

            case AppScreen::RadarSettings:
                radarSettingsDraw();

                Serial.println(
                    "Screen changed: RADAR SETTINGS"
                );
                break;

            case AppScreen::SystemSettings:
                systemSettingsDraw();

                Serial.println(
                    "Screen changed: SYSTEM SETTINGS"
                );
                break;

            case AppScreen::DisplaySettings:
                displaySettingsDraw();

                Serial.println(
                    "Screen changed: DISPLAY SETTINGS"
                );
                break;

            case AppScreen::WiFiSettings:
                wifiSettingsDraw();

                Serial.println(
                    "Screen changed: WI-FI SETTINGS"
                );
                break;

            case AppScreen::SoundSettings:
                soundSettingsDraw();

                Serial.println(
                    "Screen changed: SOUND SETTINGS"
                );
                break;

            case AppScreen::DeviceInfo:
                deviceInfoDraw();

                Serial.println(
                    "Screen changed: DEVICE INFO"
                );
                break;

            default:
                currentScreen =
                    AppScreen::Home;

                navigationCount = 0;

                homeScreenDraw();

                Serial.println(
                    "Unknown screen. Returned to HOME."
                );
                break;
        }
    }
}

void appUiInit()
{
    touchInit();

    navigationCount = 0;

    showScreen(
        AppScreen::Home
    );

    Serial.println(
        "Application UI initialized."
    );
}

void appUiSetScreen(
    AppScreen screen
)
{
    /*
     * Do not reopen the page that is already active.
     */
    if(screen == currentScreen)
    {
        return;
    }

    /*
     * Store the current page before opening the new page.
     */
    pushNavigation(
        currentScreen
    );

    showScreen(
        screen
    );
}

void appUiGoBack()
{
    /*
     * Home is the root page.
     */
    if(currentScreen == AppScreen::Home)
    {
        return;
    }

    /*
     * If navigation history is empty, return safely to Home.
     */
    if(navigationCount == 0)
    {
        showScreen(
            AppScreen::Home
        );

        return;
    }

    navigationCount--;

    const AppScreen destination =
        navigationStack[navigationCount];

    showScreen(
        destination
    );
}

AppScreen appUiGetScreen()
{
    return currentScreen;
}

void appUiUpdate()
{
    /*
     * Poll the touchscreen once per application update.
     */
    touchUpdate();

    const TouchState& touch =
        touchGetState();

    /*
     * HOME SCREEN
     */
    if(currentScreen == AppScreen::Home)
    {
        /*
         * Top-left tile:
         * BLE RADAR
         */
        if(
            tilePressed(
                touch.justPressed,
                touch.x,
                touch.y,
                LEFT_TILE_X,
                TOP_TILE_Y
            )
        )
        {
            appUiSetScreen(
                AppScreen::Radar
            );

            return;
        }

        /*
         * Bottom-right tile:
         * SYSTEM SETTINGS
         */
        if(
            tilePressed(
                touch.justPressed,
                touch.x,
                touch.y,
                RIGHT_TILE_X,
                BOTTOM_TILE_Y
            )
        )
        {
            appUiSetScreen(
                AppScreen::SystemSettings
            );

            return;
        }

        /*
         * Top-right tile:
         * APP 2 placeholder
         */
        if(
            tilePressed(
                touch.justPressed,
                touch.x,
                touch.y,
                RIGHT_TILE_X,
                TOP_TILE_Y
            )
        )
        {
            Serial.println(
                "APP 2 selected: COMING SOON"
            );

            touchReset();
            return;
        }

        /*
         * Bottom-left tile:
         * APP 3 placeholder
         */
        if(
            tilePressed(
                touch.justPressed,
                touch.x,
                touch.y,
                LEFT_TILE_X,
                BOTTOM_TILE_Y
            )
        )
        {
            Serial.println(
                "APP 3 selected: COMING SOON"
            );

            touchReset();
            return;
        }

        return;
    }

    /*
     * RADAR SCREEN
     */
    if(currentScreen == AppScreen::Radar)
    {
        /*
         * radarUpdate() redraws the radar screen.
         */
        radarUpdate();

        /*
         * Draw navigation afterward so the radar does not
         * erase the buttons.
         */
        backButtonDraw();
        drawRadarSettingsButton();

        if(
            backButtonPressed(
                touch.justPressed,
                touch.x,
                touch.y
            )
        )
        {
            touchReset();
            appUiGoBack();
            return;
        }

        if(
            radarSettingsButtonPressed(
                touch.justPressed,
                touch.x,
                touch.y
            )
        )
        {
            appUiSetScreen(
                AppScreen::RadarSettings
            );

            return;
        }

        return;
    }

    /*
     * RADAR SETTINGS
     */
    if(currentScreen == AppScreen::RadarSettings)
    {
        radarSettingsUpdate(
            touch.justPressed,
            touch.x,
            touch.y
        );

        return;
    }

    /*
     * SYSTEM SETTINGS
     */
    if(currentScreen == AppScreen::SystemSettings)
    {
        systemSettingsUpdate(
            touch.justPressed,
            touch.x,
            touch.y
        );

        return;
    }

    /*
     * DISPLAY SETTINGS
     */
    if(currentScreen == AppScreen::DisplaySettings)
    {
        displaySettingsUpdate(
            touch.justPressed,
            touch.x,
            touch.y
        );

        return;
    }

    /*
     * WI-FI SETTINGS
     */
    if(currentScreen == AppScreen::WiFiSettings)
    {
        wifiSettingsUpdate(
            touch.justPressed,
            touch.x,
            touch.y
        );

        return;
    }

    /*
     * SOUND SETTINGS
     */
    if(currentScreen == AppScreen::SoundSettings)
    {
        soundSettingsUpdate(
            touch.justPressed,
            touch.x,
            touch.y
        );

        return;
    }

    /*
     * DEVICE INFO
     */
    if(currentScreen == AppScreen::DeviceInfo)
    {
        deviceInfoUpdate(
            touch.justPressed,
            touch.x,
            touch.y
        );

        return;
    }
}