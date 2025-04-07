#include <iostream>
#include "KrakenFeed.hpp"
#include "BinanceFeed.hpp"
#include "ServiceRunner.hpp"
#include "BookTicker.hpp"

int main() {
    auto kraken_feed_cb = [](const BookTicker &ticker) {
        std::cout << "(Kraken) update received: ";
        ticker.print();
    };

    auto binance_feed_cb = [](const BookTicker &ticker) {
        std::cout << "(Binance) update received: ";
        ticker.print();
    };

    KrakenFeed kraken("BTC", "USDT");
    BinanceFeed binance("BTC", "USDT");

    kraken.start(kraken_feed_cb);
    binance.start(binance_feed_cb);

    ServiceRunner runner;
    runner.run();

    return 0;
}
