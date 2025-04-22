#include "ChartServer.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

using json = nlohmann::json;
using namespace httplib;

ChartServer::ChartServer(
    std::map<std::string, std::vector<BookTicker>>& data_map,
    std::mutex& mtx,
    const std::string& host,
    int port)
    : data_map_(data_map),
      mtx_(mtx),
      host_(host),
      port_(port)
{
}

void ChartServer::start() {
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

    // Serve HTML at root
    svr_.Get("/", [html](const Request&, Response& res) {
        res.set_content(html, "text/html");
    });

    // Endpoint for ticker data as server-sent events.
    svr_.Get("/ticker-events", [this](const Request&, Response& res) {
        res.set_header("Content-Type", "text/event-stream");
        res.set_header("Cache-Control", "no-cache");

        res.set_chunked_content_provider("text/event-stream",
            [this](size_t, DataSink &sink) {
                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    json j;
                    {
                        std::lock_guard<std::mutex> lk(mtx_);
                        for (auto& kv : data_map_) {
                            const auto& exchange = kv.first;
                            const auto& vec = kv.second;
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

    // Launch the server on a separate thread.
    std::thread([this]() {
        svr_.listen(host_.c_str(), port_);
    }).detach();
}
