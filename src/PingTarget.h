#ifndef CPPINGER_PINGTARGET_H
#define CPPINGER_PINGTARGET_H
#include <string>
#include <thread>
#include "Logger/Logger.h"
#include "Logger/LogLevels.h"
#include "include/msd/channel.hpp"
#include "icmplib/icmplib.h"

class PingTarget {
public:
    PingTarget(std::string address, int interval, int pingWarningThreshold);
    void stopThread();
    void pushResult(const icmplib::PingResult& result);
private:
    std::string address;
    void eventHandling();
    int interval;
    int pingWarningThreshold;
    std::thread eventHandler;
    msd::channel<icmplib::PingResult> chan;
    bool stop = false;
};


#endif //CPPINGER_PINGTARGET_H
