#ifndef CPPINGER_EVENTFUNCTIONS_H
#define CPPINGER_EVENTFUNCTIONS_H
#include "DBInterface/DBInterface.h"
#include "Logger/Logger.h"

// variables initialized and set in main.cpp
extern bool useLogger;
extern bool useDatabase;
extern int pingerVerbosity;

// used for timestamp output in CLI
auto t = std::time(nullptr);
auto tm = *std::localtime(&t);

static void eventDetected(const std::string& eventDescription, EventType eventType, LogLevel loglevel, Logger* logger, DBInterface* db){
    if(useLogger) logger->log(loglevel, eventDescription, eventType);
    if(useDatabase) db->insertEvent(eventDescription, eventType);
    if(pingerVerbosity>0) std::cout << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << " " << eventDescription << std::endl;
}

static void connectionLost(DBInterface* db, Logger* logger){
    const std::string eD = "Lost connection";
    eventDetected(eD, EventType::Timeout, LogLevel::logINFO, logger,db);
}

static void reconnected(DBInterface* db, Logger* logger){
    const std::string eD = "Reconnected";
    eventDetected(eD, EventType::Timeout, LogLevel::logINFO, logger,db);
}

static void pingAboveThreshold(DBInterface* db, Logger* logger){
    const std::string eD = "Ping exceeded warning treshold";
    eventDetected(eD, EventType::High_Ping, LogLevel::logINFO, logger,db);
}

static void pingBelowThreshold(DBInterface* db, Logger* logger){
    const std::string eD = "Ping went back to normal";
    eventDetected(eD, EventType::High_Ping, LogLevel::logINFO, logger,db);
}



#endif //CPPINGER_EVENTFUNCTIONS_H
