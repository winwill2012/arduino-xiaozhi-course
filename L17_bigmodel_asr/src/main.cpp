#include <Arduino.h>
#include "WebSocketsClient.h"
#include "ArduinoJson.h"
#include "utils.h"
#include "driver/i2s.h"
#include "freertos/ringbuf.h"

#define MICROPHONE_I2S_NUM             I2S_NUM_1
#define AUDIO_SAMPLE_RATE              16000
#define AUDIO_RECORDING_SAMPLE_NUMBER  160    // 10ms音频样本数
#define MICROPHONE_I2S_BCLK            42
#define MICROPHONE_I2S_LRC             2
#define MICROPHONE_I2S_DOUT            1

#define CHUNK_SIZE 800             // 50ms音频大小
#define TASK_COMPLETED_EVENT (1<<0)  // 表示一次语音识别任务结束的时间位

const char* TAG = "ASR";

// 默认头部
constexpr byte DoubaoASRDefaultFullClientWsHeader[] = {0x11, 0x10, 0x10, 0x00};
constexpr byte DoubaoASRDefaultAudioOnlyWsHeader[] = {0x11, 0x20, 0x10, 0x00};
constexpr byte DoubaoASRDefaultLastAudioWsHeader[] = {0x11, 0x22, 0x10, 0x00};

int16_t buffer[CHUNK_SIZE];
RingbufHandle_t ringBuffer; // 用来暂存录音的环形缓冲区
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
            // 服务端下发包含识别结果的 full server response
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
                    // sequence小于0，表示这是最后一个数据包，直接可以打印语音识别全部内容
                    if (sequence < 0)
                    {
                        ESP_LOGI(TAG, "speech recognize result: %s", text.c_str());
                        // 这是服务器返回的最后一个数据包，表示任务结束，往事件组发送事件，通知另一个任务可以结束了
                        xEventGroupSetBits(eventGroup, TASK_COMPLETED_EVENT);
                    }
                }
            }
            break;
        }
    case 0b1111:
        {
            // 服务端处理错误时下发的消息类型（如无效的消息格式，不支持的序列化方法等）
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
        ESP_LOGI(TAG, "websocket连接成功");
        break;
    case WStype_DISCONNECTED:
        ESP_LOGI(TAG, "websocket断开连接");
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
    app["appid"] = "8988564775";
    app["cluster"] = "volcengine_streaming_common";
    app["token"] = "dsWgV1rCvxiinw_H2clmJuAI-O1D8P94";

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
    // 先写入报头（四字节）
    requestBuilder.insert(requestBuilder.end(), DoubaoASRDefaultFullClientWsHeader,
                          DoubaoASRDefaultFullClientWsHeader + sizeof(DoubaoASRDefaultFullClientWsHeader));
    // 写入payload长度（四字节）
    requestBuilder.insert(requestBuilder.end(), payloadSize.begin(), payloadSize.end());
    // 写入payload内容
    requestBuilder.insert(requestBuilder.end(), payload, payload + payloadStr.length());
}

void buildAudioOnlyRequest(uint8_t* audio, const size_t size, const bool lastPacket)
{
    requestBuilder.clear();
    std::vector<uint8_t> payloadLength = uint32ToUint8Array(size);

    if (lastPacket)
    {
        // 先写入报头（四字节）
        requestBuilder.insert(requestBuilder.end(), DoubaoASRDefaultLastAudioWsHeader,
                              DoubaoASRDefaultLastAudioWsHeader + sizeof(DoubaoASRDefaultLastAudioWsHeader));
    }
    else
    {
        // 先写入报头（四字节）
        requestBuilder.insert(requestBuilder.end(), DoubaoASRDefaultAudioOnlyWsHeader,
                              DoubaoASRDefaultAudioOnlyWsHeader + sizeof(DoubaoASRDefaultAudioOnlyWsHeader));
    }

    // 写入payload长度（四字节）
    requestBuilder.insert(requestBuilder.end(), payloadLength.begin(), payloadLength.end());
    // 写入payload内容
    requestBuilder.insert(requestBuilder.end(), audio, audio + size);
}

void asr(uint8_t* buffer, const size_t size, const bool firstPacket, const bool lastPacket)
{
    ESP_LOGI(TAG, "开始语音识别, 音频长度: %d, fistPacket = %d, lastPacket = %d",
             size, firstPacket, lastPacket);
    if (firstPacket)
    {
        xEventGroupClearBits(eventGroup, TASK_COMPLETED_EVENT);
        while (!client.isConnected())
        {
            // 如果websocket没有连接，持续调用websocket的loop函数（函数内部会有连接的创建逻辑），知道连接成功才继续往后
            client.loop();
            vTaskDelay(1);
        }
        // 构建第一个语音识别请求的相关报文头，可以参考官方文档：https://www.volcengine.com/docs/6561/80818
        buildFullClientRequest();
        // 第一个数据包发往服务器，开启识别任务
        if (!client.sendBIN(requestBuilder.data(), requestBuilder.size()))
        {
            ESP_LOGD(TAG, "发送语音识别请求第一个数据包失败");
        }
        // 给loop一个执行的机会，接收可能的服务器端下发的数据
        client.loop();
    }
    // 构建语音数据包
    buildAudioOnlyRequest(buffer, size, lastPacket);
    if (!client.sendBIN(requestBuilder.data(), requestBuilder.size()))
    {
        ESP_LOGE(TAG, "发送语音识别音频数据包失败...");
    }
    // 继续给loop函数执行的机会
    client.loop();
    if (lastPacket)
    {
        // 如果已经往服务器发送了最后一个语音识别数据包，则等待任务结束
        while ((xEventGroupWaitBits(eventGroup, TASK_COMPLETED_EVENT,
                                    false, true, pdMS_TO_TICKS(1)) & TASK_COMPLETED_EVENT) == 0)
        {
            // 持续调用loop，接收服务器下发的数据
            client.loop();
            vTaskDelay(1);
        }
        // 任务完成，关闭websocket连接
        client.disconnect();
    }
}

void consumeRingBuffer(void* ptr)
{
    size_t bytesRead;
    bool firstPacket = true; // 流式语音识别，用这个表示这是识别的第一个语音包
    while (true)
    {
        void* buffer = xRingbufferReceive(ringBuffer, &bytesRead, pdMS_TO_TICKS(100));
        if (buffer != nullptr)
        {
            ESP_LOGI(TAG, "从RingBuffer读到音频数据，长度: %d", bytesRead);
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
            // 模拟最后一个空报文，没有任何音频数据，主要作用是让服务端结束一轮识别任务，返回最终的识别内容
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
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // 这里的左右声道要和电路保持一致
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
    ESP_LOGI(TAG, "正在联网");
    while (WiFiClass::status() != WL_CONNECTED)
    {
        ESP_LOGI(TAG, ".");
        vTaskDelay(1000);
    }
    ESP_LOGI(TAG, "联网成功");

    // 这里的4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW需要换成你自己的access token
    client.setExtraHeaders("Authorization: Bearer; dsWgV1rCvxiinw_H2clmJuAI-O1D8P94");
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
        ESP_LOGI(TAG, "开始录音，请说话，持续时间5s...");
        // 录制100次，每次录制50ms的音频
        for (int i = 0; i < 100; i++)
        {
            const esp_err_t err = i2s_read(MICROPHONE_I2S_NUM,
                                           buffer,
                                           CHUNK_SIZE * sizeof(int16_t), // 每次录取50ms音频
                                           &bytesRead,
                                           portMAX_DELAY);
            if (err == ESP_OK)
            {
                // 录到的音频，直接发往环形缓冲区
                BaseType_t result = xRingbufferSend(ringBuffer, buffer, bytesRead, portMAX_DELAY);
                if (result != pdTRUE)
                {
                    ESP_LOGE(TAG, "将录音数据发送到RingBuffer失败");
                }
            }
        }
        ESP_LOGI(TAG, "录音结束");
    }
}
