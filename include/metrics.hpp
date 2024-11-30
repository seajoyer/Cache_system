#pragma once

#include <chrono>
#include <atomic>
#include <string>

class Metrics {
public:
    void record_operation(const std::string& operation, std::chrono::nanoseconds duration);
    void update_disk_usage(size_t usage);

    double get_average_put_time() const;
    double get_average_get_time() const;
    double get_average_remove_time() const;
    size_t get_disk_usage() const;
private:
    std::atomic<uint64_t> put_count_{0};
    std::atomic<uint64_t> get_count_{0};
    std::atomic<uint64_t> remove_count_{0};
    std::atomic<uint64_t> put_total_time_{0};
    std::atomic<uint64_t> get_total_time_{0};
    std::atomic<uint64_t> remove_total_time_{0};
    std::atomic<size_t> disk_usage_{0};
};
