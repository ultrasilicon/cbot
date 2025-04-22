#ifndef FEED_HPP
#define FEED_HPP

#include "BookTicker.hpp"
#include "WebSocket.hpp"

#include <functional>
#include <thread>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

/**
 * @class Feed
 * @brief Abstract base_asset class for handling market data feeds.
 */
class Feed {
public:
    using BookTickerUpdateCallback = std::function<void(const BookTicker&)>;

    virtual ~Feed() {}

    /**
     * @brief Starts the feed and processes incoming market data.
     * @param callback A function to handle updates for each BookTicker event.
     */
    virtual void start(BookTickerUpdateCallback callback) = 0;

    virtual void on_connected(boost::system::error_code ec) = 0;
    virtual void on_write(boost::system::error_code ec, std::size_t size) = 0;
    virtual void on_read(boost::system::error_code ec, std::size_t size, const std::string &data) = 0;

protected:
    std::string base_asset_, quote_asset_;
    boost::asio::io_context ioc_;
    boost::asio::ssl::context ssl_ctx_;
    WebSocket ws_;
    std::thread io_thread_;
    BookTickerUpdateCallback callback_;

    /**
     * @brief Constructs a Feed object with the given base_asset and quote_asset symbols.
     * @param base_asset The base_asset currency or asset.
     * @param quote_asset The quote_asset currency or asset.
     * @param ssl_method The selected SSL method.
     */
    Feed(const std::string &base_asset, const std::string &quote_asset, boost::asio::ssl::context::method ssl_method)
        : base_asset_(base_asset),
          quote_asset_(quote_asset),
          ioc_(),
          ssl_ctx_(ssl_method),
          ws_(ioc_, ssl_ctx_)
    {
    }
};

#endif // FEED_HPP
