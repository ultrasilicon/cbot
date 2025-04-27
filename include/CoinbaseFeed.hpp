#ifndef COINBASE_FEED_HPP
#define COINBASE_FEED_HPP

#include "Feed.hpp"
#include "WebSocket.hpp"

/**
 * @class CoinbaseFeed
 * @brief Handles real-time market data feed from Coinbase via WebSocket.
 */
class CoinbaseFeed : public Feed {
public:
    CoinbaseFeed(const std::string &base_asset, const std::string &quote_asset);
    ~CoinbaseFeed();

    void start(BookTickerUpdateCallback callback) override;

    void on_connected(boost::system::error_code ec) override;
    void on_write(boost::system::error_code ec, std::size_t size) override;
    void on_read(boost::system::error_code ec, std::size_t size, const std::string &data) override;
};

#endif // COINBASE_FEED_HPP
