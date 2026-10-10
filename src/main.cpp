#include <Arduino.h>

#include "app_ui.h"
#include "ble_scanner.h"
#include "display.h"

void setup()
{
    Serial.begin(115200);

    initDisplay();
    showSplash();

    bleScannerInit();
    appUiInit();
}

void loop()
{
    bleScannerUpdate();
    appUiUpdate();

    delay(1);
}