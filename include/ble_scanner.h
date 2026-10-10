#pragma once

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include "device_classifier.h"

/*
 * Maximum number of classified BLE devices stored
 * by the scanner at one time.
 */
constexpr size_t MAX_BLE_TARGETS = 12;

/*
 * One BLE device displayed on the radar.
 */
struct BleTarget
{
    /*
     * BLE address formatted as:
     * AA:BB:CC:DD:EE:FF
     */
    char address[18];

    /*
     * Advertised BLE name.
     *
     * Contains "Unknown" when the advertisement does not
     * provide a readable device name.
     */
    char name[24];

    /*
     * Short classification label, such as:
     *
     * FLOCK?
     * AXON?
     * HEART RATE
     * APPLE?
     * BLE DEVICE
     */
    char classification[16];

    /*
     * Most recent raw RSSI measurement.
     */
    int rawRssi;

    /*
     * Smoothed RSSI value used for radar positioning.
     */
    float smoothedRssi;

    /*
     * Stable visual angle generated from the device address.
     *
     * This is only for display positioning and is not a
     * measured physical direction.
     */
    float angle;

    /*
     * millis() timestamp of the most recent advertisement.
     */
    uint32_t lastSeen;

    /*
     * RGB565 target color selected by the classifier.
     */
    uint16_t color;

    /*
     * Category selected by the device classifier.
     */
    DeviceClass deviceClass;

    /*
     * True while the target remains active.
     */
    bool active;
};

/*
 * Initialize NimBLE and start continuous scanning.
 */
void bleScannerInit();

/*
 * Maintain scanner state, remove stale devices, and restart
 * scanning if it stops unexpectedly.
 *
 * Call continuously from loop().
 */
void bleScannerUpdate();

/*
 * Copy active radar targets into a caller-owned array.
 *
 * destination:
 *     Array that receives copies of active targets.
 *
 * maximumTargets:
 *     Maximum number of target entries that can be copied.
 *
 * Returns:
 *     Number of targets copied.
 */
size_t bleScannerGetTargets(
    BleTarget* destination,
    size_t maximumTargets
);

/*
 * Return the number of active classified targets.
 */
size_t bleScannerGetDeviceCount();

/*
 * Return true while NimBLE is scanning.
 */
bool bleScannerIsRunning();

/*
 * Immediately clear all currently stored radar targets.
 *
 * Use this whenever one or more classification filters change.
 * New advertisements can repopulate the radar according to the
 * updated filter settings.
 */
void bleScannerClearTargets();