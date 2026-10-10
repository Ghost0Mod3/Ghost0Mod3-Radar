#include <Arduino.h>
#include <Preferences.h>

#include "wifi_credential_store.h"

namespace
{
    /*
     * Preferences namespace and keys must remain short.
     */
    constexpr const char* STORAGE_NAMESPACE =
        "ghostwifi";

    constexpr const char* KEY_SSID =
        "ssid";

    constexpr const char* KEY_PASSWORD =
        "password";

    constexpr const char* KEY_VALID =
        "valid";

    bool storageReady = false;

    bool openPreferences(
        Preferences& preferences,
        bool readOnly
    )
    {
        return preferences.begin(
            STORAGE_NAMESPACE,
            readOnly
        );
    }
}

bool wifiCredentialStoreInit()
{
    Preferences preferences;

    if(
        !openPreferences(
            preferences,
            true
        )
    )
    {
        storageReady = false;

        Serial.println(
            "ERROR: Could not initialize Wi-Fi credential storage."
        );

        return false;
    }

    preferences.end();

    storageReady = true;

    Serial.println(
        "Wi-Fi credential storage initialized."
    );

    return true;
}

bool wifiCredentialStoreSave(
    const String& ssid,
    const String& password
)
{
    if(!storageReady)
    {
        Serial.println(
            "Cannot save Wi-Fi credentials: storage unavailable."
        );

        return false;
    }

    if(ssid.length() == 0)
    {
        Serial.println(
            "Cannot save Wi-Fi credentials: SSID is empty."
        );

        return false;
    }

    Preferences preferences;

    if(
        !openPreferences(
            preferences,
            false
        )
    )
    {
        Serial.println(
            "Could not open Wi-Fi credential storage for writing."
        );

        return false;
    }

    const size_t ssidBytes =
        preferences.putString(
            KEY_SSID,
            ssid
        );

    /*
     * An open network may have an empty password.
     * putString() is still called so an old password is replaced.
     */
    preferences.putString(
        KEY_PASSWORD,
        password
    );

    const size_t validBytes =
        preferences.putBool(
            KEY_VALID,
            true
        );

    preferences.end();

    const bool saved =
        ssidBytes > 0 &&
        validBytes > 0;

    if(saved)
    {
        Serial.print(
            "Saved Wi-Fi network: "
        );

        Serial.println(ssid);
    }
    else
    {
        Serial.println(
            "ERROR: Wi-Fi credentials were not saved."
        );
    }

    return saved;
}

bool wifiCredentialStoreLoad(
    String& ssid,
    String& password
)
{
    ssid = "";
    password = "";

    if(!storageReady)
    {
        return false;
    }

    Preferences preferences;

    if(
        !openPreferences(
            preferences,
            true
        )
    )
    {
        Serial.println(
            "Could not open Wi-Fi credential storage for reading."
        );

        return false;
    }

    const bool valid =
        preferences.getBool(
            KEY_VALID,
            false
        );

    if(valid)
    {
        ssid =
            preferences.getString(
                KEY_SSID,
                ""
            );

        password =
            preferences.getString(
                KEY_PASSWORD,
                ""
            );
    }

    preferences.end();

    if(
        !valid ||
        ssid.length() == 0
    )
    {
        ssid = "";
        password = "";

        return false;
    }

    Serial.print(
        "Loaded saved Wi-Fi network: "
    );

    Serial.println(ssid);

    return true;
}

bool wifiCredentialStoreClear()
{
    if(!storageReady)
    {
        return false;
    }

    Preferences preferences;

    if(
        !openPreferences(
            preferences,
            false
        )
    )
    {
        Serial.println(
            "Could not open Wi-Fi credential storage for clearing."
        );

        return false;
    }

    /*
     * Clear only this namespace.
     */
    const bool cleared =
        preferences.clear();

    preferences.end();

    if(cleared)
    {
        Serial.println(
            "Saved Wi-Fi credentials cleared."
        );
    }
    else
    {
        Serial.println(
            "No Wi-Fi credentials were cleared."
        );
    }

    return cleared;
}

bool wifiCredentialStoreIsReady()
{
    return storageReady;
}

bool wifiCredentialStoreHasSavedNetwork()
{
    if(!storageReady)
    {
        return false;
    }

    Preferences preferences;

    if(
        !openPreferences(
            preferences,
            true
        )
    )
    {
        return false;
    }

    const bool valid =
        preferences.getBool(
            KEY_VALID,
            false
        );

    const String ssid =
        preferences.getString(
            KEY_SSID,
            ""
        );

    preferences.end();

    return
        valid &&
        ssid.length() > 0;
}