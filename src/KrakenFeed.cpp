#include "KrakenFeed.hpp"
#include "Utils.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace asio = boost::asio;

KrakenFeed::KrakenFeed(const std::string &base_asset, const std::string &quote_asset)
    : Feed(base_asset, quote_asset, boost::asio::ssl::context::tlsv12_client)
{
}

KrakenFeed::~KrakenFeed()
{
    ioc_.stop();
    if (io_thread_.joinable())
        io_thread_.join();
}

void KrakenFeed::start(BookTickerUpdateCallback callback)
{
    callback_ = callback;
    ws_.connect("ws.kraken.com", "443", "/v2",
                cbot::bind(&KrakenFeed::on_connected, this));
    io_thread_ = std::thread([this]() { ioc_.run(); });
}

void KrakenFeed::on_connected(boost::system::error_code ec)
{
    if (ec)
    {
        std::cerr << "Kraken connection error: " << ec.message() << "\n";
        return;
    }

    json j;
    j["method"] = "subscribe";
    j["params"] = {
        {"channel", "ticker"},
        {"symbol", {base_asset_ + "/" + quote_asset_}}
    };
    std::string msg = j.dump();

    ws_.write(msg, cbot::bind(&KrakenFeed::on_write, this));
}

void KrakenFeed::on_write(boost::system::error_code ec, std::size_t size)
{
    if (ec)
    {
        std::cerr << "Kraken write error: " << ec.message() << "\n";
        return;
    }
    ws_.read(cbot::bind(&KrakenFeed::on_read, this));
}

void KrakenFeed::on_read(boost::system::error_code ec, std::size_t size, const std::string &data)
{
    if (ec)
    {
        std::cerr << "Kraken read error: " << ec.message() << "\n";
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
                std::cerr << "Kraken ticker 'data' is not an array: " << j.dump() << "\n";
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Kraken JSON parse error: " << e.what() << "\n";
    }
    ws_.read(cbot::bind(&KrakenFeed::on_read, this));
}
