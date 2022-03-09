#ifndef PING_TEST_LOGLEVELS_H
#define PING_TEST_LOGLEVELS_H
#include <string>

enum LogLevel {logERROR, logWARNING, logINFO};
const std::string LogLevelStrings[] = {"Error", "Warning", "Info"};

// has to be static for some reason or the build fails
static std::string logLevelToString(LogLevel level){
    return LogLevelStrings[level];
}

enum EventType {Timeout, High_Ping, SetupInformation ,NoEvent};
const std::string EventStrings[] = {"Timeout", "High Ping","Setup Information", "NoEvent"};

// has to be static for some reason or the build fails
static std::string eventToString(EventType event){
    return EventStrings[event];
}

#endif //PING_TEST_LOGLEVELS_H
