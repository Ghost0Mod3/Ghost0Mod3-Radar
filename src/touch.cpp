#include <Arduino.h>

#include "display.h"
#include "touch.h"

namespace
{
    TouchState currentTouch =
    {
        0,      // x
        0,      // y
        false,  // pressed
        false,  // justPressed
        false   // justReleased
    };

    bool previousPressed = false;

    /*
     * Minimum delay between touchscreen polls.
     */
    constexpr uint32_t TOUCH_POLL_INTERVAL_MS = 15;

    /*
     * Your touchscreen reports the horizontal axis backward.
     *
     * Physical bottom-right:
     *   Reported X approximately 30
     *
     * Corrected X:
     *   319 - 30 = 289
     */
    constexpr bool MIRROR_TOUCH_X = true;
    constexpr bool MIRROR_TOUCH_Y = false;

    uint32_t previousPollTime = 0;
}

void touchInit()
{
    currentTouch.x = 0;
    currentTouch.y = 0;

    currentTouch.pressed = false;
    currentTouch.justPressed = false;
    currentTouch.justReleased = false;

    previousPressed = false;
    previousPollTime = 0;

    Serial.println("Touch module initialized.");
}

void touchUpdate()
{
    const uint32_t currentTime = millis();

    /*
     * These events remain true for only one update cycle.
     */
    currentTouch.justPressed = false;
    currentTouch.justReleased = false;

    /*
     * Limit touchscreen polling frequency.
     */
    if(
        currentTime - previousPollTime <
        TOUCH_POLL_INTERVAL_MS
    )
    {
        return;
    }

    previousPollTime = currentTime;

    uint16_t touchX = 0;
    uint16_t touchY = 0;

    const bool isPressed =
        lcd.getTouch(
            &touchX,
            &touchY
        );

    if(isPressed)
    {
        /*
         * Correct the horizontally mirrored touchscreen.
         */
        if(MIRROR_TOUCH_X)
        {
            touchX =
                static_cast<uint16_t>(
                    lcd.width() - 1 - touchX
                );
        }

        /*
         * Y is currently correct, but this option is
         * available if Y is discovered to be reversed later.
         */
        if(MIRROR_TOUCH_Y)
        {
            touchY =
                static_cast<uint16_t>(
                    lcd.height() - 1 - touchY
                );
        }

        /*
         * Clamp coordinates to the visible display bounds.
         */
        if(touchX >= lcd.width())
        {
            touchX =
                static_cast<uint16_t>(
                    lcd.width() - 1
                );
        }

        if(touchY >= lcd.height())
        {
            touchY =
                static_cast<uint16_t>(
                    lcd.height() - 1
                );
        }

        currentTouch.x = touchX;
        currentTouch.y = touchY;
    }

    currentTouch.pressed = isPressed;

    /*
     * A new press starts when the current reading is pressed
     * but the previous reading was not pressed.
     */
    currentTouch.justPressed =
        isPressed &&
        !previousPressed;

    /*
     * A release occurs when the current reading is not pressed
     * but the previous reading was pressed.
     */
    currentTouch.justReleased =
        !isPressed &&
        previousPressed;

    if(currentTouch.justPressed)
    {
        Serial.print("Touch pressed: X=");
        Serial.print(currentTouch.x);

        Serial.print(" Y=");
        Serial.println(currentTouch.y);
    }

    if(currentTouch.justReleased)
    {
        Serial.println("Touch released.");
    }

    previousPressed = isPressed;
}

const TouchState& touchGetState()
{
    return currentTouch;
}

bool touchWasPressed()
{
    return currentTouch.justPressed;
}

bool touchIsPressed()
{
    return currentTouch.pressed;
}

bool touchInside(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height
)
{
    if(!currentTouch.pressed)
    {
        return false;
    }

    if(width <= 0 || height <= 0)
    {
        return false;
    }

    return
        currentTouch.x >= x &&
        currentTouch.x < x + width &&
        currentTouch.y >= y &&
        currentTouch.y < y + height;
}

void touchReset()
{
    currentTouch.x = 0;
    currentTouch.y = 0;

    currentTouch.pressed = false;
    currentTouch.justPressed = false;
    currentTouch.justReleased = false;

    previousPressed = false;
}