#include <iostream>
#include <csignal>
#include <thread>
#include <yaml-cpp/yaml.h>
#include <include/msd/channel.hpp>
#include "icmplib/icmplib.h"
#include "Logger/Logger.h"
#include "DBInterface/DBInterface.h"
#include "EventFunctions.h"

using p_chan = msd::channel<icmplib::PingResult>;
void eventHandling(p_chan& in, int pingWarningThreshold);

const std::string ResponseStrings[] = {"Success","Unreachable","TimeExceeded","Timeout","Unsupported","Failure"};

std::string responseToString(icmplib::PingResponseType response){
    return ResponseStrings[static_cast<int>(response)];
}

bool stop = false; // flag to stop the pinger and event handler threads
// these are later overwritten by the value from the config file
bool useLogger = false;
bool useDatabase = false;
/// Determines how much information is written to the command line.
/// 0 = Only control information
/// 1 = control information + detected events
/// 2 = control information + detected events + ping responses
int pingerVerbosity = 1;
DBInterface* db;

void sigintHandler(int signum){
    std::cout << "Stopping..." << std::endl;
    stop=true;
}

/// Continuously reads incoming pingResults from the in parameter and reacts to detected events.
/// To stop this function set the global stop variable to true
void eventHandling(p_chan& in, int pingWarningThreshold){
    bool pingWarningDetected = false; // flag to determine the flank when the ping goes above/below the threshold
    icmplib::PingResponseType lastResponse = icmplib::PingResponseType::Success;

    Logger* logger = Logger::getInstance();
    while(!stop){
        icmplib::PingResult res;
        res << in;

        // Connection events
        if(res.response!=lastResponse){ // if change in network connection occurs
            if(lastResponse==icmplib::PingResponseType::Success){ // connection lost
                connectionLost(db, logger);
            }
            else if (res.response==icmplib::PingResponseType::Success){ // reconnected
                reconnected(db, logger);
            }
        }

        // Ping events
        if(res.response==icmplib::PingResponseType::Success) {
            if (res.interval > pingWarningThreshold && !pingWarningDetected) { // ping went above the threshold
                pingAboveThreshold(db, logger);
                pingWarningDetected = true;
            } else if (res.interval < pingWarningThreshold && pingWarningDetected) { // ping went back below the threshold
                pingBelowThreshold(db, logger);
                pingWarningDetected = false;
            }
        }
        lastResponse = res.response;
        if(pingerVerbosity==2) std::cout << "Ping: " << res.interval << " Response: " << responseToString(res.response) << std::endl;
    }
}

/// Continuously pings the given target and sends the results to an eventHandler using the out parameter.
/// To stop this function set the global stop variable to true.
void continuousPing(p_chan& out, const std::string& target, const int& interval = 5){
    icmplib::IPAddress t = icmplib::IPAddress(target);
    if(pingerVerbosity>0) std::cout << "Starting to ping target " << target << " with interval " << interval << std::endl;
    while (!stop){
        icmplib::PingResult result = icmplib::Ping(t, interval);
        result >> out;
        sleep(interval);
    }
    out.close();
}

/// Concatenate the connection string required by the database based on the given settings
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
    signal(SIGINT, sigintHandler);

    // channel to send information between pinger and event handler
    p_chan pingResults;

    threads.emplace_back(&continuousPing, std::ref(pingResults), std::ref(target), std::ref(pingInterval));
    threads.emplace_back(std::thread(&eventHandling, std::ref(pingResults), std::ref(pingWarningThreshold)));

    for(std::thread& t: threads){
        t.join();
    }
    db->closeConnection();
    delete db;
    return 0;
}
