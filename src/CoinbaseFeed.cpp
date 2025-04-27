#include "CoinbaseFeed.hpp"
#include "BookTicker.hpp"
#include "Utils.hpp"
#include "Logger.hpp"
#include <nlohmann/json.hpp>
#include <cctype>

using json = nlohmann::json;
namespace asio = boost::asio;

CoinbaseFeed::CoinbaseFeed(const std::string &base_asset, const std::string &quote_asset)
    : Feed(base_asset, quote_asset, boost::asio::ssl::context::tlsv12_client)
{
}

CoinbaseFeed::~CoinbaseFeed()
{
    LOG_INFO("Stopping Coinbase feed for " + base_asset_ + "/" + quote_asset_);
    ioc_.stop();
    if (io_thread_.joinable())
        io_thread_.join();
    LOG_INFO("Coinbase feed stopped");
}

void CoinbaseFeed::start(BookTickerUpdateCallback callback)
{
    LOG_INFO("Starting Coinbase feed for " + base_asset_ + "/" + quote_asset_);
    callback_ = callback;
    ws_.connect("ws-feed.exchange.coinbase.com", "443", "/", cbot::bind(&CoinbaseFeed::on_connected, this));
    io_thread_ = std::thread([this]() { ioc_.run(); });
}

void CoinbaseFeed::on_connected(boost::system::error_code ec)
{
    LOG_INFO("Connected to Coinbase WebSocket");
    if (ec)
    {
        LOG_ERROR("Coinbase connection error: " + ec.message());
        return;
    }

    std::string product_id = cbot::to_upper_copy(base_asset_) + "-" + cbot::to_upper_copy(quote_asset_);

    json j;
    j["type"] = "subscribe";
    j["channels"] = json::array();
    j["channels"].push_back({
        {"name", "ticker"},
        {"product_ids", { product_id }}
    });
    std::string msg = j.dump();

    LOG_INFO("Subscribing to Coinbase ticker channel: " + msg);
    ws_.write(msg, cbot::bind(&CoinbaseFeed::on_write, this));
}

void CoinbaseFeed::on_write(boost::system::error_code ec, std::size_t size)
{
    LOG_INFO("Subscribed to Coinbase ticker channel");
    if (ec)
    {
        LOG_ERROR("Coinbase write error: " + ec.message());
        return;
    }
    ws_.read(cbot::bind(&CoinbaseFeed::on_read, this));
}

void CoinbaseFeed::on_read(boost::system::error_code ec, std::size_t size, const std::string &data)
{
    if (ec)
    {
        LOG_ERROR("Coinbase read error: " + ec.message());
        return;
    }

    try
    {
        auto j = json::parse(data);
        if (j.contains("type") && j["type"] == "ticker" &&
            j.contains("product_id") && j.contains("best_bid") && j.contains("best_ask") &&
            j.contains("best_bid_size") && j.contains("best_ask_size"))
        {
            double bidPrice = std::stod(j["best_bid"].get<std::string>());
            double bidQty   = std::stod(j["best_bid_size"].get<std::string>());
            double askPrice = std::stod(j["best_ask"].get<std::string>());
            double askQty   = std::stod(j["best_ask_size"].get<std::string>());
            std::string symbol = j["product_id"].get<std::string>();
            BookTicker bt(symbol, bidPrice, bidQty, askPrice, askQty);
            if (callback_)
                callback_(bt);
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(std::string("Coinbase JSON parse error: ") + e.what());
    }
    ws_.read(cbot::bind(&CoinbaseFeed::on_read, this));
}
