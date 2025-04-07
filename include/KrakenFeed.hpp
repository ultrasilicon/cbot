#ifndef KRAKEN_FEED_HPP
#define KRAKEN_FEED_HPP

#include "Feed.hpp"
#include "WebSocket.hpp"
#include <thread>
#include <functional>
#include <string>

/**
 * @class KrakenFeed
 * @brief Handles real-time market data feed from Kraken via WebSocket.
 */
class KrakenFeed : public Feed {
public:
    KrakenFeed(const std::string &base, const std::string &quote);
    ~KrakenFeed();

    // TODO: std::function<void(const BookTicker &)> should be typedef'ed in base class
    void start(std::function<void(const BookTicker &)> callback) override;

private:
    std::string base_, quote_;
    boost::asio::io_context ioc_;
    boost::asio::ssl::context ssl_ctx_;
    WebSocket ws_;
    std::thread io_thread_;
    std::function<void(const BookTicker &)> callback_;

    // TODO: move these functions to base class
    void on_connected(boost::system::error_code ec);
    void on_write(boost::system::error_code ec, std::size_t size);
    void on_read(boost::system::error_code ec, std::size_t size, const std::string &data);
};

#endif // KRAKEN_FEED_HPP
