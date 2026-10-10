#include <Arduino.h>
#include <math.h>

#include "ble_scanner.h"
#include "display.h"
#include "radar.h"

namespace
{
    constexpr int RADAR_CENTER_X = 160;
    constexpr int RADAR_CENTER_Y = 122;
    constexpr int RADAR_RADIUS = 88;

    constexpr int INNER_RING_RADIUS = 29;
    constexpr int MIDDLE_RING_RADIUS = 58;

    constexpr float SWEEP_SPEED = 3.0f;
    constexpr uint32_t FRAME_INTERVAL_MS = 40;

    constexpr uint16_t COLOR_BACKGROUND = 0x0000;
    constexpr uint16_t COLOR_GRID = 0x0320;
    constexpr uint16_t COLOR_SWEEP = 0x07E0;
    constexpr uint16_t COLOR_TARGET = 0xF800;
    constexpr uint16_t COLOR_STRONG = 0xFFE0;
    constexpr uint16_t COLOR_STATUS = 0x07FF;

    float sweepAngle = 0.0f;
    uint32_t previousFrameTime = 0;

    float rssiToRadius(float rssi)
    {
        // Clamp readings to a useful visual range.
        if(rssi > -35.0f)
        {
            rssi = -35.0f;
        }

        if(rssi < -95.0f)
        {
            rssi = -95.0f;
        }

        // Stronger signals appear nearer the center.
        const float normalized =
            (-35.0f - rssi) / 60.0f;

        return 14.0f +
            normalized *
            (RADAR_RADIUS - 18.0f);
    }

    uint16_t rssiToColor(float rssi)
    {
        if(rssi >= -60.0f)
        {
            return COLOR_STRONG;
        }

        return COLOR_TARGET;
    }

    void drawLiveTarget(const BleTarget& target)
    {
        const float radius =
            rssiToRadius(target.smoothedRssi);

        const float radians =
            target.angle * (PI / 180.0f);

        const int targetX =
            RADAR_CENTER_X +
            static_cast<int>(
                cosf(radians) * radius
            );

        const int targetY =
            RADAR_CENTER_Y +
            static_cast<int>(
                sinf(radians) * radius
            );

        const uint16_t color =
            rssiToColor(target.smoothedRssi);

        lcd.drawCircle(
            targetX,
            targetY,
            6,
            COLOR_GRID
        );

        lcd.fillCircle(
            targetX,
            targetY,
            3,
            color
        );
    }
}

void radarInit()
{
    sweepAngle = 0.0f;
    previousFrameTime = millis();

    lcd.fillScreen(COLOR_BACKGROUND);
}

void drawRadarGrid()
{
    lcd.drawCircle(
        RADAR_CENTER_X,
        RADAR_CENTER_Y,
        INNER_RING_RADIUS,
        COLOR_GRID
    );

    lcd.drawCircle(
        RADAR_CENTER_X,
        RADAR_CENTER_Y,
        MIDDLE_RING_RADIUS,
        COLOR_GRID
    );

    lcd.drawCircle(
        RADAR_CENTER_X,
        RADAR_CENTER_Y,
        RADAR_RADIUS,
        COLOR_GRID
    );

    lcd.drawLine(
        RADAR_CENTER_X - RADAR_RADIUS,
        RADAR_CENTER_Y,
        RADAR_CENTER_X + RADAR_RADIUS,
        RADAR_CENTER_Y,
        COLOR_GRID
    );

    lcd.drawLine(
        RADAR_CENTER_X,
        RADAR_CENTER_Y - RADAR_RADIUS,
        RADAR_CENTER_X,
        RADAR_CENTER_Y + RADAR_RADIUS,
        COLOR_GRID
    );

    lcd.fillCircle(
        RADAR_CENTER_X,
        RADAR_CENTER_Y,
        2,
        COLOR_SWEEP
    );
}

void drawRadarTarget(float angleDegrees, float radius)
{
    const float radians =
        angleDegrees * (PI / 180.0f);

    const int targetX =
        RADAR_CENTER_X +
        static_cast<int>(cosf(radians) * radius);

    const int targetY =
        RADAR_CENTER_Y +
        static_cast<int>(sinf(radians) * radius);

    lcd.fillCircle(
        targetX,
        targetY,
        3,
        COLOR_TARGET
    );
}

void drawRadarSweep()
{
    const float radians =
        sweepAngle * (PI / 180.0f);

    const int sweepX =
        RADAR_CENTER_X +
        static_cast<int>(
            cosf(radians) * RADAR_RADIUS
        );

    const int sweepY =
        RADAR_CENTER_Y +
        static_cast<int>(
            sinf(radians) * RADAR_RADIUS
        );

    lcd.drawLine(
        RADAR_CENTER_X,
        RADAR_CENTER_Y,
        sweepX,
        sweepY,
        COLOR_SWEEP
    );

    sweepAngle += SWEEP_SPEED;

    if(sweepAngle >= 360.0f)
    {
        sweepAngle -= 360.0f;
    }
}

void radarUpdate()
{
    const uint32_t currentTime = millis();

    if(currentTime - previousFrameTime < FRAME_INTERVAL_MS)
    {
        return;
    }

    previousFrameTime = currentTime;

    BleTarget targets[MAX_BLE_TARGETS];

    const size_t targetCount =
        bleScannerGetTargets(
            targets,
            MAX_BLE_TARGETS
        );

    lcd.fillScreen(COLOR_BACKGROUND);

    lcd.setTextDatum(lgfx::textdatum_t::top_center);
    lcd.setTextColor(COLOR_SWEEP, COLOR_BACKGROUND);

    lcd.setFont(&fonts::Font2);

    lcd.drawString(
        "GHOST0MOD3 RADAR",
        lcd.width() / 2,
        5
    );

    drawRadarGrid();
    drawRadarSweep();

    for(size_t index = 0; index < targetCount; index++)
    {
        drawLiveTarget(targets[index]);
    }

    lcd.setTextDatum(lgfx::textdatum_t::bottom_left);
    lcd.setTextColor(COLOR_STATUS, COLOR_BACKGROUND);

    String deviceText =
        "DEVICES: " + String(targetCount);

    lcd.drawString(
        deviceText,
        8,
        lcd.height() - 4
    );

    lcd.setTextDatum(lgfx::textdatum_t::bottom_right);

    lcd.drawString(
        bleScannerIsRunning() ? "SCANNING" : "STOPPED",
        lcd.width() - 8,
        lcd.height() - 4
    );

    lcd.setTextDatum(lgfx::textdatum_t::top_left);
}