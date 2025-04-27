#include "BinanceFeed.hpp"
#include "BookTicker.hpp"
#include "Utils.hpp"
#include "Logger.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <cctype>

using json = nlohmann::json;
namespace asio = boost::asio;

BinanceFeed::BinanceFeed(const std::string &base_asset, const std::string &quote_asset)
    : Feed(base_asset, quote_asset, boost::asio::ssl::context::tlsv12_client)
{
}

BinanceFeed::~BinanceFeed()
{
    LOG_INFO("Stopping Binance feed for " + base_asset_ + "/" + quote_asset_);
    ioc_.stop();
    if (io_thread_.joinable())
        io_thread_.join();
    LOG_INFO("Binance feed stopped");
}

void BinanceFeed::start(BookTickerUpdateCallback callback)
{
    LOG_INFO("Starting Binance feed for " + base_asset_ + "/" + quote_asset_);
    callback_ = callback;
    ws_.connect("stream.binance.us", "9443", "/ws",
                cbot::bind(&BinanceFeed::on_connected, this));
    io_thread_ = std::thread([this]() { ioc_.run(); });
}

void BinanceFeed::on_connected(boost::system::error_code ec)
{
    LOG_INFO("Connected to Binance WebSocket");
    if (ec)
    {
        LOG_ERROR("Binance connection error: " + ec.message());
        return;
    }

    std::string symbol;
    for (auto c : base_asset_)
        symbol.push_back(static_cast<char>(std::tolower(c)));
    for (auto c : quote_asset_)
        symbol.push_back(static_cast<char>(std::tolower(c)));
    std::string subscription = symbol + "@bookTicker";

    json j;
    j["method"] = "SUBSCRIBE";
    j["params"] = {subscription};
    j["id"] = 1;
    std::string msg = j.dump();

    LOG_INFO("Subscribing to Binance ticker channel: " + msg);
    ws_.write(msg, cbot::bind(&BinanceFeed::on_write, this));
}

void BinanceFeed::on_write(boost::system::error_code ec, std::size_t size)
{
    LOG_INFO("Subscribed to Binance ticker channel");
    if (ec)
    {
        LOG_ERROR("Binance write error: " + ec.message());
        return;
    }
    ws_.read(cbot::bind(&BinanceFeed::on_read, this));
}

void BinanceFeed::on_read(boost::system::error_code ec, std::size_t size, const std::string &data)
{
    if (ec)
    {
        LOG_ERROR("Binance read error: " + ec.message());
        return;
    }

    try
    {
        auto j = json::parse(data);
        if (j.contains("s") &&
            j.contains("b") &&
            j.contains("a") &&
            j.contains("B") &&
            j.contains("A"))
        {
            double bidPrice = std::stod(j["b"].get<std::string>());
            double bidQty = std::stod(j["B"].get<std::string>());
            double askPrice = std::stod(j["a"].get<std::string>());
            double askQty = std::stod(j["A"].get<std::string>());
            BookTicker bt(j["s"].get<std::string>(), bidPrice, bidQty, askPrice, askQty);
            if (callback_)
                callback_(bt);
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(std::string("Binance JSON parse error: ") + e.what());
    }
    ws_.read(cbot::bind(&BinanceFeed::on_read, this));
}
