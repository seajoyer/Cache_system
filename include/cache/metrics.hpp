#pragma once

#include <atomic>
#include <chrono>
#include <algorithm>
#include <sys/types.h>

class CacheMetrics {
public:
    CacheMetrics();
    CacheMetrics(const CacheMetrics& other);
    CacheMetrics& operator=(const CacheMetrics& other);

    // Inline methods (small and frequently called)
    void record_cache_hit() {
        cache_hits_.fetch_add(1, std::memory_order_relaxed);
    }

    void record_cache_miss() {
        cache_misses_.fetch_add(1, std::memory_order_relaxed);
    }

    void update_memory_usage(ssize_t delta) {
        auto current = memory_usage_.fetch_add(delta, std::memory_order_relaxed);
        auto new_usage = current + delta;

        auto peak = peak_memory_usage_.load(std::memory_order_relaxed);
        while (new_usage > static_cast<ssize_t>(peak) &&
               !peak_memory_usage_.compare_exchange_weak(peak, new_usage)) {
            // Keep trying if CAS failed
        }
    }

    void update_string_memory(ssize_t delta) {
        string_memory_.fetch_add(delta, std::memory_order_relaxed);
    }

    void update_item_count(ssize_t delta) {
        item_count_.fetch_add(delta, std::memory_order_relaxed);
    }

    // Non-inline methods (declare only)
    void record_read(std::chrono::nanoseconds duration);
    void record_write(std::chrono::nanoseconds duration);
    void reset_counters();

    // Getters
    double get_avg_read_time() const;
    double get_avg_write_time() const;
    size_t get_memory_usage() const;
    size_t get_peak_memory_usage() const;
    size_t get_string_memory() const;
    size_t get_item_count() const;
    double get_hit_rate() const;
    double get_average_item_size() const;

private:
    std::atomic<uint64_t> total_reads_{0};
    std::atomic<uint64_t> total_writes_{0};
    std::atomic<uint64_t> total_read_time_{0};
    std::atomic<uint64_t> total_write_time_{0};
    std::atomic<ssize_t> memory_usage_{0};
    std::atomic<size_t> peak_memory_usage_{0};
    std::atomic<size_t> item_count_{0};
    std::atomic<size_t> string_memory_{0};
    std::atomic<uint64_t> cache_hits_{0};
    std::atomic<uint64_t> cache_misses_{0};
};
