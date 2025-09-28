#include "CozeAgent.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <utility>

#include "utils.h"

CozeAgent::CozeAgent(String botId, DoubaoTTS *tts) {
    _botId = std::move(botId);
    _tts = tts;
    _stateTransferRouterMap = {
            {std::make_pair(Init, NormalChar),             Init},
            {std::make_pair(Init, Delimiter),              CommandCompleted},

            {std::make_pair(CommandCompleted, NormalChar), CommandCompleted},
            {std::make_pair(CommandCompleted, Delimiter),  ParamsCompleted},

            {std::make_pair(ParamsCompleted, NormalChar),  ParamsCompleted},
            {std::make_pair(ParamsCompleted, Delimiter),   ResponseCompleted}
    };
}

void CozeAgent::reset() {
    _command = "";
    _params = "";
    _response = "";
    _ttsBuffer = "";
    _state = Init;
}

String CozeAgent::getConversationId() {
    return _conversationId;
}

String CozeAgent::getBotId() {
    return _botId;
}

void CozeAgent::chat(const String &query) {
    reset();
    ESP_LOGI(TAG, "Start a conversation: %s", query.c_str());
    HTTPClient http;
    http.begin("https://api.coze.cn/v3/chat?conversation_id=" + getConversationId());
    // Here_the_token_behind_bearer_is_modified_to_your_own_coze_platform_token, available_at_https://www.coze.cn/open/oauth/pats are obtained here
    http.addHeader("Authorization", "Bearer pat_TWjCVBrZMdB0i3hieuhpWpyiF3hTqrouEGd6f3WAaaQvo0mU4dRjCQN9k3tk9WS4");
    http.addHeader("Content-Type", "application/json");
    JsonDocument requestBody;
    requestBody.clear();
    requestBody["stream"] = true;
    requestBody["bot_id"] = getBotId();
    requestBody["user_id"] = "123";
    const JsonArray additionalMessages = requestBody["additional_messages"].to<JsonArray>();
    const JsonObject message = additionalMessages.add<JsonObject>();
    message["content_type"] = "text";
    message["content"] = query;
    message["role"] = "user";
    String requestBodyStr;
    serializeJson(requestBody, requestBodyStr);
    const int httpResponseCode = http.POST(requestBodyStr.c_str());
    if (httpResponseCode > 0) {
        ESP_LOGI(TAG, "Response code: %d", httpResponseCode);
        // start_creating_speech_synthesis_connection_when_calling_coze_agent，speed_up_subsequent_synthesis
        _tts->connect();
        WiFiClient *stream = http.getStreamPtr();
        String line = "";
        String lastEvent;
        String output = "";
        // continuous_reading_of_streaming_output
        while (stream->connected() || stream->available()) {
            // wait_for_the_data_stream_to_have_new_data_to_read
            while (!stream->available()) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            line = stream->readStringUntil('\n');
            if (!line.isEmpty()) {
                // ESP_LOGD(TAG, "%s", line.c_str());
                if (line.startsWith("event:")) {
                    // Coze agent streaming call has returned full content
                    if (lastEvent == "event:conversation.message.delta" &&
                        line == "event:conversation.message.completed") {
                        http.end();
                        ESP_LOGI(TAG, "Coze agent call ends");
                        ESP_LOGI(TAG, "command: %s", _command.c_str());
                        ESP_LOGI(TAG, "params: %s", _params.c_str());
                        ESP_LOGI(TAG, "response: %s", _response.c_str());
                        // if_there_is_still_unsynthesised_audio_data，continue_to_synthesize_the_pronunciation
                        if (!_ttsBuffer.isEmpty()) {
                            _tts->tts(_ttsBuffer, true);
                        }
                        return;
                    }
                    lastEvent = line;
                }
                if (line.startsWith("data:")) {
                    String response = line.substring(5);
                    JsonDocument doc;
                    DeserializationError error = deserializeJson(doc, response);
                    if (error) {
                        ESP_LOGE(TAG, "json deserialization failed: %s", error.c_str());
                        continue;
                    }
                    if (doc["content"].is<String>() && doc["type"] == "answer") {
                        auto content = doc["content"].as<String>();
                        processDelta(content);
                        _conversationId = doc["conversation_id"].as<String>();
                    }
                }
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        ESP_LOGI(TAG, "Coze agent call ends");
        http.end();
    }
}

// execution_status_transition
void CozeAgent::stateTransfer(LLMState state, LLMEvent event) {
    const auto it = _stateTransferRouterMap.find(std::make_pair(state, event));
    if (it != _stateTransferRouterMap.end()) {
        _state = it->second;
    }
}

void CozeAgent::appendField(const String &delta) {
    // according_to_current_status，append_the_content_of_the_corresponding_field
    switch (_state) {
        case Init:
            _command += delta;
            break;
        case CommandCompleted:
            _params += delta;
            break;
        case ParamsCompleted: {
            _response += delta;
            _ttsBuffer += delta;
            const std::pair<int, size_t> delimiterIndex = findMinIndexOfDelimiter(_ttsBuffer);
            // if_there_is_a_semantic_separator
            if (delimiterIndex.first >= 0) {
                // intercept_the_contents_before_the_delimiter，perform_speech_synthesis
                _tts->tts(_ttsBuffer.substring(0, delimiterIndex.first), false);
                // updated_the_part_that_has_not_been_synthesized_yet
                _ttsBuffer = _ttsBuffer.substring(delimiterIndex.first + delimiterIndex.second);
            }
        }
            break;
        default:
            break;
    }
}

// process_incremental_shard_data
void CozeAgent::processDelta(const String &delta) {
    if (delta.isEmpty()) return;
    ESP_LOGV(TAG, "Processing agent incremental messages: %s", delta.c_str());
    // if_the_new_shard_does_not_contain_a_separator，no_state_transfer_required
    const int index = delta.indexOf(DELIMITER);
    if (index < 0) {
        // according_to_current_status，append_the_content_of_the_corresponding_field
        appendField(delta);
        return;
    }
    // intercept_the_part_on_the_left_of_the_delimiter
    const String leftPart = delta.substring(0, index);
    // the_remaining_part_on_the_right_side_of_the_separator（it_is_likely_to_contain_delimiters）
    const String remainingPart = delta.substring(index + 1);

    // Step1: add_the_left_half
    appendField(leftPart);
    // Step2: then_perform_state_transfer
    stateTransfer(_state, Delimiter);
    // Step3: recursively_process_the_rest
    processDelta(remainingPart);
}
