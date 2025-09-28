#include <Arduino.h>

void setup() {
    Serial.begin(9600);
    delay(5000);
    Serial.print("Total remaining memory size:");
    Serial.println(esp_get_free_heap_size());

    Serial.print("PSRAM remaining memory size:");
    Serial.println(heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    Serial.print("Total Flash size:");
    Serial.println(ESP.getFlashChipSize());
}

void loop() {
}
