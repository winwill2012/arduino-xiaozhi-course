#include <Arduino.h>

const char* TAG = "ASR";

void setup()
{
    // Serial.begin(9600);
    // UART0 115200
}

void loop()
{
    Serial.println("Hello World");

    ESP_LOGV(TAG, "This is a VERBOSE log: %d", millis());
    ESP_LOGD(TAG, "This is a DEBUG log: %d", millis());
    ESP_LOGI(TAG, "This is an INFO log: %d", millis());
    ESP_LOGW(TAG, "This is a WARN log: %d", millis());
    ESP_LOGE(TAG, "This is an ERROR log: %d", millis());
}
