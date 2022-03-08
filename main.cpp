#include <iostream>
#include <csignal>
#include <thread>
#include "icmplib/icmplib.h"
#include "msd/channel.hpp"


using p_chan = msd::channel<icmplib::PingResult>;
bool stop = false;

void sigint_handler(int signum){
    std::cout << "Caught signal " << signum << std::endl;
    stop=true;
}

void event_handling(p_chan& in){
    while(!stop){
        icmplib::PingResult res;
        res << in;
        std::cout << "Ping: " << res.interval << std::endl;
    }
}

void continuous_ping(p_chan& out){
    while (!stop){
        icmplib::IPAddress target = icmplib::IPAddress("192.168.1.1");
        icmplib::PingResult result = icmplib::Ping(target);
        result >> out;
        sleep(1);
    }
}

int main() {
    signal(SIGINT, sigint_handler);
    p_chan pingresults;

    std::thread threads[] = {
            std::thread(&continuous_ping,std::ref(pingresults)),
            std::thread(&event_handling, std::ref(pingresults))
    };

    for(std::thread& t: threads){
        t.join();
    }
    return 0;
}
