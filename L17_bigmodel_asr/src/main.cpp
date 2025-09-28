#include <Arduino.h>
#include "WebSocketsClient.h"
#include "ArduinoJson.h"
#include "utils.h"
#include "driver/i2s.h"
#include "freertos/ringbuf.h"

#define MICROPHONE_I2S_NUM             I2S_NUM_1
#define AUDIO_SAMPLE_RATE              16000
#define AUDIO_RECORDING_SAMPLE_NUMBER  160    // 10ms audio samples
#define MICROPHONE_I2S_BCLK            42
#define MICROPHONE_I2S_LRC             2
#define MICROPHONE_I2S_DOUT            1

#define CHUNK_SIZE 800             // 50ms audio size
#define TASK_COMPLETED_EVENT (1<<0)  // indicates_the_time_bit_at_which_a_speech_recognition_task_ends

const char* TAG = "ASR";

// default_header
constexpr byte DoubaoASRDefaultFullClientWsHeader[] = {0x11, 0x10, 0x10, 0x00};
constexpr byte DoubaoASRDefaultAudioOnlyWsHeader[] = {0x11, 0x20, 0x10, 0x00};
constexpr byte DoubaoASRDefaultLastAudioWsHeader[] = {0x11, 0x22, 0x10, 0x00};

int16_t buffer[CHUNK_SIZE];
RingbufHandle_t ringBuffer; // ring_buffer_used_to_temporarily_store_recordings
EventGroupHandle_t eventGroup;
size_t bytesRead;
std::vector<uint8_t> requestBuilder;

WebSocketsClient client;

void parseResponse(const uint8_t* response)
{
    const uint8_t messageType = response[1] >> 4;
    const uint8_t* payload = response + 4;
    ESP_LOGI(TAG, "message type: %d", messageType);
    switch (messageType)
    {
    case 0b1001:
        {
            // the_server_sends_the_recognition_results full server response
            const uint32_t payloadSize = readInt32(payload);
            payload += 4;
            std::string recognizeResult = readString(payload, payloadSize);
            JsonDocument jsonResult;
            const DeserializationError err = deserializeJson(jsonResult, recognizeResult);
            if (err)
            {
                ESP_LOGE(TAG, "parse speech recognize result failed: %s", err.c_str());
                return;
            }
            const String reqId = jsonResult["reqid"];
            const int32_t code = jsonResult["code"];
            const String message = jsonResult["message"];
            const int32_t sequence = jsonResult["sequence"];
            const JsonArray result = jsonResult["result"];
            ESP_LOGI(TAG, "sequence = %d, code = %d, message = %s, result size = %d", sequence, code, message.c_str(),
                     result.size());
            if (code == 1000 && result.size() > 0)
            {
                for (const auto& item : result)
                {
                    String text = item["text"];
                    ESP_LOGD(TAG, "text = %s", text.c_str());
                    // sequence less than 0, it_means_this_is_the_last_packet, you_can_print_all_the_contents_of_voice_recognition_directly
                    if (sequence < 0)
                    {
                        ESP_LOGI(TAG, "speech recognize result: %s", text.c_str());
                        // this_is_the_last_packet_returned_by_the_server，indicates_that_the_task_is_over，send_events_to_event_group，notify_another_task_to_be_finished
                        xEventGroupSetBits(eventGroup, TASK_COMPLETED_EVENT);
                    }
                }
            }
            break;
        }
    case 0b1111:
        {
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
    default:
        {
            break;
        }
    }
}

void eventCallback(WStype_t type, uint8_t* payload, size_t length)
{
    switch (type)
    {
    case WStype_PING:
    case WStype_ERROR:
        break;
    case WStype_CONNECTED:
        ESP_LOGI(TAG, "The websocket connection is successful");
        break;
    case WStype_DISCONNECTED:
        ESP_LOGI(TAG, "websocket disconnect");
        break;
    case WStype_TEXT:
        {
            break;
        }
    case WStype_BIN:
        parseResponse(payload);
        break;
    default:
        break;
    }
}

void buildFullClientRequest()
{
    JsonDocument doc;
    doc.clear();
    const JsonObject app = doc["app"].to<JsonObject>();
    app["appid"] = "4630330133";
    app["cluster"] = "volcengine_streaming_common";
    app["token"] = "4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW";

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
    for (int i = 0; i < payloadStr.length(); i++)
    {
        payload[i] = static_cast<uint8_t>(payloadStr.charAt(i));
    }
    payload[payloadStr.length()] = '\0';
    std::vector<uint8_t> payloadSize = uint32ToUint8Array(payloadStr.length());
    requestBuilder.clear();
    // write_to_the_header_first（four_bytes）
    requestBuilder.insert(requestBuilder.end(), DoubaoASRDefaultFullClientWsHeader,
                          DoubaoASRDefaultFullClientWsHeader + sizeof(DoubaoASRDefaultFullClientWsHeader));
    // write_payload_length（four_bytes）
    requestBuilder.insert(requestBuilder.end(), payloadSize.begin(), payloadSize.end());
    // write_payload_content
    requestBuilder.insert(requestBuilder.end(), payload, payload + payloadStr.length());
}

void buildAudioOnlyRequest(uint8_t* audio, const size_t size, const bool lastPacket)
{
    requestBuilder.clear();
    std::vector<uint8_t> payloadLength = uint32ToUint8Array(size);

    if (lastPacket)
    {
        // write_to_the_header_first（four_bytes）
        requestBuilder.insert(requestBuilder.end(), DoubaoASRDefaultLastAudioWsHeader,
                              DoubaoASRDefaultLastAudioWsHeader + sizeof(DoubaoASRDefaultLastAudioWsHeader));
    }
    else
    {
        // write_to_the_header_first（four_bytes）
        requestBuilder.insert(requestBuilder.end(), DoubaoASRDefaultAudioOnlyWsHeader,
                              DoubaoASRDefaultAudioOnlyWsHeader + sizeof(DoubaoASRDefaultAudioOnlyWsHeader));
    }

    // write_payload_length（four_bytes）
    requestBuilder.insert(requestBuilder.end(), payloadLength.begin(), payloadLength.end());
    // write_payload_content
    requestBuilder.insert(requestBuilder.end(), audio, audio + size);
}

void asr(uint8_t* buffer, const size_t size, const bool firstPacket, const bool lastPacket)
{
    ESP_LOGI(TAG, "Start speech recognition, audio_length: %d, fistPacket = %d, lastPacket = %d",
             size, firstPacket, lastPacket);
    if (firstPacket)
    {
        xEventGroupClearBits(eventGroup, TASK_COMPLETED_EVENT);
        while (!client.isConnected())
        {
            // if_the_websocket_is_not_connected，continuously_call_websocket_loop_function（there_will_be_a_connection_creation_logic_inside_the_function），keep_going_after_knowing_that_the_connection_is_successful
            client.loop();
            vTaskDelay(1);
        }
        // build_the_first_voice_recognition_request_related_message_header，you_can_refer_to_the_official_documentation：https://www.volcengine.com/docs/6561/80818
        buildFullClientRequest();
        // the_first_packet_is_sent_to_the_server，turn_on_the_recognition_task
        if (!client.sendBIN(requestBuilder.data(), requestBuilder.size()))
        {
            ESP_LOGD(TAG, "Send the first packet to send the voice recognition request failed");
        }
        // give_loop_an_opportunity_to_execute，receive_possible_serverside_data
        client.loop();
    }
    // build_voice_packets
    buildAudioOnlyRequest(buffer, size, lastPacket);
    if (!client.sendBIN(requestBuilder.data(), requestBuilder.size()))
    {
        ESP_LOGE(TAG, "Sending voice recognition audio packet failed...");
    }
    // continue_to_give_loop_function_a_chance_to_execute
    client.loop();
    if (lastPacket)
    {
        // if_the_last_voice_recognition_packet_has_been_sent_to_the_server，wait_for_the_end_of_the_task
        while ((xEventGroupWaitBits(eventGroup, TASK_COMPLETED_EVENT,
                                    false, true, pdMS_TO_TICKS(1)) & TASK_COMPLETED_EVENT) == 0)
        {
            // continuously_call_loop，receive_data_sent_by_the_server
            client.loop();
            vTaskDelay(1);
        }
        // task_completion，close_the_websocket_connection
        client.disconnect();
    }
}

void consumeRingBuffer(void* ptr)
{
    size_t bytesRead;
    bool firstPacket = true; // streaming_voice_recognition，use_this_to_indicate_that_this_is_the_first_voice_packet_recognized
    while (true)
    {
        void* buffer = xRingbufferReceive(ringBuffer, &bytesRead, pdMS_TO_TICKS(100));
        if (buffer != nullptr)
        {
            ESP_LOGI(TAG, "Read audio data from RingBuffer, length: %d", bytesRead);
            auto* audioData = static_cast<uint8_t*>(buffer);
            asr(audioData, bytesRead, firstPacket, false);
            if (firstPacket)
            {
                firstPacket = false;
            }
            vRingbufferReturnItem(ringBuffer, buffer);
        }
        else if (!firstPacket)
        {
            // simulate_the_last_empty_message，no_audio_data，the_main_function_is_to_let_the_server_end_a_round_of_identification_tasks，return_to_the_final_identified_content
            uint8_t fakeAudio[1] = {0};
            asr(fakeAudio, 1, firstPacket, true);
            firstPacket = true;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void setup()
{
    Serial.begin(9600);
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

    WiFiClass::mode(WIFI_MODE_STA);
    WiFi.begin("ChinaNet-GdPt", "19910226");
    ESP_LOGI(TAG, "Connecting to the Internet");
    while (WiFiClass::status() != WL_CONNECTED)
    {
        ESP_LOGI(TAG, ".");
        vTaskDelay(1000);
    }
    ESP_LOGI(TAG, "Successful Internet connection");

    // the_4yozbpbofizgvhwbqzrova3ftxqbewow_here_needs_to_be_replaced_with_your_own_access token
    client.setExtraHeaders("Authorization: Bearer; 4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW");
    client.beginSSL("openspeech.bytedance.com", 443, "/api/v2/asr");
    client.onEvent(eventCallback);

    ringBuffer = xRingbufferCreate(80000 * sizeof(int16_t), RINGBUF_TYPE_BYTEBUF);
    eventGroup = xEventGroupCreate();
    xTaskCreate(consumeRingBuffer, "consumeRingBuffer", 4096, nullptr, 1, nullptr);
}

void loop()
{
    if (Serial.available() > 0)
    {
        Serial.readStringUntil('\n');
        ESP_LOGI(TAG, "Start recording, please_speak, duration_5s...");
        // recorded_100_times，record_50ms_of_audio_each_time
        for (int i = 0; i < 100; i++)
        {
            const esp_err_t err = i2s_read(MICROPHONE_I2S_NUM,
                                           buffer,
                                           CHUNK_SIZE * sizeof(int16_t), // var_50ms_audio_is_admitted_each_time
                                           &bytesRead,
                                           portMAX_DELAY);
            if (err == ESP_OK)
            {
                // recorded_audio，sent_directly_to_the_ring_buffer
                BaseType_t result = xRingbufferSend(ringBuffer, buffer, bytesRead, portMAX_DELAY);
                if (result != pdTRUE)
                {
                    ESP_LOGE(TAG, "Failed to send recording data to RingBuffer");
                }
            }
        }
        ESP_LOGI(TAG, "Recording ends");
    }
}
