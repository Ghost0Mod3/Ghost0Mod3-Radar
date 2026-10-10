#include "classifier_settings.h"

/*
 * Shared runtime classifier settings.
 *
 * Default behavior after every restart:
 *
 * ALPR matches:       ON
 * Heart-rate devices: ON
 * Phone hints:        OFF
 * Custom/all BLE:     ON
 */
ClassifierSettings classifierSettings =
{
    true,   // showAlpr
    true,   // showHeartRate
    false,  // showPhones
    true    // showCustom / generic BLE fallback
};

void resetClassifierSettings()
{
    classifierSettings.showAlpr = true;
    classifierSettings.showHeartRate = true;
    classifierSettings.showPhones = false;
    classifierSettings.showCustom = true;
}