#ifndef PING_TEST_LOGGER_H
#define PING_TEST_LOGGER_H
#include "LogLevels.h"
#include <iostream>
#include <fstream>
#include "../Events.h"

class Logger {
public:
    static Logger* getInstance();
    static void setFileTarget(const std::string& filename);
    void log(const LogLevel& level, const std::string& text, const EventType& event = NoEvent);
private:
    Logger();
    static Logger* instance;
    static std::string filename;
    std::ofstream logfileStream;
};

#endif //PING_TEST_LOGGER_H
