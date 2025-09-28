#include "DoubaoTTS.h"

#include <Arduino.h>
#include <WebSocketsClient.h>
#include "driver/i2s.h"
#include "ArduinoJson.h"
#include "Utils.h"

#define MAX98357_I2S_NUM  I2S_NUM_0
#define SAMPLE_RATE       16000
#define MAX98357_DOUT     38
#define MAX98357_LRC      40
#define MAX98357_BCLK     39

void DoubaoTTS::begin() {
    constexpr i2s_config_t max98357_i2s_config = {
            .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX),
            .sample_rate = SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // interrupt_priority，if_the_realtime_requirements_are_high，priority_can_be_raised
            .dma_buf_count = 4,
            .dma_buf_len = 1024,
            .tx_desc_auto_clear = true
    };

    constexpr i2s_pin_config_t max98357_gpio_config = {
            .bck_io_num = MAX98357_BCLK,
            .ws_io_num = MAX98357_LRC,
            .data_out_num = MAX98357_DOUT,
            .data_in_num = -1
    };

    i2s_driver_install(MAX98357_I2S_NUM, &max98357_i2s_config, 0, nullptr);
    i2s_set_pin(MAX98357_I2S_NUM, &max98357_gpio_config);

    // TODO: change_the_key_here_to_your_own
    setExtraHeaders("Authorization: Bearer; 4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW");
    beginSSL("openspeech.bytedance.com", 443, "/api/v1/tts/ws_binary");

    // use_c_as_follows++lambda_expression_syntax
    onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
        this->eventCallback(type, payload, length);
    });

    xTaskCreate([](void *arg) {
        const auto self = static_cast<DoubaoTTS *>(arg);
        self->playAudio(nullptr);
    }, "playAudio", 4096, this, 1, nullptr);
}

// used_to_parse_voice_synthesis_data_packets_sent_in_the_cloud
void DoubaoTTS::parseResponse(const uint8_t *response) const {
    const uint8_t messageType = response[1] >> 4;
    const uint8_t messageTypeSpecificFlags = response[1] & 0x0f;
    const uint8_t *payload = response + 4;

    switch (messageType) {
        case 0b1011: {
            // 0b1011 - Audio-only server response (ACK).
            if (messageTypeSpecificFlags > 0) {
                const auto sequenceNumber = readInt32(payload);
                const auto payloadSize = readInt32(payload + 4);
                if (payloadSize > 0) {
                    payload += 8;
                    PlayAudioTask task{};
                    task.length = payloadSize / sizeof(int16_t);
                    task.data = static_cast<int16_t *>(ps_malloc(payloadSize));
                    memcpy(task.data, payload, payloadSize);
                    if (xQueueSend(playAudioQueue, &task, portMAX_DELAY) != pdPASS) {
                        ESP_LOGE(TAG, "Failed to send audio playback task to queue: %d", task.length);
                        free(task.data); // send_to_queue_failed，the_producer_is_responsible_for_retrieving_the_memory
                    }
                }
                if (sequenceNumber < 0) {
                    ESP_LOGV(TAG, "Voice synthesis task ends");
                    xSemaphoreGive(taskFinished);
                }
            }
            break;
        }
        case 0b1111: {
            // Error message from server (for_example_the_wrong_message_type，unsupported_serialization_methods_etc)
            const uint8_t errorCode = readInt32(payload);
            const uint8_t messageSize = readInt32(payload + 4);
            const unsigned char *errMessage = payload + 8;
            ESP_LOGD(TAG, "Speech synthesis failed, code: %d, reason: %s", errorCode, String(errMessage, messageSize).c_str());
            xSemaphoreGive(taskFinished);
            break;
        }
        default:
            break;
    }
}

void DoubaoTTS::eventCallback(const WStype_t type, uint8_t *payload, const size_t length) const {
    switch (type) {
        case WStype_PING:
        case WStype_ERROR:
        case WStype_CONNECTED:
        case WStype_DISCONNECTED:
        case WStype_TEXT:
            break;
        case WStype_BIN:
            parseResponse(payload);
            break;
        default:
            break;
    }
}

String DoubaoTTS::buildFullClientRequest(const String &text) {
    JsonDocument params;
    const JsonObject app = params["app"].to<JsonObject>();
    // TODO: the_following_three_parameters，modify_it_into_your_own
    app["appid"] = "xxx";
    app["token"] = "xxx";
    app["cluster"] = "volcano_tts";

    const JsonObject user = params["user"].to<JsonObject>();
    user["uid"] = getChipId(nullptr);

    const JsonObject audio = params["audio"].to<JsonObject>();
    audio["voice_type"] = "zh_female_wanwanxiaohe_moon_bigtts";
    audio["encoding"] = "pcm";
    audio["rate"] = 16000;
    audio["speed_ratio"] = 1.0;
    audio["loudness_ratio"] = 2;

    const JsonObject request = params["request"].to<JsonObject>();
    request["reqid"] = generateTaskId();
    request["text"] = text;
    request["operation"] = "submit";
    String resStr;
    serializeJson(params, resStr);
    return resStr;
}

void DoubaoTTS::tts(const String &text, bool lastPacket) {
    ESP_LOGD(TAG, "Start speech synthesis: %s", text.c_str());
    // wait_for_the_websocket_to_establish_a_connection
    while (!isConnected()) {
        connect();
        vTaskDelay(1);
    }
    // send_voice_synthesis_packets
    const String payloadStr = buildFullClientRequest(text);
    uint8_t payload[payloadStr.length()];
    for (int i = 0; i < payloadStr.length(); i++) {
        payload[i] = static_cast<uint8_t>(payloadStr.charAt(i));
    }
    payload[payloadStr.length()] = '\0';

    // get_packet_length，convert_to_a_4byte_array
    const uint32_t payloadSize = payloadStr.length();
    std::vector<uint8_t> payloadLength = uint32ToUint8Array(payloadSize);

    // write_the_fourbyte_header_first，please_refer_to_the_official_documentation: https://www.volcengine.com/docs/6561/1257584
    std::vector<uint8_t> clientRequest(defaultHeader, defaultHeader + sizeof(defaultHeader));
    // write_another_4_byte_packet_length
    clientRequest.insert(clientRequest.end(), payloadLength.begin(), payloadLength.end());
    // write_to_the_packet
    clientRequest.insert(clientRequest.end(), payload, payload + sizeof(payload));

    if (!sendBIN(clientRequest.data(), clientRequest.size())) {
        ESP_LOGE(TAG, "Failed to send voice synthesis request packet: %s", text.c_str());
        xSemaphoreGive(taskFinished);
        return;
    }
    // keep_waiting_for_the_voice_synthesis_task_to_end
    while (xSemaphoreTake(taskFinished, pdMS_TO_TICKS(1)) == pdFALSE) {
        loop(); // continuously_call_the_loop_function_to_receive_data_packets_sent_by_the_cloud，until_the_last_package_task_is_received
        vTaskDelay(1);
    }
    if (lastPacket) {
        disconnect();
    }
}

// used_to_consume_audio_playback_task_queue，extract_audio_data_from_queue，play_through_i2s
void DoubaoTTS::playAudio(void *ptr) const {
    PlayAudioTask task{};
    size_t bytesWritten;
    while (true) {
        // continuously_remove_playback_tasks_from_the_queue
        if (xQueueReceive(playAudioQueue, &task, portMAX_DELAY) == pdPASS) {
            // write_to_i2s_to_complete_playback
            const esp_err_t result = i2s_write(MAX98357_I2S_NUM,
                                               task.data,
                                               task.length * sizeof(int16_t),
                                               &bytesWritten,
                                               portMAX_DELAY);
            // remember_to_release_the_memory_after_playing（the_memory_is_applied_by_the_producer，consumers_need_to_release_after_processing）
            free(task.data);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Play audio failed, errorCode: %d", result);
            }
        }
        vTaskDelay(1);
    }
}

void DoubaoTTS::connect() {
    if (isConnected() || _isConnecting) return;
    _isConnecting = true;
    xTaskCreate([](void *arg) {
        auto *self = static_cast<DoubaoTTS *>(arg);
        while (!self->isConnected()) {
            self->loop();
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        self->_isConnecting = false;
        vTaskDelete(nullptr);
    }, "DoubaoTTSConnect", 4096, this, 1, nullptr);
}
