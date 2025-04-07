#ifndef FEED_HPP
#define FEED_HPP

#include "BookTicker.hpp"
#include "WebSocket.hpp"

#include <functional>

/**
 * @class Feed
 * @brief Abstract base class for handling market data feeds.
 */
class Feed {
public:
    virtual ~Feed() {}

    /**
     * @brief Starts the feed and processes incoming market data.
     * @param callback A function to handle updates for each BookTicker event.
     */
    virtual void start(std::function<void(const BookTicker&)> callback) = 0;

    // virtual void on_connected(boost::system::error_code ec);
    // virtual void on_write(boost::system::error_code ec, std::size_t size);
    // virtual void on_read(boost::system::error_code ec, std::size_t size, const std::string &data);
};

#endif // FEED_HPP
