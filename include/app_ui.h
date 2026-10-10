#pragma once

#include <stdint.h>

enum class AppScreen : uint8_t
{
    Home = 0,
    Radar,
    RadarSettings,
    SystemSettings,
    DisplaySettings,
    WiFiSettings,
    SoundSettings,
    DeviceInfo
};

void appUiInit();
void appUiUpdate();

AppScreen appUiGetScreen();

void appUiSetScreen(
    AppScreen screen
);

void appUiGoBack();