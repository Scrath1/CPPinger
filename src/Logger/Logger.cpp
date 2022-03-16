#include "Logger.h"
#include <iomanip>
#include <ctime>

Logger* Logger::instance = nullptr;
std::string Logger::filename;

Logger::Logger() {
    const std::string folder = "logs/";
    if(filename.empty()){
        filename = "Log.log";
    }
    logfileStream.open(folder+filename, std::ofstream::out);
}

Logger* Logger::getInstance() {
    if(instance==nullptr){
        instance = new Logger();
    }
    return instance;
}

void Logger::log(const LogLevel &level, const std::string &text, const EventType &event) {
    std::string ev = event==EventType::NoEvent ? "" : eventToString(event);

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    logfileStream << std::left
        << std::setw(25) << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::setw(3) << '|'
        << std::setw(15) << logLevelToString(level) << std::setw(3) << '|'
        << std::setw(20) << ev << std::setw(3) << '|'
        << text << std::endl;
}

void Logger::setFileTarget(const std::string &fname) {
    filename = fname;
}
