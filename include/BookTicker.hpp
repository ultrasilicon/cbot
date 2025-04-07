#ifndef BOOK_TICKER_HPP
#define BOOK_TICKER_HPP

#include <iostream>
#include <string>

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
        , timestamp(0.0)
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

    // TODO: use ostream operator overload
    void print() const
    {
        std::cout << "Time: " << timestamp << " | Symbol: " << symbol
                  << " | Bid: " << bidPrice << " (" << bidQty << ")"
                  << " | Ask: " << askPrice << " (" << askQty << ")" << std::endl;
    }
};

#endif // BOOK_TICKER_HPP
