#include <Arduino.h>
#include <WiFi.h>

#include "app_ui.h"
#include "back_button.h"
#include "display.h"
#include "touch.h"
#include "wifi_credential_store.h"
#include "wifi_settings.h"

namespace
{
    constexpr uint16_t COLOR_BACKGROUND = 0x0000;
    constexpr uint16_t COLOR_GREEN = 0x07E0;
    constexpr uint16_t COLOR_DARK_GREEN = 0x0320;
    constexpr uint16_t COLOR_DIM_GREEN = 0x4208;

    constexpr size_t MAX_VISIBLE_NETWORKS = 4;
    constexpr size_t MAX_STORED_NETWORKS = 8;
    constexpr size_t MAX_PASSWORD_LENGTH = 63;

    constexpr uint32_t CONNECTION_TIMEOUT_MS = 15000;

    enum class WiFiPage : uint8_t
    {
        NetworkList,
        Keyboard,
        Connecting,
        Connected,
        Message
    };

    enum class KeyboardMode : uint8_t
    {
        Lowercase,
        Uppercase,
        Symbols
    };

    struct ScannedNetwork
    {
        String ssid;
        int32_t rssi;
        bool open;
    };

    WiFiPage currentPage =
        WiFiPage::NetworkList;

    KeyboardMode keyboardMode =
        KeyboardMode::Lowercase;

    ScannedNetwork networks[MAX_STORED_NETWORKS];

    size_t networkCount = 0;
    size_t firstVisibleNetwork = 0;

    String selectedSsid;
    String passwordText;
    String messageText;

    uint32_t connectionStartTime = 0;

    bool initialized = false;

    /*
     * Network-list layout.
     */
    constexpr int NETWORK_X = 8;
    constexpr int NETWORK_WIDTH = 304;
    constexpr int NETWORK_HEIGHT = 29;

    constexpr int NETWORK_ROWS[MAX_VISIBLE_NETWORKS] =
    {
        47,
        80,
        113,
        146
    };

    /*
     * Network-list controls.
     */
    constexpr int SCAN_X = 92;
    constexpr int SCAN_Y = 202;
    constexpr int SCAN_WIDTH = 92;
    constexpr int SCAN_HEIGHT = 34;

    constexpr int NEXT_X = 204;
    constexpr int NEXT_Y = 202;
    constexpr int NEXT_WIDTH = 108;
    constexpr int NEXT_HEIGHT = 34;

    /*
     * Touch keyboard layout.
     */
    constexpr int KEY_START_X = 5;
    constexpr int KEY_START_Y = 60;
    constexpr int KEY_WIDTH = 31;
    constexpr int KEY_HEIGHT = 25;
    constexpr int KEY_COLUMNS = 10;
    constexpr int KEY_ROWS = 4;

    constexpr int MODE_X = 82;
    constexpr int MODE_Y = 169;
    constexpr int MODE_WIDTH = 65;
    constexpr int MODE_HEIGHT = 28;

    constexpr int BACKSPACE_X = 152;
    constexpr int BACKSPACE_Y = 169;
    constexpr int BACKSPACE_WIDTH = 74;
    constexpr int BACKSPACE_HEIGHT = 28;

    constexpr int CONNECT_X = 231;
    constexpr int CONNECT_Y = 169;
    constexpr int CONNECT_WIDTH = 84;
    constexpr int CONNECT_HEIGHT = 28;

    /*
     * Connected-page controls.
     */
    constexpr int DISCONNECT_X = 90;
    constexpr int DISCONNECT_Y = 151;
    constexpr int DISCONNECT_WIDTH = 140;
    constexpr int DISCONNECT_HEIGHT = 36;

    /*
     * Message-page controls.
     */
    constexpr int NETWORK_LIST_X = 90;
    constexpr int NETWORK_LIST_Y = 143;
    constexpr int NETWORK_LIST_WIDTH = 140;
    constexpr int NETWORK_LIST_HEIGHT = 36;

    const char* LOWERCASE_KEYS[KEY_ROWS] =
    {
        "1234567890",
        "qwertyuiop",
        "asdfghjkl;",
        "zxcvbnm.-_"
    };

    const char* UPPERCASE_KEYS[KEY_ROWS] =
    {
        "1234567890",
        "QWERTYUIOP",
        "ASDFGHJKL:",
        "ZXCVBNM,?_"
    };

    const char* SYMBOL_KEYS[KEY_ROWS] =
    {
        "!@#$%^&*()",
        "[]{}<>?/\\|",
        "+-=_:;,.~`",
        "'\"        "
    };

    bool pointInside(
        uint16_t touchX,
        uint16_t touchY,
        int x,
        int y,
        int width,
        int height
    )
    {
        return
            touchX >= x &&
            touchX < x + width &&
            touchY >= y &&
            touchY < y + height;
    }

    void drawHeader(
        const char* title
    )
    {
        lcd.setFont(&fonts::Font2);

        lcd.setTextDatum(
            lgfx::textdatum_t::top_center
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            title,
            lcd.width() / 2,
            6
        );

        lcd.drawLine(
            45,
            30,
            lcd.width() - 45,
            30,
            COLOR_DARK_GREEN
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::top_left
        );
    }

    void drawButton(
        int x,
        int y,
        int width,
        int height,
        const char* label,
        bool active = true
    )
    {
        const uint16_t color =
            active
                ? COLOR_GREEN
                : COLOR_DIM_GREEN;

        lcd.fillRoundRect(
            x,
            y,
            width,
            height,
            5,
            COLOR_BACKGROUND
        );

        lcd.drawRoundRect(
            x,
            y,
            width,
            height,
            5,
            color
        );

        if(active)
        {
            lcd.drawRoundRect(
                x + 2,
                y + 2,
                width - 4,
                height - 4,
                4,
                COLOR_DARK_GREEN
            );
        }

        lcd.setFont(&fonts::Font0);

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_center
        );

        lcd.setTextColor(
            color,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            label,
            x + width / 2,
            y + height / 2
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::top_left
        );
    }

    const char* keyboardRow(
        size_t row
    )
    {
        switch(keyboardMode)
        {
            case KeyboardMode::Uppercase:
                return UPPERCASE_KEYS[row];

            case KeyboardMode::Symbols:
                return SYMBOL_KEYS[row];

            case KeyboardMode::Lowercase:
            default:
                return LOWERCASE_KEYS[row];
        }
    }

    const char* keyboardModeLabel()
    {
        switch(keyboardMode)
        {
            case KeyboardMode::Uppercase:
                return "ABC";

            case KeyboardMode::Symbols:
                return "SYM";

            case KeyboardMode::Lowercase:
            default:
                return "abc";
        }
    }

    void drawConnectionStatus()
    {
        lcd.setFont(&fonts::Font0);

        lcd.setTextDatum(
            lgfx::textdatum_t::top_center
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        if(WiFi.status() == WL_CONNECTED)
        {
            String status =
                "CONNECTED: " +
                WiFi.SSID() +
                " " +
                String(WiFi.RSSI()) +
                " DBM";

            if(status.length() > 48)
            {
                status =
                    status.substring(0, 48);
            }

            lcd.drawString(
                status,
                lcd.width() / 2,
                34
            );
        }
        else if(
            WiFi.status() ==
            WL_IDLE_STATUS
        )
        {
            lcd.drawString(
                "STATUS: CONNECTING",
                lcd.width() / 2,
                34
            );
        }
        else
        {
            lcd.drawString(
                "STATUS: DISCONNECTED",
                lcd.width() / 2,
                34
            );
        }

        lcd.setTextDatum(
            lgfx::textdatum_t::top_left
        );
    }

    void drawNetworkRow(
        size_t visibleIndex,
        size_t networkIndex
    )
    {
        if(
            visibleIndex >=
                MAX_VISIBLE_NETWORKS ||
            networkIndex >=
                networkCount
        )
        {
            return;
        }

        const int rowY =
            NETWORK_ROWS[visibleIndex];

        const ScannedNetwork& network =
            networks[networkIndex];

        lcd.fillRoundRect(
            NETWORK_X,
            rowY,
            NETWORK_WIDTH,
            NETWORK_HEIGHT,
            4,
            COLOR_BACKGROUND
        );

        lcd.drawRoundRect(
            NETWORK_X,
            rowY,
            NETWORK_WIDTH,
            NETWORK_HEIGHT,
            4,
            COLOR_DARK_GREEN
        );

        lcd.setFont(&fonts::Font0);

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_left
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        String displayName =
            network.ssid;

        if(displayName.length() > 24)
        {
            displayName =
                displayName.substring(
                    0,
                    24
                );
        }

        lcd.drawString(
            displayName,
            NETWORK_X + 8,
            rowY + NETWORK_HEIGHT / 2
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_right
        );

        String details =
            String(network.rssi) +
            " " +
            (network.open
                ? "OPEN"
                : "LOCK");

        lcd.drawString(
            details,
            NETWORK_X +
                NETWORK_WIDTH -
                8,
            rowY +
                NETWORK_HEIGHT /
                2
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::top_left
        );
    }

    void drawNetworkList()
    {
        lcd.fillScreen(
            COLOR_BACKGROUND
        );

        drawHeader(
            "WI-FI SETTINGS"
        );

        drawConnectionStatus();

        if(networkCount == 0)
        {
            lcd.setFont(
                &fonts::Font2
            );

            lcd.setTextDatum(
                lgfx::textdatum_t::middle_center
            );

            lcd.setTextColor(
                COLOR_DARK_GREEN,
                COLOR_BACKGROUND
            );

            lcd.drawString(
                "PRESS SCAN",
                lcd.width() / 2,
                112
            );
        }
        else
        {
            for(
                size_t visible = 0;
                visible <
                    MAX_VISIBLE_NETWORKS;
                visible++
            )
            {
                drawNetworkRow(
                    visible,
                    firstVisibleNetwork +
                        visible
                );
            }
        }

        backButtonDraw();

        drawButton(
            SCAN_X,
            SCAN_Y,
            SCAN_WIDTH,
            SCAN_HEIGHT,
            "SCAN"
        );

        const bool multiplePages =
            networkCount >
            MAX_VISIBLE_NETWORKS;

        drawButton(
            NEXT_X,
            NEXT_Y,
            NEXT_WIDTH,
            NEXT_HEIGHT,
            firstVisibleNetwork == 0
                ? "NEXT"
                : "FIRST",
            multiplePages
        );
    }

    void scanForNetworks()
    {
        lcd.fillScreen(
            COLOR_BACKGROUND
        );

        drawHeader(
            "WI-FI SETTINGS"
        );

        lcd.setFont(
            &fonts::Font2
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_center
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            "SCANNING...",
            lcd.width() / 2,
            lcd.height() / 2
        );

        Serial.println(
            "Starting Wi-Fi scan."
        );

        WiFi.mode(
            WIFI_STA
        );

        WiFi.scanDelete();

        const int found =
            WiFi.scanNetworks();

        networkCount = 0;
        firstVisibleNetwork = 0;

        if(found > 0)
        {
            for(
                int index = 0;
                index < found &&
                networkCount <
                    MAX_STORED_NETWORKS;
                index++
            )
            {
                const String ssid =
                    WiFi.SSID(index);

                if(ssid.length() == 0)
                {
                    continue;
                }

                bool duplicate = false;

                for(
                    size_t existing = 0;
                    existing <
                        networkCount;
                    existing++
                )
                {
                    if(
                        networks[
                            existing
                        ].ssid ==
                        ssid
                    )
                    {
                        duplicate = true;
                        break;
                    }
                }

                if(duplicate)
                {
                    continue;
                }

                networks[
                    networkCount
                ].ssid = ssid;

                networks[
                    networkCount
                ].rssi =
                    WiFi.RSSI(index);

                networks[
                    networkCount
                ].open =
                    WiFi.encryptionType(
                        index
                    ) ==
                    WIFI_AUTH_OPEN;

                networkCount++;
            }
        }

        WiFi.scanDelete();

        Serial.print(
            "Wi-Fi networks stored: "
        );

        Serial.println(
            networkCount
        );

        currentPage =
            WiFiPage::NetworkList;

        drawNetworkList();
        touchReset();
    }

    String maskedPassword()
    {
        String masked;

        for(
            size_t index = 0;
            index <
                passwordText.length();
            index++
        )
        {
            masked += '*';
        }

        return masked;
    }

    void drawKeyboard()
    {
        lcd.fillScreen(
            COLOR_BACKGROUND
        );

        drawHeader(
            "WI-FI PASSWORD"
        );

        lcd.setFont(
            &fonts::Font0
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::top_center
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        String networkTitle =
            "SSID: " +
            selectedSsid;

        if(networkTitle.length() > 42)
        {
            networkTitle =
                networkTitle.substring(
                    0,
                    42
                );
        }

        lcd.drawString(
            networkTitle,
            lcd.width() / 2,
            33
        );

        String passwordDisplay =
            "PASS: " +
            maskedPassword();

        if(passwordDisplay.length() > 46)
        {
            passwordDisplay =
                passwordDisplay.substring(
                    passwordDisplay.length() -
                        46
                );
        }

        lcd.drawString(
            passwordDisplay,
            lcd.width() / 2,
            45
        );

        for(
            int row = 0;
            row < KEY_ROWS;
            row++
        )
        {
            const char* keys =
                keyboardRow(row);

            for(
                int column = 0;
                column < KEY_COLUMNS;
                column++
            )
            {
                const char character =
                    keys[column];

                const int keyX =
                    KEY_START_X +
                    column *
                        KEY_WIDTH;

                const int keyY =
                    KEY_START_Y +
                    row *
                        KEY_HEIGHT;

                lcd.drawRect(
                    keyX,
                    keyY,
                    KEY_WIDTH - 2,
                    KEY_HEIGHT - 2,
                    COLOR_DARK_GREEN
                );

                if(character != ' ')
                {
                    char label[2] =
                    {
                        character,
                        '\0'
                    };

                    lcd.setTextDatum(
                        lgfx::textdatum_t::
                            middle_center
                    );

                    lcd.setTextColor(
                        COLOR_GREEN,
                        COLOR_BACKGROUND
                    );

                    lcd.drawString(
                        label,
                        keyX +
                            (KEY_WIDTH - 2) /
                                2,
                        keyY +
                            (KEY_HEIGHT - 2) /
                                2
                    );
                }
            }
        }

        drawButton(
            MODE_X,
            MODE_Y,
            MODE_WIDTH,
            MODE_HEIGHT,
            keyboardModeLabel()
        );

        drawButton(
            BACKSPACE_X,
            BACKSPACE_Y,
            BACKSPACE_WIDTH,
            BACKSPACE_HEIGHT,
            "DELETE"
        );

        drawButton(
            CONNECT_X,
            CONNECT_Y,
            CONNECT_WIDTH,
            CONNECT_HEIGHT,
            "CONNECT"
        );

        backButtonDraw();

        lcd.setTextDatum(
            lgfx::textdatum_t::top_left
        );
    }

    void drawConnecting()
    {
        lcd.fillScreen(
            COLOR_BACKGROUND
        );

        drawHeader(
            "WI-FI SETTINGS"
        );

        lcd.setFont(
            &fonts::Font2
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_center
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            "CONNECTING...",
            lcd.width() / 2,
            92
        );

        lcd.setFont(
            &fonts::Font0
        );

        String networkName =
            selectedSsid;

        if(networkName.length() > 38)
        {
            networkName =
                networkName.substring(
                    0,
                    38
                );
        }

        lcd.drawString(
            networkName,
            lcd.width() / 2,
            120
        );

        lcd.drawString(
            "PLEASE WAIT",
            lcd.width() / 2,
            145
        );

        backButtonDraw();
    }

    void drawConnected()
    {
        lcd.fillScreen(
            COLOR_BACKGROUND
        );

        drawHeader(
            "WI-FI CONNECTED"
        );

        lcd.setFont(
            &fonts::Font2
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_center
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        String connectedSsid =
            WiFi.SSID();

        if(connectedSsid.length() > 30)
        {
            connectedSsid =
                connectedSsid.substring(
                    0,
                    30
                );
        }

        lcd.drawString(
            connectedSsid,
            lcd.width() / 2,
            72
        );

        lcd.setFont(
            &fonts::Font0
        );

        lcd.drawString(
            "IP: " +
                WiFi.localIP().
                    toString(),
            lcd.width() / 2,
            103
        );

        lcd.drawString(
            "SIGNAL: " +
                String(WiFi.RSSI()) +
                " DBM",
            lcd.width() / 2,
            123
        );

        lcd.drawString(
            wifiCredentialStoreHasSavedNetwork()
                ? "SAVED: YES"
                : "SAVED: NO",
            lcd.width() / 2,
            139
        );

        drawButton(
            DISCONNECT_X,
            DISCONNECT_Y,
            DISCONNECT_WIDTH,
            DISCONNECT_HEIGHT,
            "FORGET NETWORK"
        );

        backButtonDraw();
    }

    void drawMessage()
    {
        lcd.fillScreen(
            COLOR_BACKGROUND
        );

        drawHeader(
            "WI-FI SETTINGS"
        );

        lcd.setFont(
            &fonts::Font2
        );

        lcd.setTextDatum(
            lgfx::textdatum_t::middle_center
        );

        lcd.setTextColor(
            COLOR_GREEN,
            COLOR_BACKGROUND
        );

        lcd.drawString(
            messageText,
            lcd.width() / 2,
            105
        );

        drawButton(
            NETWORK_LIST_X,
            NETWORK_LIST_Y,
            NETWORK_LIST_WIDTH,
            NETWORK_LIST_HEIGHT,
            "NETWORK LIST"
        );

        backButtonDraw();
    }

    void beginConnection()
    {
        WiFi.mode(
            WIFI_STA
        );

        /*
         * Cancel any prior connection attempt without
         * erasing the credential store.
         */
        WiFi.disconnect(
            false,
            false
        );

        delay(50);

        Serial.print(
            "Connecting to Wi-Fi: "
        );

        Serial.println(
            selectedSsid
        );

        if(passwordText.length() == 0)
        {
            WiFi.begin(
                selectedSsid.c_str()
            );
        }
        else
        {
            WiFi.begin(
                selectedSsid.c_str(),
                passwordText.c_str()
            );
        }

        connectionStartTime =
            millis();

        currentPage =
            WiFiPage::Connecting;

        drawConnecting();
        touchReset();
    }

    void selectNetwork(
        size_t networkIndex
    )
    {
        if(networkIndex >= networkCount)
        {
            return;
        }

        selectedSsid =
            networks[
                networkIndex
            ].ssid;

        passwordText = "";

        if(
            networks[
                networkIndex
            ].open
        )
        {
            beginConnection();
        }
        else
        {
            currentPage =
                WiFiPage::Keyboard;

            keyboardMode =
                KeyboardMode::Lowercase;

            drawKeyboard();
            touchReset();
        }
    }

    void updateConnecting()
    {
        if(
            WiFi.status() ==
            WL_CONNECTED
        )
        {
            currentPage =
                WiFiPage::Connected;

            const bool saved =
                wifiCredentialStoreSave(
                    selectedSsid,
                    passwordText
                );

            if(saved)
            {
                Serial.println(
                    "Wi-Fi credentials saved to Preferences."
                );
            }
            else
            {
                Serial.println(
                    "Wi-Fi connected, but credentials were not saved."
                );
            }

            Serial.print(
                "Wi-Fi connected. IP: "
            );

            Serial.println(
                WiFi.localIP()
            );

            drawConnected();
            touchReset();
            return;
        }

        if(
            millis() -
                connectionStartTime >=
            CONNECTION_TIMEOUT_MS
        )
        {
            WiFi.disconnect(
                false,
                false
            );

            messageText =
                "CONNECTION FAILED";

            currentPage =
                WiFiPage::Message;

            Serial.println(
                "Wi-Fi connection timed out."
            );

            drawMessage();
            touchReset();
        }
    }

    void addKeyboardCharacter(
        uint16_t touchX,
        uint16_t touchY
    )
    {
        if(
            touchX < KEY_START_X ||
            touchY < KEY_START_Y
        )
        {
            return;
        }

        const int column =
            (
                touchX -
                KEY_START_X
            ) /
            KEY_WIDTH;

        const int row =
            (
                touchY -
                KEY_START_Y
            ) /
            KEY_HEIGHT;

        if(
            column < 0 ||
            column >= KEY_COLUMNS ||
            row < 0 ||
            row >= KEY_ROWS
        )
        {
            return;
        }

        const char character =
            keyboardRow(row)[column];

        if(
            character == ' ' ||
            passwordText.length() >=
                MAX_PASSWORD_LENGTH
        )
        {
            return;
        }

        passwordText += character;

        drawKeyboard();
        touchReset();
    }

    void cycleKeyboardMode()
    {
        switch(keyboardMode)
        {
            case KeyboardMode::Lowercase:
                keyboardMode =
                    KeyboardMode::Uppercase;
                break;

            case KeyboardMode::Uppercase:
                keyboardMode =
                    KeyboardMode::Symbols;
                break;

            case KeyboardMode::Symbols:
            default:
                keyboardMode =
                    KeyboardMode::Lowercase;
                break;
        }

        drawKeyboard();
        touchReset();
    }
}

void wifiSettingsInit()
{
    if(initialized)
    {
        return;
    }

    WiFi.mode(
        WIFI_STA
    );

    const bool storageStarted =
        wifiCredentialStoreInit();

    if(!storageStarted)
    {
        Serial.println(
            "Wi-Fi credentials will not persist."
        );
    }

    String savedSsid;
    String savedPassword;

    if(
        storageStarted &&
        wifiCredentialStoreLoad(
            savedSsid,
            savedPassword
        )
    )
    {
        selectedSsid =
            savedSsid;

        passwordText =
            savedPassword;

        Serial.print(
            "Attempting saved Wi-Fi network: "
        );

        Serial.println(
            selectedSsid
        );

        if(passwordText.length() == 0)
        {
            WiFi.begin(
                selectedSsid.c_str()
            );
        }
        else
        {
            WiFi.begin(
                selectedSsid.c_str(),
                passwordText.c_str()
            );
        }
    }

    initialized = true;

    Serial.println(
        "Wi-Fi Settings initialized."
    );
}

void wifiSettingsDraw()
{
    wifiSettingsInit();

    if(
        WiFi.status() ==
            WL_CONNECTED &&
        currentPage ==
            WiFiPage::NetworkList
    )
    {
        currentPage =
            WiFiPage::Connected;
    }

    switch(currentPage)
    {
        case WiFiPage::Keyboard:
            drawKeyboard();
            break;

        case WiFiPage::Connecting:
            drawConnecting();
            break;

        case WiFiPage::Connected:
            drawConnected();
            break;

        case WiFiPage::Message:
            drawMessage();
            break;

        case WiFiPage::NetworkList:
        default:
            drawNetworkList();
            break;
    }
}

void wifiSettingsUpdate(
    bool justPressed,
    uint16_t touchX,
    uint16_t touchY
)
{
    /*
     * Connection progress must be checked continuously,
     * even when the screen is not being touched.
     */
    if(
        currentPage ==
        WiFiPage::Connecting
    )
    {
        updateConnecting();

        if(
            backButtonPressed(
                justPressed,
                touchX,
                touchY
            )
        )
        {
            WiFi.disconnect(
                false,
                false
            );

            currentPage =
                WiFiPage::NetworkList;

            touchReset();
            appUiGoBack();
        }

        return;
    }

    if(!justPressed)
    {
        return;
    }

    if(
        backButtonPressed(
            justPressed,
            touchX,
            touchY
        )
    )
    {
        if(
            currentPage ==
            WiFiPage::Keyboard
        )
        {
            currentPage =
                WiFiPage::NetworkList;

            drawNetworkList();
            touchReset();
            return;
        }

        if(
            currentPage ==
                WiFiPage::Connected ||
            currentPage ==
                WiFiPage::Message
        )
        {
            currentPage =
                WiFiPage::NetworkList;

            drawNetworkList();
            touchReset();
            return;
        }

        touchReset();
        appUiGoBack();
        return;
    }

    if(
        currentPage ==
        WiFiPage::NetworkList
    )
    {
        if(
            pointInside(
                touchX,
                touchY,
                SCAN_X,
                SCAN_Y,
                SCAN_WIDTH,
                SCAN_HEIGHT
            )
        )
        {
            scanForNetworks();
            return;
        }

        if(
            pointInside(
                touchX,
                touchY,
                NEXT_X,
                NEXT_Y,
                NEXT_WIDTH,
                NEXT_HEIGHT
            ) &&
            networkCount >
                MAX_VISIBLE_NETWORKS
        )
        {
            firstVisibleNetwork +=
                MAX_VISIBLE_NETWORKS;

            if(
                firstVisibleNetwork >=
                networkCount
            )
            {
                firstVisibleNetwork = 0;
            }

            drawNetworkList();
            touchReset();
            return;
        }

        for(
            size_t visible = 0;
            visible <
                MAX_VISIBLE_NETWORKS;
            visible++
        )
        {
            if(
                pointInside(
                    touchX,
                    touchY,
                    NETWORK_X,
                    NETWORK_ROWS[
                        visible
                    ],
                    NETWORK_WIDTH,
                    NETWORK_HEIGHT
                )
            )
            {
                selectNetwork(
                    firstVisibleNetwork +
                    visible
                );

                return;
            }
        }

        return;
    }

    if(
        currentPage ==
        WiFiPage::Keyboard
    )
    {
        if(
            pointInside(
                touchX,
                touchY,
                MODE_X,
                MODE_Y,
                MODE_WIDTH,
                MODE_HEIGHT
            )
        )
        {
            cycleKeyboardMode();
            return;
        }

        if(
            pointInside(
                touchX,
                touchY,
                BACKSPACE_X,
                BACKSPACE_Y,
                BACKSPACE_WIDTH,
                BACKSPACE_HEIGHT
            )
        )
        {
            if(passwordText.length() > 0)
            {
                passwordText.remove(
                    passwordText.length() -
                    1
                );
            }

            drawKeyboard();
            touchReset();
            return;
        }

        if(
            pointInside(
                touchX,
                touchY,
                CONNECT_X,
                CONNECT_Y,
                CONNECT_WIDTH,
                CONNECT_HEIGHT
            )
        )
        {
            beginConnection();
            return;
        }

        addKeyboardCharacter(
            touchX,
            touchY
        );

        return;
    }

    if(
        currentPage ==
        WiFiPage::Connected
    )
    {
        if(
            pointInside(
                touchX,
                touchY,
                DISCONNECT_X,
                DISCONNECT_Y,
                DISCONNECT_WIDTH,
                DISCONNECT_HEIGHT
            )
        )
        {
            WiFi.disconnect(
                true,
                false
            );

            wifiCredentialStoreClear();

            selectedSsid = "";
            passwordText = "";

            currentPage =
                WiFiPage::NetworkList;

            Serial.println(
                "Wi-Fi disconnected and saved credentials cleared."
            );

            drawNetworkList();
            touchReset();
        }

        return;
    }

    if(
        currentPage ==
        WiFiPage::Message
    )
    {
        if(
            pointInside(
                touchX,
                touchY,
                NETWORK_LIST_X,
                NETWORK_LIST_Y,
                NETWORK_LIST_WIDTH,
                NETWORK_LIST_HEIGHT
            )
        )
        {
            currentPage =
                WiFiPage::NetworkList;

            drawNetworkList();
            touchReset();
        }
    }
}