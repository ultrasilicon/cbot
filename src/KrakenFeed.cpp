#include "KrakenFeed.hpp"
#include "Utils.hpp"
#include "Logger.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace asio = boost::asio;

KrakenFeed::KrakenFeed(const std::string &base_asset, const std::string &quote_asset)
    : Feed(
        cbot::to_upper_copy(base_asset),
        cbot::to_upper_copy(quote_asset),
        boost::asio::ssl::context::tlsv12_client
    )
{
}

KrakenFeed::~KrakenFeed()
{
    LOG_INFO("Stopping Kraken feed for " + base_asset_ + "/" + quote_asset_);
    ioc_.stop();
    if (io_thread_.joinable())
        io_thread_.join();
    LOG_INFO("> Kraken feed stopped");
}

void KrakenFeed::start(BookTickerUpdateCallback callback)
{
    LOG_INFO("Starting Kraken feed for " + base_asset_ + "/" + quote_asset_);

    callback_ = callback;
    ws_.connect("ws.kraken.com", "443", "/v2",
                cbot::bind(&KrakenFeed::on_connected, this));
    io_thread_ = std::thread([this]() { ioc_.run(); });
}

void KrakenFeed::on_connected(boost::system::error_code ec)
{
    LOG_INFO("Connected to Kraken WebSocket");
    if (ec)
    {
        LOG_ERROR("Kraken connection error: " + ec.message());
        return;
    }

    json j;
    j["method"] = "subscribe";
    j["params"] = {
        {"channel", "ticker"},
        {"symbol", {base_asset_ + "/" + quote_asset_}}
    };
    std::string msg = j.dump();
    LOG_INFO("Subscribing to Kraken ticker channel: " + msg);

    ws_.write(msg, cbot::bind(&KrakenFeed::on_write, this));
}

void KrakenFeed::on_write(boost::system::error_code ec, std::size_t size)
{
    LOG_INFO("Subscribed to Kraken ticker channel");
    if (ec)
    {
        LOG_ERROR("Kraken write error: " + ec.message());
        return;
    }
    ws_.read(cbot::bind(&KrakenFeed::on_read, this));
}

void KrakenFeed::on_read(boost::system::error_code ec, std::size_t size, const std::string &data)
{
    if (ec)
    {
        LOG_ERROR("Kraken read error: " + ec.message());
        return;
    }
    try
    {
        auto j = json::parse(data);
        if (j.is_object() && j.contains("data"))
        {
            auto dataArray = j["data"];
            if (dataArray.is_array())
            {
                for (auto &ticker : dataArray)
                {
                    if (ticker.contains("bid") &&
                        ticker.contains("bid_qty") &&
                        ticker.contains("ask") &&
                        ticker.contains("ask_qty"))
                    {
                        double bidPrice = ticker["bid"].get<double>();
                        double bidQty = ticker["bid_qty"].get<double>();
                        double askPrice = ticker["ask"].get<double>();
                        double askQty = ticker["ask_qty"].get<double>();
                        std::string symbol =
                            ticker.contains("symbol") ? ticker["symbol"].get<std::string>() : (base_asset_ + "/" + quote_asset_);
                        BookTicker bt(symbol, bidPrice, bidQty, askPrice, askQty);
                        if (callback_)
                            callback_(bt);
                    }
                }
            }
            else
            {
                LOG_ERROR("Kraken ticker 'data' is not an array: " + j.dump());
            }
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(std::string("Kraken JSON parse error: ") + e.what());
    }
    ws_.read(cbot::bind(&KrakenFeed::on_read, this));
}
