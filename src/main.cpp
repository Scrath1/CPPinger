#include <iostream>
#include <csignal>
#include <thread>
#include "include/msd/channel.hpp"
#include "icmplib/icmplib.h"
#include "yaml-cpp/yaml.h"
#include "Logger/Logger.h"

using p_chan = msd::channel<icmplib::PingResult>;
bool stop = false;

static const std::string ResponseStrings[] = {"Success","Unreachable","TimeExceeded","Timeout","Unsupported","Failure"};

std::string responseToString(icmplib::PingResponseType response){
    return ResponseStrings[static_cast<int>(response)];
}

void sigint_handler(int signum){
    std::cout << "Caught stop signal" << std::endl;
    stop=true;
}

void event_handling(p_chan& in, int pingWarningThreshold){
    bool pingWarningDetected = false; // flag to determine the flank when the ping goes above/below the threshold
    icmplib::PingResponseType lastResponse = icmplib::PingResponseType::Success;

    Logger* logger = Logger::getInstance();
    while(!stop){
        icmplib::PingResult res;
        res << in;

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

void continuous_ping(p_chan& out, const std::string& target,const int& interval = 5){
    icmplib::IPAddress t = icmplib::IPAddress(target);
    std::cout << "Starting to ping target " << target << " with interval " << interval << std::endl;
    while (!stop){
        icmplib::PingResult result = icmplib::Ping(t, interval);
        result >> out;
        sleep(interval);
    }
    out.close();
}

int main() {
    YAML::Node config = YAML::LoadFile("config.yml");

    int pingInterval = config["interval"].as<int>();
    std::string target = config["target"].as<std::string>();
    std::string logfileName = config["logfile_name"].as<std::string>();
    int pingWarningThreshold = config["pingwarning_threshold"].as<int>();

    Logger::setFileTarget(logfileName);
    Logger* logger = Logger::getInstance();

    logger->log(LogLevel::logINFO, "Started logging");

    std::vector<std::thread> threads;
    signal(SIGINT, sigint_handler);
    p_chan pingResults;

    threads.emplace_back(&continuous_ping, std::ref(pingResults), std::ref(target), std::ref(pingInterval));
    threads.emplace_back(std::thread(&event_handling, std::ref(pingResults), std::ref(pingWarningThreshold)));

    for(std::thread& t: threads){
        t.join();
    }
    return 0;
}
