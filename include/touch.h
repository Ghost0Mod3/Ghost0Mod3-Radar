#pragma once

#include <Arduino.h>
#include <stdint.h>

/*
 * Current touchscreen state.
 *
 * Coordinates are corrected and mapped to the display:
 *
 * Landscape width:  320 pixels
 * Landscape height: 240 pixels
 */
struct TouchState
{
    // Latest corrected screen coordinates.
    uint16_t x;
    uint16_t y;

    // True while the touchscreen remains pressed.
    bool pressed;

    // True for one update cycle when a new press begins.
    bool justPressed;

    // True for one update cycle when the touch is released.
    bool justReleased;
};

/*
 * Initialize the touch-state tracking module.
 *
 * The physical XPT2046 controller is configured in display.cpp.
 */
void touchInit();

/*
 * Poll the touchscreen and update the current state.
 *
 * Call this continuously through appUiUpdate().
 */
void touchUpdate();

/*
 * Return the current touchscreen state.
 */
const TouchState& touchGetState();

/*
 * Return true only when a new touch has started.
 */
bool touchWasPressed();

/*
 * Return true while the screen remains pressed.
 */
bool touchIsPressed();

/*
 * Check whether the current corrected touch coordinate is
 * inside a rectangular area.
 *
 * x, y:
 *     Top-left corner of the rectangle.
 *
 * width, height:
 *     Size of the rectangle.
 */
bool touchInside(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height
);

/*
 * Clear the current touch state.
 *
 * Call this after changing screens so the original touch
 * does not immediately activate something on the new page.
 */
void touchReset();