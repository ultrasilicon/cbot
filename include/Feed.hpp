#ifndef FEED_HPP
#define FEED_HPP

#include "BookTicker.hpp"
#include <functional>

class Feed {
public:
    virtual ~Feed() {}
    virtual void start(std::function<void(const BookTicker&)> callback) = 0;
};

#endif // FEED_HPP
