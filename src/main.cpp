#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <utility>

#include "KrakenFeed.hpp"
#include "BinanceFeed.hpp"
#include "ServiceRunner.hpp"
#include "BookTicker.hpp"
#include "Utils.hpp"

#include "gnuplot-iostream.h"

// For now we use gnuplot-iostream to plot the data.
// Used ref example here: https://stahlke.org/dan/gnuplot-iostream/
static Gnuplot gp;

static std::vector<BookTicker> g_kraken_tickers;
static std::vector<BookTicker> g_binance_tickers;

// TODO: does C++ has some message passing mechanism? I hate mutexes.
static std::mutex g_data_mutex;
static std::mutex g_plot_mutex;

// Update the plot using all available ticker data.
void updatePlot() {
    std::lock_guard<std::mutex> lock(g_plot_mutex);

    // I had to use ChatGPT to generate all GNU Plot code like the following string,
    // because it's way out of our interest to explore how to do this.
    AI_GENERATED {
        gp << "set title 'Realtime Price Difference'\n";
        gp << "set xlabel 'Time (s)'\n";
        gp << "set ylabel 'Price'\n";
        gp << "set grid\n";
        gp << "set autoscale\n";
        gp << "set style line 1 lc rgb '#90EE90' lw 2\n";
        gp << "set style line 2 lc rgb '#F08080' lw 2\n";
        gp << "set style line 3 lc rgb '#006400' lw 2\n";
        gp << "set style line 4 lc rgb '#8B0000' lw 2\n";
    }

    std::vector<std::pair<double, double>> kraken_bid_data;
    std::vector<std::pair<double, double>> kraken_ask_data;
    std::vector<std::pair<double, double>> binance_bid_data;
    std::vector<std::pair<double, double>> binance_ask_data;

    {
        std::lock_guard<std::mutex> data_lock(g_data_mutex);
        for (const auto &kt : g_kraken_tickers) {
            kraken_bid_data.push_back({kt.timestamp, kt.bidPrice});
            kraken_ask_data.push_back({kt.timestamp, kt.askPrice});
        }
        for (const auto &bt : g_binance_tickers) {
            binance_bid_data.push_back({bt.timestamp, bt.bidPrice});
            binance_ask_data.push_back({bt.timestamp, bt.askPrice});
        }
    }

    AI_GENERATED {
        gp << "plot '-' with lines ls 1 title 'Kraken Bid', "
            "'-' with lines ls 2 title 'Kraken Ask', "
            "'-' with lines ls 3 title 'Binance Bid', "
            "'-' with lines ls 4 title 'Binance Ask'\n";
    }

    gp.send1d(kraken_bid_data);
    gp.send1d(kraken_ask_data);
    gp.send1d(binance_bid_data);
    gp.send1d(binance_ask_data);
}

void addKrakenTicker(const BookTicker &ticker) {
    std::lock_guard<std::mutex> lock(g_data_mutex);
    g_kraken_tickers.push_back(ticker);
}

void addBinanceTicker(const BookTicker &ticker) {
    std::lock_guard<std::mutex> lock(g_data_mutex);
    g_binance_tickers.push_back(ticker);
}

int main() {
    auto start_time = std::chrono::steady_clock::now();

    // Callback for Kraken: include the current timestamp, store it, and update the plot.
    auto kraken_feed_cb = [start_time](const BookTicker &t) {
        // TODO: move this into BookTicker constructor
        auto now = std::chrono::steady_clock::now();
        double t_sec = std::chrono::duration<double>(now - start_time).count();

        BookTicker ticker(t.symbol, t.bidPrice, t.bidQty, t.askPrice, t.askQty, t_sec);
        std::cout << "(Kraken) update received: ";
        ticker.print();

        addKrakenTicker(ticker);
        updatePlot();
    };

    // Callback for Binance: include the current timestamp, store it, and update the plot.
    auto binance_feed_cb = [start_time](const BookTicker &t) {
        // TODO: move this into BookTicker constructor
        auto now = std::chrono::steady_clock::now();
        double t_sec = std::chrono::duration<double>(now - start_time).count();

        BookTicker ticker(t.symbol, t.bidPrice, t.bidQty, t.askPrice, t.askQty, t_sec);
        std::cout << "(Binance) update received: ";
        ticker.print();

        addBinanceTicker(ticker);
        updatePlot();
    };

    KrakenFeed kraken("BTC", "USD");
    BinanceFeed binance("BTC", "USD");

    kraken.start(kraken_feed_cb);
    binance.start(binance_feed_cb);

    // SIGINT handler seems to be failing for some reason...
    ServiceRunner runner;
    runner.run();

    return 0;
}
