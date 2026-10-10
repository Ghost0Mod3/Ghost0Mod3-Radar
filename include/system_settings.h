#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * Draw the device-wide System Settings menu.
 */
void systemSettingsDraw(void);

/*
 * Process touch input for System Settings.
 */
void systemSettingsUpdate(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
);