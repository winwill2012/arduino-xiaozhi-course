#ifndef DOUBAOTTS_H
#define DOUBAOTTS_H

#include "WebSocketsClient.h"

constexpr uint8_t defaultHeader[] = {0x11, 0x10, 0x10, 0x00};

// used_to_describe_an_audio_packet_returned_from_the_cloud
struct PlayAudioTask {
    size_t length;
    int16_t *data;
};

class DoubaoTTS : public WebSocketsClient {
public:
    void begin();

    void connect();

    String buildFullClientRequest(const String &text);

    void parseResponse(const uint8_t *response) const;

    void eventCallback(WStype_t type, uint8_t *payload, size_t length) const;

    void tts(const String &text, bool lastPacket);

    void playAudio(void *ptr) const;

private:
    const char *TAG = "DoubaoTTS";
    // queue_to_save_audio_playback_tasks
    QueueHandle_t playAudioQueue = xQueueCreate(10, sizeof(PlayAudioTask));;

    // binary_semaphore_used_to_indicate_whether_the_speech_synthesis_task_has_ended，you_can_also_use_eventgroup_to_implement_it
    SemaphoreHandle_t taskFinished = xSemaphoreCreateBinary();

    volatile bool _isConnecting = false;
};


#endif //DOUBAOTTS_H
