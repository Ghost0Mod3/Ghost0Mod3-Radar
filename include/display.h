#pragma once

#include <Arduino.h>
#include <LovyanGFX.hpp>

/*
 * LovyanGFX hardware configuration for:
 *
 * Display: 2.8-inch ILI9341, 240x320
 * Touch:   XPT2046 resistive touchscreen
 * Board:   ESP32 CYD-style board
 */
class LGFX : public lgfx::LGFX_Device
{
private:
    // LCD panel controller
    lgfx::Panel_ILI9341 panel;

    // LCD SPI bus
    lgfx::Bus_SPI bus;

    // Resistive touchscreen controller
    lgfx::Touch_XPT2046 touch;

public:
    /*
     * The constructor configures:
     *
     * - Display SPI pins
     * - ILI9341 panel
     * - XPT2046 touch controller
     * - Touch SPI pins
     *
     * The actual hardware initialization happens later
     * when initDisplay() calls lcd.init().
     */
    LGFX();
};

/*
 * Shared display instance.
 *
 * display.cpp creates this object.
 * Other modules access the same object through this declaration.
 */
extern LGFX lcd;

/*
 * Initialize the display, touchscreen, rotation, and backlight.
 */
void initDisplay();

/*
 * Show the Ghost0Mod3 startup screen.
 */
void showSplash();

/*
 * Clear the display using a black background.
 */
void clearScreen();