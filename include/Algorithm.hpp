#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <vector>
#include <map>
#include <string>
#include <chrono>
#include <limits>
#include "BookTicker.hpp"

/**
 * @brief Compute moving average of midprice over a time window.
 * @param ticks Vector of BookTicker.
 * @param window_seconds Time window.
 * @return Moving average value.
 */
inline double moving_average(const std::vector<BookTicker>& ticks, double window_seconds) {
    if (ticks.empty()) {
        return 0.0;
    }

    using namespace std::chrono;
    double now = duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();

    double sum = 0.0;
    int count = 0;
    for (const auto& tick : ticks) {
        if ((now - tick.timestamp) <= window_seconds) {
            sum += (tick.bidPrice + tick.askPrice) / 2.0;
            ++ count;
        }
    }

    double last_price = (ticks.back().bidPrice + ticks.back().askPrice) / 2.0;
    return (count > 0) ? (sum / count) : last_price;
}

/**
 * @brief Detect arbitrage opportunity across exchanges.
 * @param exch_data_map_ Map of exchange to BookTicker vector.
 * @return True if arbitrage exists, otherwise false.
 */
inline bool find_arb(const std::map<std::string, std::vector<BookTicker>>& exch_data_map_) {
    double globalHighestBid = std::numeric_limits<double>::lowest();
    double globalLowestAsk = std::numeric_limits<double>::max();

    for (const auto& entry : exch_data_map_) {
        if (!entry.second.empty()) {
            double bid = entry.second.back().bidPrice;
            double ask = entry.second.back().askPrice;

            if (bid > globalHighestBid)
            {
                globalHighestBid = bid;
            }
            if (ask < globalLowestAsk)
            {
                globalLowestAsk = ask;
            }
        }
    }
    return globalLowestAsk < globalHighestBid;
}

#endif // ALGORITHM_HPP
