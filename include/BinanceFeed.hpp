#ifndef BINANCE_FEED_HPP
#define BINANCE_FEED_HPP

#include "Feed.hpp"
#include "WebSocket.hpp"

/**
 * @class BinanceFeed
 * @brief Handles real-time market data feed from Binance via WebSocket.
 */
class BinanceFeed : public Feed {
public:
    BinanceFeed(const std::string &base, const std::string &quote);
    ~BinanceFeed();

    void start(BookTickerUpdateCallback callback) override;

    void on_connected(boost::system::error_code ec) override;
    void on_write(boost::system::error_code ec, std::size_t size) override;
    void on_read(boost::system::error_code ec, std::size_t size, const std::string &data) override;
};

#endif // BINANCE_FEED_HPP
