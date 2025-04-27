// Logger.h
#pragma once

#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

class Logger {
public:
    enum class Level { INFO, WARN, ERROR };

    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    inline void info(const std::string& domain, const std::string& msg)
    {
        log(Level::INFO,  domain, msg);
    }

    inline void warn(const std::string& domain, const std::string& msg)
    {
        log(Level::WARN,  domain, msg);
    }

    inline void error(const std::string& domain, const std::string& msg)
    {
        log(Level::ERROR, domain, msg);
    }

private:
    std::ofstream _file;
    std::mutex _mutex;

    Logger() {
        const char* home = std::getenv("HOME");
        if (!home) {
            if (auto pw = getpwuid(getuid())) home = pw->pw_dir;
            else home = "/";
        }

        const char* xdg = std::getenv("XDG_DATA_HOME");
        std::string base = xdg ? xdg : std::string(home) + "/.local/share";

        // ~/.local/share/cbot/logs or $XDG_DATA_HOME/cbot/logs
        std::string path = base + "/cbot";
        mkdir(path.c_str(), 0755);
        path += "/logs";
        mkdir(path.c_str(), 0755);

        std::string logfile = path + "/cbot.log";
        _file.open(logfile, std::ios::out | std::ios::trunc);
        if (!_file.is_open()) {
            throw std::runtime_error("Unable to open " + logfile + " for writing");
        }
    }

    ~Logger() {
        if (_file.is_open()) _file.close();
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(Level level, const std::string& domain, const std::string& msg) {
        std::lock_guard<std::mutex> lock(_mutex);
        static const char* names[] = { "INFO", "WARN", "ERROR" };

        _file << "[" << timestamp() << "]"
              << "[" << names[static_cast<int>(level)] << "]";

        if (!domain.empty())
            _file << "[" << domain << "]";

        _file << " " << msg << "\n";
        _file.flush();
    }

    std::string timestamp() {
        using namespace std::chrono;
        auto now = system_clock::now();

        std::time_t t = system_clock::to_time_t(now);
        std::tm tm{};
        localtime_r(&t, &tm);

        std::ostringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

static std::string strip_extension(const std::string& filename) {
    size_t lastdot = filename.find_last_of('.');
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot);
}

// default log domain to file name
#ifndef LOG_DOMAIN
// A hack to strip file path into file name only, what a pain, C++!
    #define LOG_DOMAIN strip_extension((__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__))
#endif

#define LOG_INFO(msg)  Logger::instance().info(LOG_DOMAIN, msg)
#define LOG_WARN(msg)  Logger::instance().warn(LOG_DOMAIN, msg)
#define LOG_ERROR(msg) Logger::instance().error(LOG_DOMAIN, msg)
