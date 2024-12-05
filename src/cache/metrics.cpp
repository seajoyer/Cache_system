#include "cache/metrics.hpp"

void CacheMetrics::record_read(std::chrono::nanoseconds duration) {
    total_reads_++;
    total_read_time_ += duration.count();
}

void CacheMetrics::record_write(std::chrono::nanoseconds duration) {
    total_writes_++;
    total_write_time_ += duration.count();
}

void CacheMetrics::update_memory_usage(size_t bytes) {
    memory_usage_ += bytes;
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

size_t CacheMetrics::get_memory_usage() const {
    return memory_usage_.load();
}
