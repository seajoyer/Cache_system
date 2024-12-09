#include "cache/metrics.hpp"

CacheMetrics::CacheMetrics() :
    total_reads_(0),
    total_writes_(0),
    total_read_time_(0),
    total_write_time_(0),
    memory_usage_(0),
    cache_hits_(0),
    cache_misses_(0) {}

CacheMetrics::CacheMetrics(const CacheMetrics& other) :
    total_reads_(other.total_reads_.load()),
    total_writes_(other.total_writes_.load()),
    total_read_time_(other.total_read_time_.load()),
    total_write_time_(other.total_write_time_.load()),
    memory_usage_(other.memory_usage_.load()),
    cache_hits_(other.cache_hits_.load()),
    cache_misses_(other.cache_misses_.load()) {}

CacheMetrics& CacheMetrics::operator=(const CacheMetrics& other) {
    if (this != &other) {
        total_reads_.store(other.total_reads_.load());
        total_writes_.store(other.total_writes_.load());
        total_read_time_.store(other.total_read_time_.load());
        total_write_time_.store(other.total_write_time_.load());
        memory_usage_.store(other.memory_usage_.load());
        cache_hits_.store(other.cache_hits_.load());
        cache_misses_.store(other.cache_misses_.load());
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
    uint64_t reads = total_reads_.load(std::memory_order_relaxed);
    if (reads == 0) return 0.0;
    return static_cast<double>(total_read_time_.load(std::memory_order_relaxed)) / reads;
}

double CacheMetrics::get_avg_write_time() const {
    uint64_t writes = total_writes_.load(std::memory_order_relaxed);
    if (writes == 0) return 0.0;
    return static_cast<double>(total_write_time_.load(std::memory_order_relaxed)) / writes;
}

void CacheMetrics::record_cache_hit() {
    cache_hits_.fetch_add(1, std::memory_order_relaxed);
}

void CacheMetrics::record_cache_miss() {
    cache_misses_.fetch_add(1, std::memory_order_relaxed);
}

double CacheMetrics::get_hit_rate() const {
    uint64_t hits = cache_hits_.load(std::memory_order_relaxed);
    uint64_t misses = cache_misses_.load(std::memory_order_relaxed);
    uint64_t total = hits + misses;

    if (total == 0) return 0.0;
    return (static_cast<double>(hits) / total) * 100.0;
}
