#ifndef CPPINGER_EVENTS_H
#define CPPINGER_EVENTS_H
#include <string>

enum EventType {Timeout, High_Ping, SetupInformation ,NoEvent};
const std::string EventStrings[] = {"Timeout", "High Ping","Setup Information", "NoEvent"};

// has to be static for some reason or the build fails
static std::string eventToString(EventType event){
    return EventStrings[event];
}

#endif //CPPINGER_EVENTS_H
