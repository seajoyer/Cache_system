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

    double get_avg_read_time() const;
    double get_avg_write_time() const;
    size_t get_memory_usage() const;

    void record_cache_hit();
    void record_cache_miss();
    double get_hit_rate() const;

    void update_string_memory(ssize_t delta) {
        string_memory_.fetch_add(delta, std::memory_order_relaxed);
    }

    void update_item_count(ssize_t delta) {
        item_count_.fetch_add(delta, std::memory_order_relaxed);
    }

    void update_peak_memory() {
        size_t current = memory_usage_.load(std::memory_order_relaxed);
        size_t peak = peak_memory_usage_.load(std::memory_order_relaxed);
        while (current > peak &&
               !peak_memory_usage_.compare_exchange_weak(peak, current)) {
            // Keep trying if the compare_exchange failed
        }
    }

    size_t get_peak_memory_usage() const {
        return peak_memory_usage_.load(std::memory_order_relaxed);
    }

    size_t get_item_count() const {
        return item_count_.load(std::memory_order_relaxed);
    }

    size_t get_string_memory() const {
        return string_memory_.load(std::memory_order_relaxed);
    }

    double get_average_item_size() const {
        size_t items = get_item_count();
        return items > 0 ? static_cast<double>(get_memory_usage()) / items : 0.0;
    }

private:
    std::atomic<uint64_t> total_reads_{0};
    std::atomic<uint64_t> total_writes_{0};
    std::atomic<uint64_t> total_read_time_{0};
    std::atomic<uint64_t> total_write_time_{0};
    std::atomic<ssize_t> memory_usage_{0};
    std::atomic<uint64_t> cache_hits_{0};
    std::atomic<uint64_t> cache_misses_{0};
    std::atomic<size_t> peak_memory_usage_{0};
    std::atomic<size_t> item_count_{0};
    std::atomic<size_t> string_memory_{0};
};
