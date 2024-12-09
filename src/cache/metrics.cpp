#include "cache/metrics.hpp"

// Add copy constructor implementation
CacheMetrics::CacheMetrics(const CacheMetrics& other) :
    total_reads_(other.total_reads_.load()),
    total_writes_(other.total_writes_.load()),
    total_read_time_(other.total_read_time_.load()),
    total_write_time_(other.total_write_time_.load()),
    memory_usage_(other.memory_usage_.load()) {}

// Add copy assignment operator implementation
CacheMetrics& CacheMetrics::operator=(const CacheMetrics& other) {
    if (this != &other) {
        total_reads_.store(other.total_reads_.load());
        total_writes_.store(other.total_writes_.load());
        total_read_time_.store(other.total_read_time_.load());
        total_write_time_.store(other.total_write_time_.load());
        memory_usage_.store(other.memory_usage_.load());
    }
    return *this;
}

void CacheMetrics::record_read(std::chrono::nanoseconds duration) {
    total_reads_.fetch_add(1, std::memory_order_relaxed);
    total_read_time_.fetch_add(duration.count(), std::memory_order_relaxed);
}

void CacheMetrics::record_write(std::chrono::nanoseconds duration) {
    total_writes_.fetch_add(1, std::memory_order_relaxed);
    total_write_time_.fetch_add(duration.count(), std::memory_order_relaxed);
}

void CacheMetrics::update_memory_usage(ssize_t delta) {
    memory_usage_.fetch_add(delta, std::memory_order_relaxed);
}

size_t CacheMetrics::get_memory_usage() const {
    ssize_t usage = memory_usage_.load(std::memory_order_relaxed);
    return usage > 0 ? static_cast<size_t>(usage) : 0;
}

double CacheMetrics::get_avg_read_time() const {
    uint64_t reads = total_reads_.load();
    if (reads == 0) return 0.0;
    return static_cast<double>(total_read_time_.load()) / reads;
}

double CacheMetrics::get_avg_write_time() const {
    uint64_t writes = total_writes_.load();
    if (writes == 0) return 0.0;
    return static_cast<double>(total_write_time_.load()) / writes;
}

void CacheMetrics::record_cache_hit() {
    cache_hits_++;
}

void CacheMetrics::record_cache_miss() {
    cache_misses_++;
}

double CacheMetrics::get_hit_rate() const {
    uint64_t total = cache_hits_.load() + cache_misses_.load();
    if (total == 0) return 0.0;
    return (static_cast<double>(cache_hits_.load()) / total) * 100.0;
}
