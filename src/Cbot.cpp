#include "Cbot.hpp"
#include "Algorithm.hpp"
#include "KrakenFeed.hpp"
#include "BinanceFeed.hpp"
#include "ChartServer.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <atomic>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

using json = nlohmann::json;

Cbot::Cbot()
    : base_asset_("SOL")
    , quote_asset_("USD")
    , running_(true)
{
    exchanges_.push_back("Kraken");
    exchanges_.push_back("Binance");
    for (const auto& exchange : exchanges_) {
        exch_data_map_[exchange] = std::vector<BookTicker>();
    }
    start_services();
}

Cbot::~Cbot() {
    stop_services();
}

void Cbot::start() {
    struct TerminalSettings {
        termios orig_termios;
        void enableRawMode() {
            tcgetattr(STDIN_FILENO, &orig_termios);
            termios raw = orig_termios;
            raw.c_lflag &= ~(ECHO | ICANON);
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        }
        void disableRawMode() {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        }
    } term;
    term.enableRawMode();
    std::thread uiThread(&Cbot::ui_loop, this);

    while (running_) {
        if (key_pressed()) {
            char c;
            if (read(STDIN_FILENO, &c, 1)) {
                handle_input(static_cast<MenuOption>(c));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    uiThread.join();
    term.disableRawMode();
}

void Cbot::start_services() {
    {
        std::lock_guard<std::mutex> lk(data_mtx_);
        for (const auto& exchange : exchanges_) {
            exch_data_map_[exchange].clear();
        }
    }

    for (const auto &exchange : exchanges_) {
        LOG_INFO("Starting " + exchange + " feed for " + base_asset_ + "/" + quote_asset_);
        Feed* feed = (exchange == "Kraken")
                   ? new KrakenFeed(base_asset_, quote_asset_)
                   : (exchange == "Binance")
                   ? new BinanceFeed(base_asset_, quote_asset_)
                   : (Feed*) nullptr;

        if (feed) {
            feed->start([this, exchange](const BookTicker& t) {
                std::lock_guard<std::mutex> lk(data_mtx_);
                auto& vec = exch_data_map_[exchange];
                vec.push_back(t);
                if (vec.size() > 10000) vec.erase(vec.begin());
            });
            exch_feed_map_[exchange] = feed;
        }
    }

    if (chart_server_) {
        delete chart_server_;
        chart_server_ = nullptr;
    }
    chart_server_ = new ChartServer(exch_data_map_, data_mtx_, "0.0.0.0", 18080);
    chart_server_->start();
}

void Cbot::stop_services() {
    for (auto &kv : exch_feed_map_) {
        delete kv.second;
    }
    exch_feed_map_.clear();
    if (chart_server_) {
        delete chart_server_;
        chart_server_ = nullptr;
    }
}

void Cbot::restart_services() {
    LOG_INFO("Restarting services with new base/quote assets");
    stop_services();
    start_services();
}

void Cbot::ui_loop() {
    while (running_) {
        if (!pause_refresh_) {
            render_tui();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Cbot::render_tui() {
    std::lock_guard<std::mutex> lk(data_mtx_);

    std::cout << "\033[2J\033[H";
    std::cout << std::string(40, '=') << "cbot TUI" << std::string(40, '=') << "\n";
    std::cout << "Base Asset: "   << base_asset_ << "    "
              << "Quote Asset: "  << quote_asset_ << "              "
              << "Arb Detected: " << (find_arb(exch_data_map_) ? GREEN_DOT : RED_DOT)
              << "\n\n";

    std::cout << std::left << std::setw(12) << "Exchange"
              << std::setw(12) << "Market"
              << std::setw(12) << "Price"
              << std::setw(12) << "MA(10s)"
              << std::setw(12) << "MA(30s)\n";
    std::cout << std::string(70, '-') << "\n";

    for (auto &entry : exch_data_map_) {
        const auto &[exchange, ticks] = entry;
        if (ticks.empty()) continue;

        double currentPrice = (ticks.back().bidPrice + ticks.back().askPrice) / 2.0;
        double ma10 = moving_average(ticks, 10.0);
        double ma30 = moving_average(ticks, 30.0);

        std::cout << std::left << std::setw(12) << exchange
                  << std::setw(12) << ticks.back().symbol
                  << std::setw(12) << std::fixed << std::setprecision(4) << currentPrice
                  << std::setw(12) << std::fixed << std::setprecision(4) << ma10
                  << std::setw(12) << std::fixed << std::setprecision(4) << ma30
                  << "\n";
    }

    std::cout << "\nMenu:\n"
              << "   (" << static_cast<char>(MenuOption::SetBaseAsset)   << ") Set base asset\n"
              << "   (" << static_cast<char>(MenuOption::SetQuoteAsset)  << ") Set quote asset\n"
              << "   (" << static_cast<char>(MenuOption::AddExchange)    << ") Add exchange\n"
              << "   (" << static_cast<char>(MenuOption::RemoveExchange) << ") Remove exchange\n"
              << "   (" << static_cast<char>(MenuOption::SaveFeed)       << ") Save feed data to file\n"
              << "   (" << static_cast<char>(MenuOption::Exit)           << ") Exit\n"
              << "Press key for option...\n";
    std::cout.flush();
}

bool Cbot::key_pressed() {
    int byteswaiting;
    ioctl(STDIN_FILENO, FIONREAD, &byteswaiting);
    return byteswaiting > 0;
}

void Cbot::handle_input(MenuOption opt) {
    switch (opt) {
        case MenuOption::SetBaseAsset: {
            std::string newBase = input("Enter new base asset: ");
            if (!newBase.empty()) {
                base_asset_ = newBase;
                restart_services();
            }
            break;
        }
        case MenuOption::SetQuoteAsset: {
            std::string newQuote = input("Enter new quote asset: ");
            if (!newQuote.empty()) {
                quote_asset_ = newQuote;
                restart_services();
            }
            break;
        }
        case MenuOption::AddExchange: {
            std::string newExch = input("Enter exchange to add (e.g., Kraken/Binance): ");
            if (!newExch.empty()) {
                if (std::find(exchanges_.begin(), exchanges_.end(), newExch) != exchanges_.end()) {
                    std::cout << "\nExchange already exists!\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                } else {
                    exchanges_.push_back(newExch);
                    {
                        std::lock_guard<std::mutex> lk(data_mtx_);
                        exch_data_map_[newExch] = std::vector<BookTicker>();
                    }
                    restart_services();
                }
            }
            break;
        }
        case MenuOption::RemoveExchange: {
            std::string remExch = input("Enter exchange to remove: ");
            if (!remExch.empty()) {
                auto it = std::find(exchanges_.begin(), exchanges_.end(), remExch);
                if (it == exchanges_.end()) {
                    std::cout << "\nExchange not found!\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                } else {
                    exchanges_.erase(it);
                    {
                        std::lock_guard<std::mutex> lk(data_mtx_);
                        exch_data_map_.erase(remExch);
                    }
                    restart_services();
                }
            }
            break;
        }
        case MenuOption::SaveFeed:
            dump_feed_data();
            break;
        case MenuOption::Exit:
            running_ = false;
            break;
        case MenuOption::Invalid:
        default:
            break;
    }
}

std::string Cbot::input(const std::string &prompt) {
    pause_refresh_ = true;

    // Temporarily disable raw mode so user can type normally.
    termios orig;
    tcgetattr(STDIN_FILENO, &orig);
    termios temp = orig;
    temp.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &temp);

    std::cout << "\n" << prompt;
    std::string input;
    std::getline(std::cin, input);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);

    pause_refresh_ = false;
    return input;
}

void Cbot::dump_feed_data() {
    std::lock_guard<std::mutex> lk(data_mtx_);

    std::ofstream out_file("feed_data.json");
    if (!out_file) {
        std::cerr << "Error: cannot open file for writing.\n";
        return;
    }

    json j;
    for (auto &entry : exch_data_map_) {
        j[entry.first] = entry.second;
    }
    out_file << j.dump(4);

    std::cout << "\nFeed data saved to feed_data.json.\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
