#ifndef PING_TEST_LOGLEVELS_H
#define PING_TEST_LOGLEVELS_H
#include <string>

enum LogLevel {logERROR, logWARNING, logINFO};
static const std::string LogLevelStrings[] = {"Error", "Warning", "Info"};

static std::string logLevelToString(LogLevel level){
    return LogLevelStrings[level];
}

enum EventType {Timeout, High_Ping, NoEvent};
static const std::string EventStrings[] = {"Timeout", "High Ping", "NoEvent"};

static std::string eventToString(EventType event){
    return EventStrings[event];
}

#endif //PING_TEST_LOGLEVELS_H
