#ifndef CHARTSERVER_HPP
#define CHARTSERVER_HPP

#include "httplib.h"
#include "BookTicker.hpp"
#include <map>
#include <mutex>
#include <string>
#include <vector>

/**
 * @brief Simple HTTP server for streaming chart data.
 */
class ChartServer {
public:
    /**
     * @brief Construct ChartServer.
     * @param data_map Reference to exchange data map.
     * @param mtx Reference to mutex for data_map.
     * @param host Host address to bind.
     * @param port Port to listen.
     */
    ChartServer(
        std::map<std::string, std::vector<BookTicker>>& data_map,
        std::mutex& mtx,
        const std::string& host = "0.0.0.0",
        int port = 18080
    );

    /**
     * @brief Start the HTTP server.
     */
    void start();

private:
    httplib::Server svr_;
    std::map<std::string, std::vector<BookTicker>>& data_map_;
    std::mutex& mtx_;
    std::string host_;
    int port_;
};

#endif // CHARTSERVER_HPP
