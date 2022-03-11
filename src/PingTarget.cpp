#include "PingTarget.h"

#include <utility>
#include "Logger/Logger.h"
#include "Logger/LogLevels.h"

const std::string ResponseStrings[] = {"Success","Unreachable","TimeExceeded","Timeout","Unsupported","Failure"};

std::string responseToString(icmplib::PingResponseType response){
    return ResponseStrings[static_cast<int>(response)];
}


PingTarget::PingTarget(std::string address, int interval, int pingWarningThreshold)
: address(std::move(address)), interval(interval), pingWarningThreshold(pingWarningThreshold), eventHandler(std::thread(&PingTarget::eventHandling, this))
{

}

void PingTarget::eventHandling() {
    bool pingWarningDetected = false; // flag to determine the flank when the ping goes above/below the threshold
    icmplib::PingResponseType lastResponse = icmplib::PingResponseType::Success;

    Logger* logger = Logger::getInstance();
    while(!stop){
        icmplib::PingResult res;
        res << chan;

        // Connection events
        if(res.response!=lastResponse){ // if change in network connection occurs
            if(lastResponse==icmplib::PingResponseType::Success){ // connection lost
                logger->log(LogLevel::logINFO, "Lost connection", EventType::Timeout);
            }
            else if (res.response==icmplib::PingResponseType::Success){ // reconnected
                logger->log(LogLevel::logINFO, "Reconnected", EventType::Timeout);
            }
        }

        // Ping events
        if(res.response==icmplib::PingResponseType::Success) {
            if (res.interval > pingWarningThreshold && !pingWarningDetected) { // ping went above the threshold
                logger->log(LogLevel::logINFO, "Ping exceeded warning threshold", EventType::High_Ping);
                pingWarningDetected = true;
            } else if (res.interval < pingWarningThreshold && pingWarningDetected) { // ping went back below the threshold
                logger->log(LogLevel::logINFO, "Ping back to normal", EventType::High_Ping);
            }
        }
        lastResponse = res.response;
        std::cout << "Ping: " << res.interval << " Response: "<< responseToString(res.response) <<std::endl;
    }
}

void PingTarget::stopThread() {
    stop=true;
}

void PingTarget::pushResult(const icmplib::PingResult& result) {
    result>>chan;
}
