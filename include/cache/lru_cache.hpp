#pragma once

#include <unordered_map>
#include <list>
#include <mutex>
#include <experimental/optional>
#include "cache_item.hpp"
#include "metrics.hpp"

class LRUCache {
public:
    explicit LRUCache(size_t capacity);

    std::experimental::optional<CacheItem> get(int key);
    void put(int key, CacheItem value);
    void remove(int key);
    void clear();
    [[nodiscard]] size_t size() const;
    [[nodiscard]] size_t capacity() const;

    void save_to_file(const std::string& filename);
    void load_from_file(const std::string& filename);

    [[nodiscard]] const CacheMetrics& get_metrics() const { return metrics_; }

private:
    struct CacheEntry {
        CacheItem item;
        std::chrono::steady_clock::time_point last_access;
    };

    void put_internal(int key, CacheItem value);
    void evict_if_needed();
    [[nodiscard]] size_t calculate_item_memory_size(const CacheItem& item) const;

    mutable std::mutex mutex_;
    const size_t capacity_;
    std::unordered_map<int, std::pair<typename std::list<CacheEntry>::iterator, CacheEntry>> cache_;
    std::list<CacheEntry> lru_list_;
    CacheMetrics metrics_;
};
