#include "../include/ServiceRunner.hpp"

std::atomic_bool ServiceRunner::stop_flag{false};
std::mutex ServiceRunner::mutex_;
std::condition_variable ServiceRunner::condition_;

ServiceRunner::ServiceRunner()
{
    std::signal(SIGINT, ServiceRunner::signal_handler);
    std::signal(SIGTERM, ServiceRunner::signal_handler);
}

void ServiceRunner::run()
{
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [] { return stop_flag.load(); });
}

void ServiceRunner::stop()
{
    stop_flag.store(true);
    condition_.notify_all();
}

void ServiceRunner::signal_handler(int)
{
    stop();
}
