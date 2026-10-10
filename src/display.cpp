#include "display.h"

namespace
{
    // CYD backlight
    constexpr int TFT_BACKLIGHT_PIN = 21;

    // ILI9341 display SPI pins
    constexpr int TFT_SCLK_PIN = 14;
    constexpr int TFT_MOSI_PIN = 13;
    constexpr int TFT_MISO_PIN = 12;
    constexpr int TFT_DC_PIN = 2;
    constexpr int TFT_CS_PIN = 15;
    constexpr int TFT_RST_PIN = -1;

    // XPT2046 touchscreen SPI pins
    constexpr int TOUCH_SCLK_PIN = 25;
    constexpr int TOUCH_MOSI_PIN = 32;
    constexpr int TOUCH_MISO_PIN = 39;
    constexpr int TOUCH_CS_PIN = 33;

    /*
     * Set to -1 so LovyanGFX polls the touchscreen.
     * This avoids depending on the touch interrupt pin
     * while testing and calibrating.
     */
    constexpr int TOUCH_INTERRUPT_PIN = -1;

    // Initial XPT2046 raw calibration values
    constexpr int TOUCH_X_MIN = 300;
    constexpr int TOUCH_X_MAX = 3900;
    constexpr int TOUCH_Y_MIN = 300;
    constexpr int TOUCH_Y_MAX = 3900;
}

// Shared display object used by all project modules.
LGFX lcd;

LGFX::LGFX()
{
    /*
     * Configure the SPI bus used by the ILI9341 display.
     */
    {
        auto config = bus.config();

        config.spi_host = VSPI_HOST;
        config.spi_mode = 0;

        config.freq_write = 40000000;
        config.freq_read = 16000000;

        config.spi_3wire = false;
        config.use_lock = true;
        config.dma_channel = 1;

        config.pin_sclk = TFT_SCLK_PIN;
        config.pin_mosi = TFT_MOSI_PIN;
        config.pin_miso = TFT_MISO_PIN;
        config.pin_dc = TFT_DC_PIN;

        bus.config(config);
        panel.setBus(&bus);
    }

    /*
     * Configure the ILI9341 panel.
     */
    {
        auto config = panel.config();

        config.pin_cs = TFT_CS_PIN;
        config.pin_rst = TFT_RST_PIN;
        config.pin_busy = -1;

        config.panel_width = 240;
        config.panel_height = 320;

        config.memory_width = 240;
        config.memory_height = 320;

        config.offset_x = 0;
        config.offset_y = 0;
        config.offset_rotation = 0;

        config.readable = true;
        config.invert = false;
        config.rgb_order = false;
        config.dlen_16bit = false;

        /*
         * The touchscreen uses a separate SPI bus,
         * but the display bus may still be shared by
         * other hardware such as the SD card.
         */
        config.bus_shared = true;

        panel.config(config);
    }

    /*
     * Configure the XPT2046 resistive touchscreen.
     */
    {
        auto config = touch.config();

        /*
         * Raw calibration range.
         *
         * These values are a starting point. They may need
         * adjustment after reading coordinates from the
         * Serial Monitor.
         */
        config.x_min = TOUCH_X_MIN;
        config.x_max = TOUCH_X_MAX;
        config.y_min = TOUCH_Y_MIN;
        config.y_max = TOUCH_Y_MAX;

        /*
         * Poll the touchscreen instead of relying on IRQ.
         */
        config.pin_int = TOUCH_INTERRUPT_PIN;
        config.pin_rst = -1;

        /*
         * Touch uses HSPI and separate pins from the TFT.
         */
        config.bus_shared = false;
        config.offset_rotation = 0;

        config.spi_host = HSPI_HOST;
        config.freq = 1000000;

        config.pin_sclk = TOUCH_SCLK_PIN;
        config.pin_mosi = TOUCH_MOSI_PIN;
        config.pin_miso = TOUCH_MISO_PIN;
        config.pin_cs = TOUCH_CS_PIN;

        touch.config(config);

        /*
         * Attach touchscreen input to the LCD panel.
         * lcd.getTouch() will now use this controller.
         */
        panel.setTouch(&touch);
    }

    setPanel(&panel);
}

void initDisplay()
{
    Serial.println("Initializing display and touch...");

    /*
     * Turn on the CYD backlight.
     */
    pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
    digitalWrite(TFT_BACKLIGHT_PIN, HIGH);

    /*
     * Initialize display and attached touchscreen.
     */
    lcd.init();

    /*
     * Landscape orientation:
     *
     * Width  = 320
     * Height = 240
     */
    lcd.setRotation(1);

    lcd.setBrightness(255);
    lcd.setTextWrap(false);

    lcd.fillScreen(TFT_BLACK);

    Serial.print("Display size: ");
    Serial.print(lcd.width());
    Serial.print(" x ");
    Serial.println(lcd.height());

    Serial.println("Display and touch initialized.");
}

void showSplash()
{
    lcd.fillScreen(TFT_BLACK);

    lcd.setFont(&fonts::Font0);
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    lcd.setTextDatum(lgfx::textdatum_t::top_left);

    /*
     * Compact positioning for the landscape CYD screen.
     */
    lcd.setCursor(69, 34);

    lcd.println("      .k$$$$$g,");
    lcd.println("   .k$$$$$$$$$$$a.");
    lcd.println(" .J$$$$$?'   `?$?^?,");
    lcd.println("JS$$SI!a,  _.JS$   ?,");
    lcd.println("k$$$SI!:?$$$$$$$$$xu$$j");
    lcd.println(":I$$SI:J$$?*\"$$$$4^?*?:");
    lcd.println(":IS$$SiJ?`  _.'$?`/'  ':");
    lcd.println(" ?ISSik? _        ',");
    lcd.println("   ?i$?` _  k$");
    lcd.println("      '?I:-?z$$I");
    lcd.println("         i$$$$$$$$$$$S");
    lcd.println("            ?$$$?");

    lcd.setFont(&fonts::Font2);
    lcd.setTextDatum(
        lgfx::textdatum_t::middle_center
    );

    lcd.setTextColor(
        TFT_GREEN,
        TFT_BLACK
    );

    lcd.drawString(
        "BLE RADAR READY",
        lcd.width() / 2,
        207
    );

    delay(3000);

    lcd.setTextDatum(
        lgfx::textdatum_t::top_left
    );
}

void clearScreen()
{
    lcd.fillScreen(TFT_BLACK);

    lcd.setTextDatum(
        lgfx::textdatum_t::top_left
    );
}