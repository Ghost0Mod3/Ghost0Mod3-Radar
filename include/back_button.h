#pragma once

#include <stdint.h>

/*
 * Standard Back button position.
 *
 * Located in the bottom-left corner of the
 * 320 x 240 landscape display.
 */
constexpr int BACK_BUTTON_X = 4;
constexpr int BACK_BUTTON_Y = 202;
constexpr int BACK_BUTTON_WIDTH = 72;
constexpr int BACK_BUTTON_HEIGHT = 34;

/*
 * Draw the standard Ghost0Mod3 Back button.
 */
void backButtonDraw();

/*
 * Return true when the supplied touch coordinates
 * are inside the Back button.
 */
bool backButtonContains(
    uint16_t touchX,
    uint16_t touchY
);

/*
 * Return true when a new touch press occurred
 * inside the Back button.
 */
bool backButtonPressed(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
);