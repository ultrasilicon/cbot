#ifndef BOOK_TICKER_HPP
#define BOOK_TICKER_HPP

#include <iostream>
#include <string>
#include <chrono>

/**
 * @class BookTicker
 * @brief Represents a snapshot of the order book ticker with bid and ask details.
 */
class BookTicker
{
public:
    std::string symbol;
    double bidPrice;
    double bidQty;
    double askPrice;
    double askQty;
    double timestamp;

    BookTicker(
        const std::string &symbol,
        double bidPrice,
        double bidQty,
        double askPrice,
        double askQty)
        : symbol(symbol)
        , bidPrice(bidPrice)
        , bidQty(bidQty)
        , askPrice(askPrice)
        , askQty(askQty)
        , timestamp(std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count())
    {}

    BookTicker(
        const std::string &symbol,
        double bidPrice,
        double bidQty,
        double askPrice,
        double askQty,
        double t)
        : symbol(symbol)
        , bidPrice(bidPrice)
        , bidQty(bidQty)
        , askPrice(askPrice)
        , askQty(askQty)
        , timestamp(t)
    {}

    friend std::ostream &operator<<(std::ostream &os, const BookTicker &ticker) {
        os << "Time: " << ticker.timestamp
           << " | Symbol: " << ticker.symbol
           << " | Bid: " << ticker.bidPrice << " (" << ticker.bidQty << ")"
           << " | Ask: " << ticker.askPrice << " (" << ticker.askQty << ")";
        return os;
    }
};

#endif // BOOK_TICKER_HPP
