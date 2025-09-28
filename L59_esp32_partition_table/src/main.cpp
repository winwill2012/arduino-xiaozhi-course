#include "Arduino.h"
#include "SPIFFS.h"

void setup()
{
    Serial.begin(9600);
    delay(3000);

    SPIFFS.begin(true);

    Serial.printf("SPIFFSS total space size: %d\n", SPIFFS.totalBytes());
    Serial.printf("SPIFFSS has used high size: %d\n", SPIFFS.usedBytes());
}

void loop()
{
}
