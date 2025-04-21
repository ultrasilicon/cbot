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

using json = nlohmann::json;
using namespace httplib;

class ChartServer {
public:
    ChartServer(std::map<std::string, std::vector<BookTicker>>& data_map,
                std::mutex& mtx,
                const std::string& host = "0.0.0.0",
                int port = 18080)
        : data_map_(data_map)
        , mtx_(mtx)
        , host_(host)
        , port_(port)
    {}

    void start() {
        std::string html;
        {
            std::ifstream ifs("index.html");
            if (!ifs) {
                std::cerr << "Failed to open index.html" << std::endl;
                return;
            }

            std::stringstream ss;
            ss << ifs.rdbuf();
            html = ss.str();
        }

        // serve HTML at /
        svr_.Get("/", [html](const Request&, Response& res) {
            res.set_content(html, "text/html");
        });

        // ws stream for ticker data
        svr_.Get("/ticker-events", [this](const Request&, Response& res) {
            res.set_header("Content-Type", "text/event-stream");
            res.set_header("Cache-Control", "no-cache");

            res.set_chunked_content_provider(
                "text/event-stream",
                [this](size_t _offset, DataSink &sink) {
                    while (true) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        json j;
                        {
                            std::lock_guard<std::mutex> lk(mtx_);
                            for (auto& kv : data_map_) {
                                const auto& exchange = kv.first;
                                const auto& vec  = kv.second;
                                if (vec.empty()) continue;

                                const auto& bt = vec.back();
                                j[exchange] = {
                                    {"timestamp", bt.timestamp},
                                    {"bid", bt.bidPrice},
                                    {"ask", bt.askPrice}
                                };
                            }
                        }
                        std::string msg = "data: " + j.dump() + "\n\n";
                        if (!sink.write(msg.c_str(), msg.size())) {
                            return false;
                        }
                    }
                    return true;
                }
            );
        });

        // start the server
        std::thread([this](){ svr_.listen(host_.c_str(), port_); }).detach();
    }

private:
    Server svr_;
    std::map<std::string, std::vector<BookTicker>>& data_map_;
    std::mutex& mtx_;
    std::string host_;
    int port_;
};

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
