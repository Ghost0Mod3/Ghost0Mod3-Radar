#pragma once

#include <Arduino.h>
#include <stdint.h>

/*
 * BLE device categories.
 *
 * Categories ending in "Possible" are classification hints only.
 * They do not confirm a device's exact identity or product type.
 */
enum class DeviceClass : uint8_t
{
    Ignored = 0,

    // Possible ALPR-related vendor matches
    FlockPossible,
    AxonPossible,

    // BLE health or wearable device
    HeartRateSensor,

    // Possible phone-brand hints
    ApplePossible,
    SamsungPossible,
    GooglePossible,
    MotorolaPossible,
    OnePlusPossible,
    XiaomiPossible,
    HuaweiPossible,

    // General possible phone hint
    PhonePossible,

    // Generic BLE advertisement accepted in open-scanning mode
    ApprovedGeneric
};

/*
 * Result returned by classifyDevice().
 */
struct Classification
{
    // Classification category
    DeviceClass type;

    // Short label displayed by the radar
    const char* label;

    // RGB565 target color
    uint16_t color;

    // True when the device is accepted by the classifier
    bool accepted;
};

/*
 * Classify a BLE advertisement using:
 *
 * - Heart Rate Service detection
 * - MAC/OUI rules
 * - Advertised-name rules
 * - Generic BLE fallback when CUSTOM DEVICES is enabled
 *
 * address:
 *     BLE address formatted as AA:BB:CC:DD:EE:FF
 *
 * advertisedName:
 *     Name included in the BLE advertisement, if available
 *
 * advertisesHeartRateService:
 *     True when the advertisement contains Heart Rate Service 0x180D
 */
Classification classifyDevice(
    const char* address,
    const char* advertisedName,
    bool advertisesHeartRateService
);

/*
 * Return true when the locally administered bit is set
 * in the first address byte.
 *
 * Locally administered BLE addresses should not be used
 * as reliable manufacturer OUI identifiers.
 */
bool isLocallyAdministeredAddress(
    const char* address
);

/*
 * Convert a device category to a readable label.
 */
const char* deviceClassToString(
    DeviceClass type
);

/*
 * Category helper functions for UI filtering,
 * logging, and future detail screens.
 */
bool isAlprClass(
    DeviceClass type
);

bool isPhoneClass(
    DeviceClass type
);

bool isHeartRateClass(
    DeviceClass type
);