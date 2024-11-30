#include "metrics.hpp"

void Metrics::increment_hit_count() {
    ++hit_count_;
}

void Metrics::increment_miss_count() {
    ++miss_count_;
}

void Metrics::increment_eviction_count() {
    ++eviction_count_;
}

size_t Metrics::get_hit_count() const {
    return hit_count_;
}

size_t Metrics::get_miss_count() const {
    return miss_count_;
}

double Metrics::get_hit_ratio() const {
    size_t total = hit_count_ + miss_count_;
    return total > 0 ? static_cast<double>(hit_count_) / total : 0.0;
}

size_t Metrics::get_eviction_count() const {
    return eviction_count_;
}
