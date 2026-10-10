#pragma once

/*
 * Runtime BLE classifier settings.
 *
 * The settings page modifies these values immediately.
 *
 * Current behavior:
 *
 * showAlpr:
 *     Show possible Flock and Axon matches.
 *
 * showHeartRate:
 *     Show devices advertising the standard BLE
 *     Heart Rate Service.
 *
 * showPhones:
 *     Show possible phone-brand hints based on
 *     advertised BLE names.
 *
 * showCustom:
 *     Show custom devices and enable the temporary
 *     open-scanning fallback for generic BLE devices.
 */
struct ClassifierSettings
{
    bool showAlpr;
    bool showHeartRate;
    bool showPhones;
    bool showCustom;
};

/*
 * Shared application settings object.
 *
 * Defined in:
 * src/classifier_settings.cpp
 */
extern ClassifierSettings classifierSettings;

/*
 * Restore default settings:
 *
 * ALPR matches:    ON
 * Heart rate:      ON
 * Phone hints:     OFF
 * Custom/all BLE:  ON
 */
void resetClassifierSettings();