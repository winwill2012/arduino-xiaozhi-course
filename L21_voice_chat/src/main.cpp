#include "Arduino.h"
#include <WiFi.h>
#include "driver/i2s.h"
#include "DoubaoASR.h"
#include "DoubaoTTS.h"

const char *TAG = "main";

#define CHUNK_SIZE 800             // 50ms audio size
int16_t buffer[CHUNK_SIZE];
size_t bytesRead;

DoubaoTTS tts;
// TODO: the_first_parameter_here_is_modified_to_your_own_agent_botid
CozeAgent agent("7518434105567395881", &tts);
DoubaoASR asr(&agent);

void setup() {
    Serial.begin(9600);
    WiFiClass::mode(WIFI_MODE_STA);
    // TODO: change_the_wifi_name_and_password_here_to_your_own
    WiFi.begin("ChinaNet-GdPt", "19910226");
    ESP_LOGI(TAG, "Connecting to the Internet");
    while (WiFiClass::status() != WL_CONNECTED) {
        ESP_LOGI(TAG, ".");
        vTaskDelay(1000);
    }
    ESP_LOGI(TAG, "Successful Internet connection");
    // enable_tts_voice_synthesis_task
    tts.begin();
    // start_the_asr_voice_recognition_task
    asr.begin();
}

void loop() {
    if (Serial.available() > 0) {
        Serial.readStringUntil('\n');
        ESP_LOGI(TAG, "Start recording, please_speak, duration_5s...");
        // recorded_100_times，record_50ms_of_audio_each_time
        for (int i = 0; i < 100; i++) {
            const esp_err_t err = i2s_read(MICROPHONE_I2S_NUM,
                                           buffer,
                                           CHUNK_SIZE * sizeof(int16_t), // var_50ms_audio_is_admitted_each_time
                                           &bytesRead,
                                           portMAX_DELAY);
            if (err == ESP_OK) {
                // start_creating_a_websocket_connection_when_recording_is_successful，speed_up_voice_recognition
                asr.connect();
                // recorded_audio，sent_directly_to_the_ring_buffer
                const BaseType_t result = xRingbufferSend(asr.getRingBuffer(), buffer, bytesRead, portMAX_DELAY);
                if (result != pdTRUE) {
                    ESP_LOGE(TAG, "Failed to send recording data to RingBuffer");
                }
            }
        }
        ESP_LOGI(TAG, "Recording ends");
    }
}
