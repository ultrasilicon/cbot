#ifndef CBOT_HPP
#define CBOT_HPP

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include "Logger.hpp"
#include "BookTicker.hpp"
#include "Feed.hpp"
#include "ChartServer.hpp"

/**
 * @brief Main TUI and service manager (feeds and chart server) for the CEX bot.
 */
class Cbot {
public:
    Cbot();
    ~Cbot();
    void start();

private:
    enum class MenuOption : char {
        SetBaseAsset    = 'b',
        SetQuoteAsset   = 'q',
        AddExchange     = 'a',
        RemoveExchange  = 'r',
        SaveFeed        = 's',
        Exit            = 'x',
        Invalid         = 0
    };

    const char* RED_DOT = "\033[1;31m●\033[0m";
    const char* GREEN_DOT = "\033[1;32m●\033[0m";

    // config
    std::string base_asset_;
    std::string quote_asset_;
    std::vector<std::string> exchanges_;

    // services
    std::map<std::string, Feed*> exch_feed_map_;
    ChartServer* chart_server_ = nullptr;

    // data
    std::map<std::string, std::vector<BookTicker>> exch_data_map_;
    std::mutex data_mtx_;

    // TUI state
    std::atomic<bool> running_;
    std::atomic<bool> pause_refresh_ = false;

    // managing feeds and chart server lifecycle
    void start_services();
    void stop_services();
    void restart_services();

    // TUI
    void ui_loop();
    void render_tui();

    // input handling
    bool key_pressed();
    void handle_input(MenuOption opt);
    std::string input(const std::string &prompt);

    void dump_feed_data();
};

// manipulation of terminal settings
struct TerminalSettings {
    // This is kind of buggy, only enable then disable works, not other way round
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
};

#endif // CBOT_HPP
