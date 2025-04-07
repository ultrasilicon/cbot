#ifndef BOOK_TICKER_HPP
#define BOOK_TICKER_HPP

#include <iostream>
#include <string>

class BookTicker {
public:
    std::string symbol;
    double bidPrice;
    double bidQty;
    double askPrice;
    double askQty;

    BookTicker(
        const std::string &symbol,
        double bidPrice,
        double bidQty,
        double askPrice,
        double askQty)
        : symbol(symbol),
          bidPrice(bidPrice),
          bidQty(bidQty),
          askPrice(askPrice),
          askQty(askQty)
    {
    }

    void print() const {
        std::cout << "Symbol: " << symbol
                  << " | Bid: " << bidPrice << " (" << bidQty << ")"
                  << " | Ask: " << askPrice << " (" << askQty << ")" << std::endl;
    }
};

#endif // BOOK_TICKER_HPP
