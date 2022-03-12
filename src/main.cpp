#include <iostream>
#include <csignal>
#include <thread>
#include "include/msd/channel.hpp"
#include "icmplib/icmplib.h"
#include "yaml-cpp/yaml.h"
#include "Logger/Logger.h"
#include "PingTarget.h"


using p_chan = msd::channel<icmplib::PingResult>;

bool stop = false;
std::vector<PingTarget> targets;

void sigint_handler(int signum){
    std::cout << "Caught stop signal" << std::endl;
    stop=true;
}

void continuous_ping(PingTarget& target){
    std::string address = target.getAddress();
    int interval = target.getInterval();
    icmplib::IPAddress t = icmplib::IPAddress(address);
    std::cout << "Starting to ping target " << address << " with interval " << interval << std::endl;
    while (!stop){
        icmplib::PingResult result = icmplib::Ping(t, interval);
        target.pushResult(result);
        sleep(interval);
    }
    target.stopThread();
}

void parseTargets(YAML::Node targetsNode){
    Logger* logger = Logger::getInstance();
    for(YAML::const_iterator it=targetsNode.begin();it!=targetsNode.end();++it){
        std::string address = it->operator[]("address").as<std::string>();
        int interval = it->operator[]("interval").as<int>();
        int threshold = it->operator[]("pingwarning_threshold").as<int>();
        targets.emplace_back(address,interval,threshold);
        logger->log(LogLevel::logINFO,"Parsed target: " + address, EventType::SetupInformation);
    }
}

int main() {
    YAML::Node config = YAML::LoadFile("config.yml");

    int pingInterval = config["interval"].as<int>();
    std::string target = config["target"].as<std::string>();
    std::string logfileName = config["logfile_name"].as<std::string>();
    int pingWarningThreshold = config["pingwarning_threshold"].as<int>();

    YAML::Node yamlTargets = config["targets"];
    parseTargets(yamlTargets);

    Logger::setFileTarget(logfileName);
    Logger* logger = Logger::getInstance();

    logger->log(LogLevel::logINFO, "Started logging");

    std::vector<std::thread> threads;
    signal(SIGINT, sigint_handler);
    p_chan pingResults;
    for(PingTarget& t:targets){
        threads.emplace_back(&continuous_ping, std::ref(t));
    }
//    threads.emplace_back(&continuous_ping, std::ref(pingResults), std::ref(target), std::ref(pingInterval));
//    threads.emplace_back(std::thread(&event_handling, std::ref(pingResults), std::ref(pingWarningThreshold)));

    for(std::thread& t: threads){
        t.join();
    }
    return 0;
}
