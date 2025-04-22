#include "httplib.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>

#include "KrakenFeed.hpp"
#include "BinanceFeed.hpp"
#include "ServiceRunner.hpp"
#include "BookTicker.hpp"
#include "ChartServer.hpp"

using json = nlohmann::json;
using namespace httplib;

int main() {
    std::map<std::string, std::vector<BookTicker>> data_map;
    std::mutex data_mtx;

    auto kraken_cb = [&](const BookTicker& t) {
        std::lock_guard<std::mutex> lk(data_mtx);
        auto& vec = data_map["Kraken"];
        vec.push_back(t);
        // limit vec size
        if (vec.size() > 1000) vec.erase(vec.begin());
    };

    auto binance_cb = [&](const BookTicker& t) {
        std::lock_guard<std::mutex> lk(data_mtx);
        auto& vec = data_map["Binance"];
        vec.push_back(t);
        // limit vec size
        if (vec.size() > 1000) vec.erase(vec.begin());
    };

    KrakenFeed kraken("BTC", "USDT");
    BinanceFeed binance("BTC", "USDT");
    kraken.start(kraken_cb);
    binance.start(binance_cb);

    ChartServer server(data_map, data_mtx, "0.0.0.0", 18080);
    server.start();

    ServiceRunner runner;
    runner.run();
    return 0;
}
