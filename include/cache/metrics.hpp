#pragma once

#include <chrono>
#include <atomic>
#include <sys/types.h>

class CacheMetrics {
public:
    // Remove default constructor declaration since we'll define it explicitly
    CacheMetrics();

    // Add copy constructor
    CacheMetrics(const CacheMetrics& other);

    // Add copy assignment operator
    CacheMetrics& operator=(const CacheMetrics& other);

    void record_read(std::chrono::nanoseconds duration);
    void record_write(std::chrono::nanoseconds duration);
    void update_memory_usage(ssize_t bytes);

    [[nodiscard]] double get_avg_read_time() const;
    [[nodiscard]] double get_avg_write_time() const;
    [[nodiscard]] size_t get_memory_usage() const;

    void record_cache_hit();
    void record_cache_miss();
    [[nodiscard]] double get_hit_rate() const;

private:
    std::atomic<uint64_t> total_reads_{0};
    std::atomic<uint64_t> total_writes_{0};
    std::atomic<uint64_t> total_read_time_{0};
    std::atomic<uint64_t> total_write_time_{0};
    std::atomic<ssize_t> memory_usage_{0};
    std::atomic<uint64_t> cache_hits_{0};
    std::atomic<uint64_t> cache_misses_{0};
};
