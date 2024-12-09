#include "cache/metrics.hpp"

CacheMetrics::CacheMetrics() = default;

CacheMetrics::CacheMetrics(const CacheMetrics& other) :
    total_reads_(other.total_reads_.load()),
    total_writes_(other.total_writes_.load()),
    total_read_time_(other.total_read_time_.load()),
    total_write_time_(other.total_write_time_.load()),
    memory_usage_(other.memory_usage_.load()),
    peak_memory_usage_(other.peak_memory_usage_.load()),
    item_count_(other.item_count_.load()),
    string_memory_(other.string_memory_.load()),
    cache_hits_(other.cache_hits_.load()),
    cache_misses_(other.cache_misses_.load()) {
}

CacheMetrics& CacheMetrics::operator=(const CacheMetrics& other) {
    if (this != &other) {
        total_reads_.store(other.total_reads_.load(), std::memory_order_relaxed);
        total_writes_.store(other.total_writes_.load(), std::memory_order_relaxed);
        total_read_time_.store(other.total_read_time_.load(), std::memory_order_relaxed);
        total_write_time_.store(other.total_write_time_.load(), std::memory_order_relaxed);
        memory_usage_.store(other.memory_usage_.load(), std::memory_order_relaxed);
        peak_memory_usage_.store(other.peak_memory_usage_.load(), std::memory_order_relaxed);
        item_count_.store(other.item_count_.load(), std::memory_order_relaxed);
        string_memory_.store(other.string_memory_.load(), std::memory_order_relaxed);
        cache_hits_.store(other.cache_hits_.load(), std::memory_order_relaxed);
        cache_misses_.store(other.cache_misses_.load(), std::memory_order_relaxed);
    }
    return *this;
}

void CacheMetrics::record_read(std::chrono::nanoseconds duration) {
    total_reads_.fetch_add(1, std::memory_order_relaxed);
    uint64_t ns = std::max<uint64_t>(1ULL, static_cast<uint64_t>(duration.count()));
    total_read_time_.fetch_add(ns, std::memory_order_relaxed);
}

void CacheMetrics::record_write(std::chrono::nanoseconds duration) {
    total_writes_.fetch_add(1, std::memory_order_relaxed);
    uint64_t ns = std::max<uint64_t>(1ULL, static_cast<uint64_t>(duration.count()));
    total_write_time_.fetch_add(ns, std::memory_order_relaxed);
}

void CacheMetrics::reset_counters() {
    total_reads_.store(0, std::memory_order_relaxed);
    total_writes_.store(0, std::memory_order_relaxed);
    total_read_time_.store(0, std::memory_order_relaxed);
    total_write_time_.store(0, std::memory_order_relaxed);
    memory_usage_.store(0, std::memory_order_relaxed);
    peak_memory_usage_.store(0, std::memory_order_relaxed);
    item_count_.store(0, std::memory_order_relaxed);
    string_memory_.store(0, std::memory_order_relaxed);
    cache_hits_.store(0, std::memory_order_relaxed);
    cache_misses_.store(0, std::memory_order_relaxed);
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

size_t CacheMetrics::get_memory_usage() const {
    return static_cast<size_t>(std::max<ssize_t>(0, memory_usage_.load(std::memory_order_relaxed)));
}

size_t CacheMetrics::get_peak_memory_usage() const {
    return peak_memory_usage_.load(std::memory_order_relaxed);
}

size_t CacheMetrics::get_string_memory() const {
    return string_memory_.load(std::memory_order_relaxed);
}

size_t CacheMetrics::get_item_count() const {
    return item_count_.load(std::memory_order_relaxed);
}

double CacheMetrics::get_hit_rate() const {
    uint64_t hits = cache_hits_.load(std::memory_order_relaxed);
    uint64_t misses = cache_misses_.load(std::memory_order_relaxed);
    uint64_t total = hits + misses;
    if (total == 0) return 0.0;
    return (static_cast<double>(hits) / total) * 100.0;
}

double CacheMetrics::get_average_item_size() const {
    size_t items = get_item_count();
    if (items == 0) return 0.0;
    return static_cast<double>(get_memory_usage()) / items;
}
