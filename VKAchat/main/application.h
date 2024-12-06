#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

enum ChatState {    

};

class Application{
public:
    void init();
    void setChatState(ChatState state);
    void AbortSpeaking();
    void ToggleChatState();
    EventGroupHandle_t event_group_;

    void MainLoop();

    void AudioEncodeTask();
    void PlayLocalFile(const char* data, size_t size);
};

#endif  //_APPLICATION_H_