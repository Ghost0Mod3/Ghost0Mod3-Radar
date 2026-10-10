#pragma once

#include <Arduino.h>

/*
 * Initialize Wi-Fi credential storage.
 *
 * Preferences opens NVS only while performing an operation,
 * so this function records that the module is ready.
 */
bool wifiCredentialStoreInit();

/*
 * Save one Wi-Fi network.
 *
 * A successful save replaces the previously saved network.
 */
bool wifiCredentialStoreSave(
    const String& ssid,
    const String& password
);

/*
 * Load the saved SSID and password.
 *
 * Returns true only when a non-empty SSID was loaded.
 */
bool wifiCredentialStoreLoad(
    String& ssid,
    String& password
);

/*
 * Remove all saved Wi-Fi credentials.
 */
bool wifiCredentialStoreClear();

/*
 * Return true after successful module initialization.
 */
bool wifiCredentialStoreIsReady();

/*
 * Return true when a saved network exists.
 */
bool wifiCredentialStoreHasSavedNetwork();