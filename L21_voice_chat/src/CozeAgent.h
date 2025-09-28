#ifndef COZEAGENT_H
#define COZEAGENT_H

#include <Arduino.h>
#include <DoubaoTTS.h>
#include <map>

// defines_the_separator_for_multiple_fields_returned_by_the_agent
#define DELIMITER "|"

enum LLMState {
    Init, // initialization_status
    CommandCompleted, // the_command_received_complete
    ParamsCompleted, // parameter_received_complete
    ResponseCompleted // reply_content_has_been_received
};

enum LLMEvent {
    NormalChar, // normal_characters_received
    Delimiter, // encounter_field_separator
};

class CozeAgent {
public:
    CozeAgent(String botId, DoubaoTTS *tts);

    void reset();

    void chat(const String &query);

    void stateTransfer(LLMState state, LLMEvent event);

    void processDelta(const String &delta);

    void appendField(const String &delta);

    String getConversationId();

    String getBotId();

private:
    const char *TAG = "CozeAgent";
    // status_transfer_routing
    std::map<std::pair<LLMState, LLMEvent>, LLMState> _stateTransferRouterMap;
    DoubaoTTS *_tts;
    String _command;
    String _params;
    String _response;
    String _ttsBuffer;
    LLMState _state = Init;
    String _conversationId = "";
    String _botId;
};


#endif //COZEAGENT_H
