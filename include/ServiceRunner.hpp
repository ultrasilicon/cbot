#ifndef SERVICE_RUNNER_HPP
#define SERVICE_RUNNER_HPP

#include <csignal>
#include <mutex>
#include <condition_variable>
#include <atomic>

/**
 * @class ServiceRunner
 * @brief Manages the lifecycle of cbot app by handling signals.
 */
class ServiceRunner
{
public:
    ServiceRunner();
    void run();
    static void stop();

private:
    static std::atomic_bool stop_flag;
    static std::mutex mutex_;
    static std::condition_variable condition_;

    static void signal_handler(int);
};

#endif // SERVICE_RUNNER_HPP
