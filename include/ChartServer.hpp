#ifndef CHARTSERVER_HPP
#define CHARTSERVER_HPP

#include "httplib.h"
#include "BookTicker.hpp"
#include <map>
#include <mutex>
#include <string>
#include <vector>

class ChartServer {
public:
    ChartServer(
        std::map<std::string, std::vector<BookTicker>>& data_map,
        std::mutex& mtx,
        const std::string& host = "0.0.0.0",
        int port = 18080
    );

    void start();

private:
    httplib::Server svr_;
    std::map<std::string, std::vector<BookTicker>>& data_map_;
    std::mutex& mtx_;
    std::string host_;
    int port_;
};

#endif // CHARTSERVER_HPP
