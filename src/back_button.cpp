#include <Arduino.h>

#include "back_button.h"
#include "display.h"

namespace
{
    // Ghost0Mod3 green-on-black theme
    constexpr uint16_t COLOR_BACKGROUND = 0x0000;
    constexpr uint16_t COLOR_GREEN = 0x07E0;
    constexpr uint16_t COLOR_DARK_GREEN = 0x0320;
}

void backButtonDraw()
{
    /*
     * Clear the area behind the button.
     */
    lcd.fillRoundRect(
        BACK_BUTTON_X,
        BACK_BUTTON_Y,
        BACK_BUTTON_WIDTH,
        BACK_BUTTON_HEIGHT,
        5,
        COLOR_BACKGROUND
    );

    /*
     * Bright-green outer border.
     */
    lcd.drawRoundRect(
        BACK_BUTTON_X,
        BACK_BUTTON_Y,
        BACK_BUTTON_WIDTH,
        BACK_BUTTON_HEIGHT,
        5,
        COLOR_GREEN
    );

    /*
     * Dark-green inner border.
     */
    lcd.drawRoundRect(
        BACK_BUTTON_X + 2,
        BACK_BUTTON_Y + 2,
        BACK_BUTTON_WIDTH - 4,
        BACK_BUTTON_HEIGHT - 4,
        4,
        COLOR_DARK_GREEN
    );

    /*
     * Draw centered button label.
     */
    lcd.setFont(&fonts::Font2);

    lcd.setTextDatum(
        lgfx::textdatum_t::middle_center
    );

    lcd.setTextColor(
        COLOR_GREEN,
        COLOR_BACKGROUND
    );

    lcd.drawString(
        "BACK",
        BACK_BUTTON_X +
            BACK_BUTTON_WIDTH / 2,
        BACK_BUTTON_Y +
            BACK_BUTTON_HEIGHT / 2
    );

    /*
     * Restore the default text alignment for other screens.
     */
    lcd.setTextDatum(
        lgfx::textdatum_t::top_left
    );
}

bool backButtonContains(
    uint16_t touchX,
    uint16_t touchY
)
{
    return
        touchX >= BACK_BUTTON_X &&
        touchX <
            BACK_BUTTON_X +
            BACK_BUTTON_WIDTH &&
        touchY >= BACK_BUTTON_Y &&
        touchY <
            BACK_BUTTON_Y +
            BACK_BUTTON_HEIGHT;
}

bool backButtonPressed(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
)
{
    if(!justPressed)
    {
        return false;
    }

    return backButtonContains(
        touchX,
        touchY
    );
}