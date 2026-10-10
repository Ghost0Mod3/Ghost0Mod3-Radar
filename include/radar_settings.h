#pragma once

#include <stdint.h>

/*
 * Draw the complete Radar Settings page.
 */
void radarSettingsDraw();

/*
 * Process touch input for Radar Settings.
 */
void radarSettingsUpdate(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
);