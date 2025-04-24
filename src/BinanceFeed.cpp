#include "BinanceFeed.hpp"
#include "BookTicker.hpp"
#include "Utils.hpp"
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
    ioc_.stop();
    if (io_thread_.joinable())
        io_thread_.join();
}

void BinanceFeed::start(BookTickerUpdateCallback callback)
{
    callback_ = callback;
    ws_.connect("stream.binance.us", "9443", "/ws",
                cbot::bind(&BinanceFeed::on_connected, this));
    io_thread_ = std::thread([this]() { ioc_.run(); });
}

void BinanceFeed::on_connected(boost::system::error_code ec)
{
    if (ec)
    {
        std::cerr << "Binance connection error: " << ec.message() << "\n";
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

    ws_.write(msg, cbot::bind(&BinanceFeed::on_write, this));
}

void BinanceFeed::on_write(boost::system::error_code ec, std::size_t size)
{
    if (ec)
    {
        std::cerr << "Binance write error: " << ec.message() << "\n";
        return;
    }
    ws_.read(cbot::bind(&BinanceFeed::on_read, this));
}

void BinanceFeed::on_read(boost::system::error_code ec, std::size_t size, const std::string &data)
{
    if (ec)
    {
        std::cerr << "Binance read error: " << ec.message() << "\n";
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
        std::cerr << "Binance JSON parse error: " << e.what() << "\n";
    }
    ws_.read(cbot::bind(&BinanceFeed::on_read, this));
}
