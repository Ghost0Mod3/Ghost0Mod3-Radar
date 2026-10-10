#include <Arduino.h>
#include <NimBLEDevice.h>

#include <cstring>
#include <string>

#include "ble_scanner.h"
#include "device_classifier.h"

namespace
{
    constexpr float RSSI_ALPHA = 0.25f;
    constexpr uint32_t DEVICE_TIMEOUT_MS = 10000;

    /*
     * Accept -75 dBm and stronger.
     *
     * Accepted: -75, -70, -60, -50, -30
     * Rejected: -76, -80, -90
     */
    constexpr int MINIMUM_ACCEPTED_RSSI = -75;

    BleTarget targets[MAX_BLE_TARGETS] = {};

    NimBLEScan* scanner = nullptr;

    portMUX_TYPE targetMutex =
        portMUX_INITIALIZER_UNLOCKED;

    uint32_t hashAddress(const char* address)
    {
        uint32_t hash = 2166136261UL;

        if(address == nullptr)
        {
            return hash;
        }

        while(*address != '\0')
        {
            hash ^= static_cast<uint8_t>(*address);
            hash *= 16777619UL;
            address++;
        }

        return hash;
    }

    float addressToAngle(const char* address)
    {
        return static_cast<float>(
            hashAddress(address) % 360UL
        );
    }

    void copyText(
        char* destination,
        size_t destinationSize,
        const char* source
    )
    {
        if(
            destination == nullptr ||
            destinationSize == 0
        )
        {
            return;
        }

        if(source == nullptr)
        {
            destination[0] = '\0';
            return;
        }

        strncpy(
            destination,
            source,
            destinationSize - 1
        );

        destination[destinationSize - 1] = '\0';
    }

    int findTarget(const char* address)
    {
        if(address == nullptr)
        {
            return -1;
        }

        for(
            size_t index = 0;
            index < MAX_BLE_TARGETS;
            index++
        )
        {
            if(
                targets[index].active &&
                strcmp(
                    targets[index].address,
                    address
                ) == 0
            )
            {
                return static_cast<int>(index);
            }
        }

        return -1;
    }

    int findAvailableSlot()
    {
        for(
            size_t index = 0;
            index < MAX_BLE_TARGETS;
            index++
        )
        {
            if(!targets[index].active)
            {
                return static_cast<int>(index);
            }
        }

        size_t oldestIndex = 0;
        uint32_t oldestTime = targets[0].lastSeen;

        for(
            size_t index = 1;
            index < MAX_BLE_TARGETS;
            index++
        )
        {
            if(targets[index].lastSeen < oldestTime)
            {
                oldestIndex = index;
                oldestTime = targets[index].lastSeen;
            }
        }

        return static_cast<int>(oldestIndex);
    }

    void deactivateTarget(const char* address)
    {
        portENTER_CRITICAL(&targetMutex);

        const int existingIndex =
            findTarget(address);

        if(existingIndex >= 0)
        {
            targets[existingIndex].active = false;
        }

        portEXIT_CRITICAL(&targetMutex);
    }

    void updateTarget(
        const char* address,
        const char* name,
        int rssi,
        bool advertisesHeartRate
    )
    {
        if(
            address == nullptr ||
            address[0] == '\0'
        )
        {
            return;
        }

        /*
         * Remove the target when its latest signal is
         * weaker than -75 dBm.
         */
        if(rssi < MINIMUM_ACCEPTED_RSSI)
        {
            deactivateTarget(address);
            return;
        }

        const Classification classification =
            classifyDevice(
                address,
                name,
                advertisesHeartRate
            );

        if(!classification.accepted)
        {
            deactivateTarget(address);
            return;
        }

        const uint32_t now = millis();

        bool isNewTarget = false;

        char logAddress[18] = {};
        char logName[24] = {};
        char logClassification[16] = {};

        portENTER_CRITICAL(&targetMutex);

        int index = findTarget(address);

        if(index < 0)
        {
            index = findAvailableSlot();

            BleTarget& target = targets[index];

            memset(
                &target,
                0,
                sizeof(BleTarget)
            );

            copyText(
                target.address,
                sizeof(target.address),
                address
            );

            if(
                name != nullptr &&
                name[0] != '\0'
            )
            {
                copyText(
                    target.name,
                    sizeof(target.name),
                    name
                );
            }
            else
            {
                copyText(
                    target.name,
                    sizeof(target.name),
                    "Unknown"
                );
            }

            copyText(
                target.classification,
                sizeof(target.classification),
                classification.label
            );

            target.rawRssi = rssi;
            target.smoothedRssi =
                static_cast<float>(rssi);

            /*
             * Stable visual angle derived from the address.
             * This does not represent physical direction.
             */
            target.angle =
                addressToAngle(address);

            target.lastSeen = now;
            target.color = classification.color;
            target.deviceClass = classification.type;
            target.active = true;

            copyText(
                logAddress,
                sizeof(logAddress),
                target.address
            );

            copyText(
                logName,
                sizeof(logName),
                target.name
            );

            copyText(
                logClassification,
                sizeof(logClassification),
                target.classification
            );

            isNewTarget = true;
        }
        else
        {
            BleTarget& target = targets[index];

            target.rawRssi = rssi;

            /*
             * Exponential moving average:
             * 25% new reading and 75% previous value.
             */
            target.smoothedRssi =
                RSSI_ALPHA *
                    static_cast<float>(rssi) +
                (1.0f - RSSI_ALPHA) *
                    target.smoothedRssi;

            if(
                name != nullptr &&
                name[0] != '\0' &&
                strcmp(target.name, "Unknown") == 0
            )
            {
                copyText(
                    target.name,
                    sizeof(target.name),
                    name
                );
            }

            copyText(
                target.classification,
                sizeof(target.classification),
                classification.label
            );

            target.color = classification.color;
            target.deviceClass = classification.type;
            target.lastSeen = now;
            target.active = true;
        }

        portEXIT_CRITICAL(&targetMutex);

        if(isNewTarget)
        {
            Serial.print("MATCH: ");
            Serial.print(logClassification);

            Serial.print(" | NAME: ");
            Serial.print(logName);

            Serial.print(" | ADDRESS: ");
            Serial.print(logAddress);

            Serial.print(" | RSSI: ");
            Serial.println(rssi);
        }
    }

    class RadarScanCallbacks :
        public NimBLEScanCallbacks
    {
    public:
        void onResult(
            const NimBLEAdvertisedDevice* device
        ) override
        {
            if(device == nullptr)
            {
                return;
            }

            const std::string address =
                device->getAddress().toString();

            std::string deviceName;

            if(device->haveName())
            {
                deviceName = device->getName();
            }

            /*
             * Standard BLE Heart Rate Service: 0x180D.
             * This detects the service only and does not read BPM.
             */
            const bool advertisesHeartRate =
                device->isAdvertisingService(
                    NimBLEUUID("180D")
                );

            updateTarget(
                address.c_str(),
                deviceName.c_str(),
                device->getRSSI(),
                advertisesHeartRate
            );
        }

        void onScanEnd(
            const NimBLEScanResults& results,
            int reason
        ) override
        {
            (void)results;

            Serial.print(
                "BLE scan ended. Reason: "
            );

            Serial.println(reason);
        }
    };

    RadarScanCallbacks scanCallbacks;

} // End anonymous namespace

void bleScannerInit()
{
    Serial.println(
        "Starting classified BLE scanner..."
    );

    portENTER_CRITICAL(&targetMutex);

    memset(
        targets,
        0,
        sizeof(targets)
    );

    portEXIT_CRITICAL(&targetMutex);

    NimBLEDevice::init("");

    scanner = NimBLEDevice::getScan();

    if(scanner == nullptr)
    {
        Serial.println(
            "ERROR: Could not create BLE scanner."
        );

        return;
    }

    scanner->setScanCallbacks(
        &scanCallbacks,
        true
    );

    /*
     * Passive scanning detects normal BLE advertisements.
     */
    scanner->setActiveScan(false);

    scanner->setInterval(100);
    scanner->setWindow(80);

    /*
     * Allow repeated reports so RSSI can update.
     */
    scanner->setDuplicateFilter(0);

    /*
     * Targets are stored in our own array.
     */
    scanner->setMaxResults(0);

    const bool started =
        scanner->start(0);

    if(started)
    {
        Serial.println(
            "Classified BLE scanner running."
        );

        Serial.print(
            "Minimum accepted RSSI: "
        );

        Serial.print(
            MINIMUM_ACCEPTED_RSSI
        );

        Serial.println(" dBm");
    }
    else
    {
        Serial.println(
            "ERROR: BLE scanner failed to start."
        );
    }
}

void bleScannerUpdate()
{
    const uint32_t now = millis();

    portENTER_CRITICAL(&targetMutex);

    for(
        size_t index = 0;
        index < MAX_BLE_TARGETS;
        index++
    )
    {
        if(
            targets[index].active &&
            now - targets[index].lastSeen >
                DEVICE_TIMEOUT_MS
        )
        {
            targets[index].active = false;
        }
    }

    portEXIT_CRITICAL(&targetMutex);

    if(
        scanner != nullptr &&
        !scanner->isScanning()
    )
    {
        const bool restarted =
            scanner->start(0);

        if(restarted)
        {
            Serial.println(
                "BLE scanner restarted."
            );
        }
    }
}

size_t bleScannerGetTargets(
    BleTarget* destination,
    size_t maximumTargets
)
{
    if(
        destination == nullptr ||
        maximumTargets == 0
    )
    {
        return 0;
    }

    size_t copied = 0;

    portENTER_CRITICAL(&targetMutex);

    for(
        size_t index = 0;
        index < MAX_BLE_TARGETS &&
        copied < maximumTargets;
        index++
    )
    {
        if(targets[index].active)
        {
            destination[copied] =
                targets[index];

            copied++;
        }
    }

    portEXIT_CRITICAL(&targetMutex);

    return copied;
}

size_t bleScannerGetDeviceCount()
{
    size_t count = 0;

    portENTER_CRITICAL(&targetMutex);

    for(
        size_t index = 0;
        index < MAX_BLE_TARGETS;
        index++
    )
    {
        if(targets[index].active)
        {
            count++;
        }
    }

    portEXIT_CRITICAL(&targetMutex);

    return count;
}

bool bleScannerIsRunning()
{
    return
        scanner != nullptr &&
        scanner->isScanning();
}

void bleScannerClearTargets()
{
    portENTER_CRITICAL(&targetMutex);

    memset(
        targets,
        0,
        sizeof(targets)
    );

    portEXIT_CRITICAL(&targetMutex);

    Serial.println(
        "Radar targets cleared after filter change."
    );
}