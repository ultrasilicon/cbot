#ifndef KRAKEN_FEED_HPP
#define KRAKEN_FEED_HPP

#include "Feed.hpp"

/**
 * @class KrakenFeed
 * @brief Handles real-time market data feed from Kraken via WebSocket.
 */
class KrakenFeed : public Feed {
public:
    KrakenFeed(const std::string &base_asset, const std::string &quote_asset);
    ~KrakenFeed();

    void start(BookTickerUpdateCallback callback) override;

    void on_connected(boost::system::error_code ec) override;
    void on_write(boost::system::error_code ec, std::size_t size) override;
    void on_read(boost::system::error_code ec, std::size_t size, const std::string &data) override;
};

#endif // KRAKEN_FEED_HPP
