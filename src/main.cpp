#include <iostream>
#include <csignal>
#include <thread>
#include <yaml-cpp/yaml.h>
#include <include/msd/channel.hpp>
#include "icmplib/icmplib.h"
#include "Logger/Logger.h"
#include "DBInterface.h"

using p_chan = msd::channel<icmplib::PingResult>;
void event_handling(p_chan& in, int pingWarningThreshold);

const std::string ResponseStrings[] = {"Success","Unreachable","TimeExceeded","Timeout","Unsupported","Failure"};

std::string responseToString(icmplib::PingResponseType response){
    return ResponseStrings[static_cast<int>(response)];
}

bool stop = false; // flag to stop the pinger and event handler threads
// these are later overwritten by the value from the config file
bool useLogger = false;
bool useDatabase = false;
int pingerVerbosity = 1;
DBInterface* db;

void sigint_handler(int signum){
    std::cout << "Stopping..." << std::endl;
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
//        TODO: add event output to console for verbosity > 0
        if(res.response!=lastResponse){ // if change in network connection occurs
            if(lastResponse==icmplib::PingResponseType::Success){ // connection lost
                const std::string eD = "Lost connection";
                if(useLogger) logger->log(LogLevel::logINFO, eD, EventType::Timeout);
                if(useDatabase) db->insertEvent(eD, EventType::Timeout);
            }
            else if (res.response==icmplib::PingResponseType::Success){ // reconnected
                const std::string eD = "Reconnected";
                if(useLogger) logger->log(LogLevel::logINFO, "Reconnected", EventType::Timeout);
                if(useDatabase) db->insertEvent(eD, EventType::Timeout);
            }
        }

        // Ping events
        if(res.response==icmplib::PingResponseType::Success) {
            if (res.interval > pingWarningThreshold && !pingWarningDetected) { // ping went above the threshold
                const std::string eD = "Ping exceeded warning treshold";
                if(useLogger) logger->log(LogLevel::logINFO, eD, EventType::High_Ping);
                if(useDatabase) db->insertEvent(eD, EventType::High_Ping);
                pingWarningDetected = true;
            } else if (res.interval < pingWarningThreshold && pingWarningDetected) { // ping went back below the threshold
                const std::string eD = "Ping went back to normal";
                if(useLogger) logger->log(LogLevel::logINFO, eD, EventType::High_Ping);
                if(useDatabase) db->insertEvent(eD, EventType::High_Ping);
            }
        }
        lastResponse = res.response;
        if(pingerVerbosity==2) std::cout << "Ping: " << res.interval << " Response: " << responseToString(res.response) << std::endl;
    }
}

void continuous_ping(p_chan& out, const std::string& target,const int& interval = 5){
    icmplib::IPAddress t = icmplib::IPAddress(target);
    if(pingerVerbosity>0) std::cout << "Starting to ping target " << target << " with interval " << interval << std::endl;
    while (!stop){
        icmplib::PingResult result = icmplib::Ping(t, interval);
        result >> out;
        sleep(interval);
    }
    out.close();
}

std::string getConnectionString(YAML::Node configNode){
//    All option for connection string listed here https://www.postgresql.org/docs/9.5/libpq-connect.html#LIBPQ-PARAMKEYWORDS
    YAML::Node dbNode = configNode["database"];
    std::string connString;
    connString += "host=" + dbNode["host"].as<std::string>() + " port=" + dbNode["port"].as<std::string>()
            + " dbname=" + dbNode["dbname"].as<std::string>() + " user=" + dbNode["user"].as<std::string>()
            + " password=" + dbNode["password"].as<std::string>();
    return connString;
}

int main() {
    YAML::Node config = YAML::LoadFile("config.yml");

//    pinger config
    int pingInterval = config["pinger"]["interval"].as<int>();
    auto target = config["pinger"]["target"].as<std::string>();
    int pingWarningThreshold = config["pinger"]["pingwarning_threshold"].as<int>();
    pingerVerbosity = config["pinger"]["verbosity"].as<int>();

//    logger config and setup
    useLogger = config["logger"]["use_logger"].as<bool>();
    std::string logfileName;
    if(useLogger){
        logfileName = config["logger"]["logfile_name"].as<std::string>();
    }
    Logger::setFileTarget(logfileName);
    Logger* logger = Logger::getInstance();

//  db config and setup
    useDatabase = config["database"]["use_database"].as<bool>();
    if(useDatabase){
        std::string connectionString = getConnectionString(config);
        try{
            db = new DBInterface(connectionString);
            db->setupDB();
            db->insertTarget(target);
            db->setDefaultAddress(target);
        }
        catch(pqxx::broken_connection& e){
            std::cerr << "Invalid connection. Confirm your hostname, username and password are correct." << std::endl;
            delete db;
            return 1;
        }
    }

    if(useLogger) logger->log(LogLevel::logINFO, "Started logging");

    std::vector<std::thread> threads;
    // configure handler for ctrl+c
    signal(SIGINT, sigint_handler);

    // channel to send information between pinger and event handler
    p_chan pingResults;

    threads.emplace_back(&continuous_ping, std::ref(pingResults), std::ref(target), std::ref(pingInterval));
    threads.emplace_back(std::thread(&event_handling, std::ref(pingResults), std::ref(pingWarningThreshold)));

    for(std::thread& t: threads){
        t.join();
    }
    db->closeConnection();
    delete db;
    return 0;
}
