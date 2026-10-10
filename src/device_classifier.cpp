#include "device_classifier.h"
#include "classifier_settings.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

namespace
{
    // RGB565 display colors
    constexpr uint16_t COLOR_IGNORED = 0x0000;
    constexpr uint16_t COLOR_FLOCK = 0xF800;
    constexpr uint16_t COLOR_AXON = 0xFFE0;
    constexpr uint16_t COLOR_HEART_RATE = 0xF81F;

    constexpr uint16_t COLOR_APPLE = 0xFFFF;
    constexpr uint16_t COLOR_SAMSUNG = 0x001F;
    constexpr uint16_t COLOR_GOOGLE = 0x07E0;
    constexpr uint16_t COLOR_MOTOROLA = 0x07FF;
    constexpr uint16_t COLOR_ONEPLUS = 0xF800;
    constexpr uint16_t COLOR_XIAOMI = 0xFD20;
    constexpr uint16_t COLOR_HUAWEI = 0xF81F;

    constexpr uint16_t COLOR_GENERIC_BLE = 0x07E0;

    struct OuiRule
    {
        const char* prefix;
        DeviceClass type;
        const char* label;
        uint16_t color;
        bool ClassifierSettings::* setting;
    };

    struct NameRule
    {
        const char* text;
        DeviceClass type;
        const char* label;
        uint16_t color;
        bool ClassifierSettings::* setting;
    };

    /*
     * Vendor/OUI rules.
     *
     * These are possible vendor associations only.
     * An OUI match does not confirm the exact device type.
     */
    constexpr OuiRule OUI_RULES[] =
    {
        {
            "B4:1E:52",
            DeviceClass::FlockPossible,
            "FLOCK?",
            COLOR_FLOCK,
            &ClassifierSettings::showAlpr
        },
        {
            "00:25:DF",
            DeviceClass::AxonPossible,
            "AXON?",
            COLOR_AXON,
            &ClassifierSettings::showAlpr
        }
    };

    /*
     * Advertised-name rules.
     *
     * These rules work only when a device broadcasts a
     * recognizable name in its BLE advertisement.
     */
    constexpr NameRule NAME_RULES[] =
    {
        // ALPR-related hints
        {
            "FLOCK",
            DeviceClass::FlockPossible,
            "FLOCK?",
            COLOR_FLOCK,
            &ClassifierSettings::showAlpr
        },
        {
            "AXON",
            DeviceClass::AxonPossible,
            "AXON?",
            COLOR_AXON,
            &ClassifierSettings::showAlpr
        },

        // Apple hints
        {
            "IPHONE",
            DeviceClass::ApplePossible,
            "APPLE?",
            COLOR_APPLE,
            &ClassifierSettings::showPhones
        },
        {
            "APPLE",
            DeviceClass::ApplePossible,
            "APPLE?",
            COLOR_APPLE,
            &ClassifierSettings::showPhones
        },
        {
            "APPLE WATCH",
            DeviceClass::ApplePossible,
            "APPLE WATCH?",
            COLOR_APPLE,
            &ClassifierSettings::showPhones
        },

        // Samsung hints
        {
            "SAMSUNG",
            DeviceClass::SamsungPossible,
            "SAMSUNG?",
            COLOR_SAMSUNG,
            &ClassifierSettings::showPhones
        },
        {
            "GALAXY",
            DeviceClass::SamsungPossible,
            "SAMSUNG?",
            COLOR_SAMSUNG,
            &ClassifierSettings::showPhones
        },

        // Google/Pixel hints
        {
            "GOOGLE",
            DeviceClass::GooglePossible,
            "GOOGLE?",
            COLOR_GOOGLE,
            &ClassifierSettings::showPhones
        },
        {
            "PIXEL",
            DeviceClass::GooglePossible,
            "PIXEL?",
            COLOR_GOOGLE,
            &ClassifierSettings::showPhones
        },

        // Motorola hints
        {
            "MOTOROLA",
            DeviceClass::MotorolaPossible,
            "MOTOROLA?",
            COLOR_MOTOROLA,
            &ClassifierSettings::showPhones
        },
        {
            "MOTO ",
            DeviceClass::MotorolaPossible,
            "MOTOROLA?",
            COLOR_MOTOROLA,
            &ClassifierSettings::showPhones
        },

        // OnePlus hints
        {
            "ONEPLUS",
            DeviceClass::OnePlusPossible,
            "ONEPLUS?",
            COLOR_ONEPLUS,
            &ClassifierSettings::showPhones
        },

        // Xiaomi/Redmi hints
        {
            "XIAOMI",
            DeviceClass::XiaomiPossible,
            "XIAOMI?",
            COLOR_XIAOMI,
            &ClassifierSettings::showPhones
        },
        {
            "REDMI",
            DeviceClass::XiaomiPossible,
            "XIAOMI?",
            COLOR_XIAOMI,
            &ClassifierSettings::showPhones
        },

        // Huawei hints
        {
            "HUAWEI",
            DeviceClass::HuaweiPossible,
            "HUAWEI?",
            COLOR_HUAWEI,
            &ClassifierSettings::showPhones
        }
    };

    Classification makeIgnoredClassification()
    {
        return
        {
            DeviceClass::Ignored,
            "IGNORED",
            COLOR_IGNORED,
            false
        };
    }

    Classification makeClassification(
        DeviceClass type,
        const char* label,
        uint16_t color
    )
    {
        return
        {
            type,
            label,
            color,
            true
        };
    }

    int hexValue(char character)
    {
        if(character >= '0' && character <= '9')
        {
            return character - '0';
        }

        character = static_cast<char>(
            toupper(
                static_cast<unsigned char>(character)
            )
        );

        if(character >= 'A' && character <= 'F')
        {
            return 10 + character - 'A';
        }

        return -1;
    }

    bool startsWithIgnoreCase(
        const char* text,
        const char* prefix
    )
    {
        if(text == nullptr || prefix == nullptr)
        {
            return false;
        }

        while(*prefix != '\0')
        {
            if(*text == '\0')
            {
                return false;
            }

            const char textCharacter =
                static_cast<char>(
                    toupper(
                        static_cast<unsigned char>(*text)
                    )
                );

            const char prefixCharacter =
                static_cast<char>(
                    toupper(
                        static_cast<unsigned char>(*prefix)
                    )
                );

            if(textCharacter != prefixCharacter)
            {
                return false;
            }

            text++;
            prefix++;
        }

        return true;
    }

    bool containsIgnoreCase(
        const char* text,
        const char* pattern
    )
    {
        if(
            text == nullptr ||
            pattern == nullptr ||
            pattern[0] == '\0'
        )
        {
            return false;
        }

        const size_t textLength = strlen(text);
        const size_t patternLength = strlen(pattern);

        if(patternLength > textLength)
        {
            return false;
        }

        for(
            size_t start = 0;
            start <= textLength - patternLength;
            start++
        )
        {
            bool matched = true;

            for(
                size_t offset = 0;
                offset < patternLength;
                offset++
            )
            {
                const char textCharacter =
                    static_cast<char>(
                        toupper(
                            static_cast<unsigned char>(
                                text[start + offset]
                            )
                        )
                    );

                const char patternCharacter =
                    static_cast<char>(
                        toupper(
                            static_cast<unsigned char>(
                                pattern[offset]
                            )
                        )
                    );

                if(textCharacter != patternCharacter)
                {
                    matched = false;
                    break;
                }
            }

            if(matched)
            {
                return true;
            }
        }

        return false;
    }

} // End anonymous namespace

/*
 * Public functions begin here.
 *
 * These functions must stay outside the anonymous namespace
 * so ble_scanner.cpp can link to them.
 */

bool isLocallyAdministeredAddress(
    const char* address
)
{
    if(address == nullptr || strlen(address) < 2)
    {
        return false;
    }

    const int highNibble = hexValue(address[0]);
    const int lowNibble = hexValue(address[1]);

    if(highNibble < 0 || lowNibble < 0)
    {
        return false;
    }

    const uint8_t firstByte =
        static_cast<uint8_t>(
            (highNibble << 4) | lowNibble
        );

    return (firstByte & 0x02U) != 0;
}

Classification classifyDevice(
    const char* address,
    const char* advertisedName,
    bool advertisesHeartRateService
)
{
    /*
     * Priority 1:
     * Standard BLE Heart Rate Service.
     */
    if(
        advertisesHeartRateService &&
        classifierSettings.showHeartRate
    )
    {
        return makeClassification(
            DeviceClass::HeartRateSensor,
            "HEART RATE",
            COLOR_HEART_RATE
        );
    }

    /*
     * Priority 2:
     * Vendor/OUI rules.
     *
     * Skip OUI classification for locally administered
     * addresses because their prefix is not a dependable
     * manufacturer identifier.
     */
    if(
        address != nullptr &&
        !isLocallyAdministeredAddress(address)
    )
    {
        for(const OuiRule& rule : OUI_RULES)
        {
            const bool categoryEnabled =
                classifierSettings.*(rule.setting);

            if(
                categoryEnabled &&
                startsWithIgnoreCase(
                    address,
                    rule.prefix
                )
            )
            {
                return makeClassification(
                    rule.type,
                    rule.label,
                    rule.color
                );
            }
        }
    }

    /*
     * Priority 3:
     * Advertised-name hints.
     */
    if(
        advertisedName != nullptr &&
        advertisedName[0] != '\0'
    )
    {
        for(const NameRule& rule : NAME_RULES)
        {
            const bool categoryEnabled =
                classifierSettings.*(rule.setting);

            if(
                categoryEnabled &&
                containsIgnoreCase(
                    advertisedName,
                    rule.text
                )
            )
            {
                return makeClassification(
                    rule.type,
                    rule.label,
                    rule.color
                );
            }
        }
    }

    /*
     * Priority 4:
     * Open prototype scanning.
     *
     * When CUSTOM DEVICES is enabled, accept every
     * remaining BLE advertisement that passes the RSSI
     * threshold in ble_scanner.cpp.
     */
    if(classifierSettings.showCustom)
    {
        return makeClassification(
            DeviceClass::ApprovedGeneric,
            "BLE DEVICE",
            COLOR_GENERIC_BLE
        );
    }

    return makeIgnoredClassification();
}

const char* deviceClassToString(
    DeviceClass type
)
{
    switch(type)
    {
        case DeviceClass::FlockPossible:
            return "FLOCK?";

        case DeviceClass::AxonPossible:
            return "AXON?";

        case DeviceClass::HeartRateSensor:
            return "HEART RATE";

        case DeviceClass::ApplePossible:
            return "APPLE?";

        case DeviceClass::SamsungPossible:
            return "SAMSUNG?";

        case DeviceClass::GooglePossible:
            return "GOOGLE?";

        case DeviceClass::MotorolaPossible:
            return "MOTOROLA?";

        case DeviceClass::OnePlusPossible:
            return "ONEPLUS?";

        case DeviceClass::XiaomiPossible:
            return "XIAOMI?";

        case DeviceClass::HuaweiPossible:
            return "HUAWEI?";

        case DeviceClass::PhonePossible:
            return "PHONE?";

        case DeviceClass::ApprovedGeneric:
            return "BLE DEVICE";

        case DeviceClass::Ignored:
        default:
            return "IGNORED";
    }
}

bool isAlprClass(
    DeviceClass type
)
{
    return
        type == DeviceClass::FlockPossible ||
        type == DeviceClass::AxonPossible;
}

bool isPhoneClass(
    DeviceClass type
)
{
    switch(type)
    {
        case DeviceClass::ApplePossible:
        case DeviceClass::SamsungPossible:
        case DeviceClass::GooglePossible:
        case DeviceClass::MotorolaPossible:
        case DeviceClass::OnePlusPossible:
        case DeviceClass::XiaomiPossible:
        case DeviceClass::HuaweiPossible:
        case DeviceClass::PhonePossible:
            return true;

        default:
            return false;
    }
}

bool isHeartRateClass(
    DeviceClass type
)
{
    return type == DeviceClass::HeartRateSensor;
}