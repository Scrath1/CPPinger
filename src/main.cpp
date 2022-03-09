#include <iostream>
#include <csignal>
#include <thread>
#include "include/msd/channel.hpp"
#include "icmplib/icmplib.h"
#include "yaml-cpp/yaml.h"
#include "Logger/Logger.h"

using p_chan = msd::channel<icmplib::PingResult>;
bool stop = false;

void sigint_handler(int signum){
    std::cout << "Caught stop signal" << std::endl;
    stop=true;
}

void event_handling(p_chan& in){
    while(!stop){
        icmplib::PingResult res;
        res << in;
        std::cout << "Ping: " << res.interval << std::endl;
    }
}

void continuous_ping(p_chan& out, const std::string& target,const int& interval = 5){
    icmplib::IPAddress t = icmplib::IPAddress(target);
    std::cout << "Starting to ping target " << target << " with interval " << interval << std::endl;
    while (!stop){
        icmplib::PingResult result = icmplib::Ping(t);
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

    Logger::setFileTarget(logfileName);
    Logger* logger = Logger::getInstance();

    logger->log(LogLevel::logINFO, "Started logging");

    std::vector<std::thread> threads;
    signal(SIGINT, sigint_handler);
    p_chan pingResults;

    threads.emplace_back(&continuous_ping, std::ref(pingResults), std::ref(target), std::ref(pingInterval));
    threads.emplace_back(std::thread(&event_handling, std::ref(pingResults)));

    for(std::thread& t: threads){
        t.join();
    }
    return 0;
}
