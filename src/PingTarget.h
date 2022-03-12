#ifndef CPPINGER_PINGTARGET_H
#define CPPINGER_PINGTARGET_H
#include <string>
#include <thread>
#include "Logger/Logger.h"
#include "Logger/LogLevels.h"
#include "include/msd/channel.hpp"
#include "icmplib/icmplib.h"
#include <semaphore.h>

class PingTarget {
public:
    PingTarget(std::string address, int interval, int pingWarningThreshold);
    void stopThread();
    void pushResult(const icmplib::PingResult& result);
    std::string getAddress() const;
    int getInterval() const;
private:
    std::string address;
    void eventHandling();
    int interval;
    int pingWarningThreshold;
    std::thread eventHandler;
    std::queue<icmplib::PingResult> pings;
    // signal that a message is available
    sem_t queueSem;
    // lock the queue when pushing or popping
    sem_t pushLock;
    bool stop = false;
};


#endif //CPPINGER_PINGTARGET_H
