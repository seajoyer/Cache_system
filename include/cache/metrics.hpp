#pragma once

#include <chrono>
#include <atomic>

class CacheMetrics {
public:
    CacheMetrics() = default;

    // Add copy constructor
    CacheMetrics(const CacheMetrics& other);

    // Add copy assignment operator
    CacheMetrics& operator=(const CacheMetrics& other);

    void record_read(std::chrono::nanoseconds duration);
    void record_write(std::chrono::nanoseconds duration);
    void update_memory_usage(size_t bytes);

    double get_avg_read_time() const;
    double get_avg_write_time() const;
    size_t get_memory_usage() const;

private:
    std::atomic<uint64_t> total_reads_{0};
    std::atomic<uint64_t> total_writes_{0};
    std::atomic<uint64_t> total_read_time_{0};
    std::atomic<uint64_t> total_write_time_{0};
    std::atomic<size_t> memory_usage_{0};
};
