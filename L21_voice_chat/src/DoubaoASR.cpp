#include "DoubaoASR.h"
#include "WebSocketsClient.h"
#include "ArduinoJson.h"
#include "utils.h"
#include "driver/i2s.h"
#include "freertos/ringbuf.h"

DoubaoASR::DoubaoASR(CozeAgent *agent) {
    _cozeAgent = agent;
}

void DoubaoASR::begin() {
    // initialize_the_microphone_i2s_related_configuration
    constexpr i2s_config_t i2s_config = {
            .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = AUDIO_SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // the_left_and_right_channels_here_should_be_consistent_with_the_circuit
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 4,
            .dma_buf_len = 1024,
            .use_apll = false
    };
    constexpr i2s_pin_config_t pin_config = {
            .bck_io_num = MICROPHONE_I2S_BCLK,
            .ws_io_num = MICROPHONE_I2S_LRC,
            .data_out_num = -1,
            .data_in_num = MICROPHONE_I2S_DOUT
    };

    i2s_driver_install(MICROPHONE_I2S_NUM, &i2s_config, 0, nullptr);
    i2s_set_pin(MICROPHONE_I2S_NUM, &pin_config);
    i2s_zero_dma_buffer(MICROPHONE_I2S_NUM);

    // TODO: the_4yozbpbofizgvhwbqzrova3ftxqbewow_here_needs_to_be_replaced_with_your_own_access token
    setExtraHeaders("Authorization: Bearer; 4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW");
    beginSSL("openspeech.bytedance.com", 443, "/api/v2/asr");

    // Here_is_c++ lambda expression
    onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
        this->eventCallback(type, payload, length);
    });

    _ringBuffer = xRingbufferCreate(80000 * sizeof(int16_t), RINGBUF_TYPE_BYTEBUF);
    _eventGroup = xEventGroupCreate();

    xTaskCreate([](void *arg) {
        DoubaoASR *self = static_cast<DoubaoASR *>(arg);
        self->consumeRingBuffer(nullptr);
    }, "consumeRingBuffer", 8192, this, 1, nullptr);
}

void DoubaoASR::parseResponse(const uint8_t *response) {
    const uint8_t messageType = response[1] >> 4;
    const uint8_t *payload = response + 4;
    switch (messageType) {
        case 0b1001: {
            // the_server_sends_the_recognition_results full server response
            const uint32_t payloadSize = readInt32(payload);
            payload += 4;
            std::string recognizeResult = readString(payload, payloadSize);
            JsonDocument jsonResult;
            const DeserializationError err = deserializeJson(jsonResult, recognizeResult);
            if (err) {
                ESP_LOGE(TAG, "parse speech recognize result failed: %s", err.c_str());
                return;
            }
            const String reqId = jsonResult["reqid"];
            const int32_t code = jsonResult["code"];
            const String message = jsonResult["message"];
            const int32_t sequence = jsonResult["sequence"];
            const JsonArray result = jsonResult["result"];
            ESP_LOGV(TAG, "sequence = %d, code = %d, message = %s, result size = %d", sequence, code, message.c_str(),
                     result.size());
            if (code == 1000 && result.size() > 0) {
                for (const auto &item: result) {
                    String text = item["text"];
                    ESP_LOGV(TAG, "text = %s", text.c_str());
                    // sequence less than 0, it_means_this_is_the_last_packet, you_can_print_all_the_contents_of_voice_recognition_directly
                    if (sequence < 0) {
                        ESP_LOGI(TAG, "speech recognize result: %s", text.c_str());
                        // this_is_the_last_packet_returned_by_the_server，indicates_that_the_task_is_over，send_events_to_event_group，notify_another_task_to_be_finished
                        xEventGroupSetBits(_eventGroup, TASK_COMPLETED_EVENT);
                        _cozeAgent->chat(text);
                    }
                }
            }
            break;
        }
        case 0b1111: {
            // the_type_of_message_sent_when_the_server_handles_errors（if_invalid_message_format，unsupported_serialization_methods_etc）
            const uint32_t errorCode = readInt32(payload);
            payload += 4;
            const uint32_t messageLength = readInt32(payload);
            payload += 4;
            const std::string errorMessage = readString(payload, messageLength);
            ESP_LOGE(TAG, "speech recognize failed: ");
            ESP_LOGE(TAG, "   errorCode =  %u\n", errorCode);
            ESP_LOGE(TAG, "errorMessage =  %s\n", errorMessage.c_str());
        }
        default: {
            break;
        }
    }
}

void DoubaoASR::eventCallback(const WStype_t type, const uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_PING:
        case WStype_ERROR:
            break;
        case WStype_CONNECTED:
            ESP_LOGI(TAG, "The websocket connection is successful");
            break;
        case WStype_DISCONNECTED:
            ESP_LOGI(TAG, "websocket disconnect");
            break;
        case WStype_TEXT: {
            break;
        }
        case WStype_BIN:
            parseResponse(payload);
            break;
        default:
            break;
    }
}

void DoubaoASR::buildFullClientRequest() {
    JsonDocument doc;
    doc.clear();
    const JsonObject app = doc["app"].to<JsonObject>();
    // TODO: the_following_three_parameters，modify_it_into_your_own
    app["appid"] = "xxx";
    app["cluster"] = "volcengine_streaming_common";
    app["token"] = "xxxxx";

    const JsonObject user = doc["user"].to<JsonObject>();
    user["uid"] = getChipId(nullptr);

    const JsonObject request = doc["request"].to<JsonObject>();
    request["reqid"] = generateTaskId();
    request["nbest"] = 1;
    // request["result_type"] = "full";
    request["sequence"] = 1;
    request["workflow"] = "audio_in,resample,partition,vad,fe,decode,itn,nlu_ddc,nlu_punctuate";

    const JsonObject audio = doc["audio"].to<JsonObject>();
    audio["format"] = "raw";
    audio["codec"] = "raw";
    audio["channel"] = 1;
    audio["rate"] = AUDIO_SAMPLE_RATE;

    String payloadStr;
    serializeJson(doc, payloadStr);
    uint8_t payload[payloadStr.length() + 1];
    for (int i = 0; i < payloadStr.length(); i++) {
        payload[i] = static_cast<uint8_t>(payloadStr.charAt(i));
    }
    payload[payloadStr.length()] = '\0';
    std::vector<uint8_t> payloadSize = uint32ToUint8Array(payloadStr.length());
    _requestBuilder.clear();
    // write_to_the_header_first（four_bytes）
    _requestBuilder.insert(_requestBuilder.end(), DoubaoASRDefaultFullClientWsHeader,
                           DoubaoASRDefaultFullClientWsHeader + sizeof(DoubaoASRDefaultFullClientWsHeader));
    // write_payload_length（four_bytes）
    _requestBuilder.insert(_requestBuilder.end(), payloadSize.begin(), payloadSize.end());
    // write_payload_content
    _requestBuilder.insert(_requestBuilder.end(), payload, payload + payloadStr.length());
}

void DoubaoASR::buildAudioOnlyRequest(uint8_t *audio, const size_t size, const bool lastPacket) {
    _requestBuilder.clear();
    std::vector<uint8_t> payloadLength = uint32ToUint8Array(size);

    if (lastPacket) {
        // write_to_the_header_first（four_bytes）
        _requestBuilder.insert(_requestBuilder.end(), DoubaoASRDefaultLastAudioWsHeader,
                               DoubaoASRDefaultLastAudioWsHeader + sizeof(DoubaoASRDefaultLastAudioWsHeader));
    } else {
        // write_to_the_header_first（four_bytes）
        _requestBuilder.insert(_requestBuilder.end(), DoubaoASRDefaultAudioOnlyWsHeader,
                               DoubaoASRDefaultAudioOnlyWsHeader + sizeof(DoubaoASRDefaultAudioOnlyWsHeader));
    }

    // write_payload_length（four_bytes）
    _requestBuilder.insert(_requestBuilder.end(), payloadLength.begin(), payloadLength.end());
    // write_payload_content
    _requestBuilder.insert(_requestBuilder.end(), audio, audio + size);
}

void DoubaoASR::asr(uint8_t *buffer, const size_t size, const bool firstPacket, const bool lastPacket) {
    ESP_LOGV(TAG, "Start speech recognition, audio_length: %d, fistPacket = %d, lastPacket = %d",
             size, firstPacket, lastPacket);
    if (firstPacket) {
        xEventGroupClearBits(_eventGroup, TASK_COMPLETED_EVENT);
        while (!isConnected()) {
            connect();
            vTaskDelay(1);
        }
        // build_the_first_voice_recognition_request_related_message_header，you_can_refer_to_the_official_documentation：https://www.volcengine.com/docs/6561/80818
        buildFullClientRequest();
        // the_first_packet_is_sent_to_the_server，turn_on_the_recognition_task
        if (!sendBIN(_requestBuilder.data(), _requestBuilder.size())) {
            ESP_LOGD(TAG, "Send the first packet to send the voice recognition request failed");
        }
        // give_loop_an_opportunity_to_execute，receive_possible_serverside_data
        loop();
    }
    // build_voice_packets
    buildAudioOnlyRequest(buffer, size, lastPacket);
    if (!sendBIN(_requestBuilder.data(), _requestBuilder.size())) {
        ESP_LOGE(TAG, "Sending voice recognition audio packet failed...");
    }
    // continue_to_give_loop_function_a_chance_to_execute
    loop();
    if (lastPacket) {
        // if_the_last_voice_recognition_packet_has_been_sent_to_the_server，wait_for_the_end_of_the_task
        while ((xEventGroupWaitBits(_eventGroup, TASK_COMPLETED_EVENT,
                                    false, true, pdMS_TO_TICKS(1)) & TASK_COMPLETED_EVENT) == 0) {
            // continuously_call_loop，receive_data_sent_by_the_server
            loop();
            vTaskDelay(1);
        }
        // task_completion，close_the_websocket_connection
        disconnect();
    }
}

void DoubaoASR::consumeRingBuffer(void *ptr) {
    size_t bytesRead;
    bool firstPacket = true; // streaming_voice_recognition，use_this_to_indicate_that_this_is_the_first_voice_packet_recognized
    while (true) {
        void *buffer = xRingbufferReceive(_ringBuffer, &bytesRead, pdMS_TO_TICKS(100));
        if (buffer != nullptr) {
            ESP_LOGV(TAG, "Read audio data from RingBuffer, length: %d", bytesRead);
            auto *audioData = static_cast<uint8_t *>(buffer);
            asr(audioData, bytesRead, firstPacket, false);
            if (firstPacket) {
                firstPacket = false;
            }
            vRingbufferReturnItem(_ringBuffer, buffer);
        } else if (!firstPacket) {
            // simulate_the_last_empty_message，no_audio_data，the_main_function_is_to_let_the_server_end_a_round_of_identification_tasks，return_to_the_final_identified_content
            uint8_t fakeAudio[1] = {0};
            asr(fakeAudio, 1, firstPacket, true);
            firstPacket = true;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

RingbufHandle_t DoubaoASR::getRingBuffer() const {
    return _ringBuffer;
}

void DoubaoASR::connect() {
    if (isConnected() || _isConnecting) return;
    _isConnecting = true;
    xTaskCreate([](void *arg) {
        auto *self = static_cast<DoubaoASR *>(arg);
        while (!self->isConnected()) {
            self->loop();
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        self->_isConnecting = false;
        vTaskDelete(nullptr);
    }, "DoubaoASRConnect", 4096, this, 1, nullptr);
}