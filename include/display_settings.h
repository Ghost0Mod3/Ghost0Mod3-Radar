#pragma once

#include <stdint.h>

void displaySettingsDraw();

void displaySettingsUpdate(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
);