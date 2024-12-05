#pragma once

#include <optional>
#include <unordered_map>
#include <list>
#include <mutex>
#include <shared_mutex>
#include "cache_item.hpp"
#include "metrics.hpp"

class LRUCache {
public:
    explicit LRUCache(size_t capacity);

    std::optional<CacheItem> get(int key);
    void put(int key, CacheItem value);
    void remove(int key);
    void clear();
    size_t size() const;

    void save_to_file(const std::string& filename);
    void load_from_file(const std::string& filename);

    CacheMetrics get_metrics() const;

private:
    struct CacheEntry {
        CacheItem item;
        std::chrono::steady_clock::time_point last_access;
    };

    void put_internal(int key, CacheItem value);

    void evict_if_needed();

    mutable std::mutex mutex_;
    size_t capacity_;
    std::unordered_map<int, std::pair<std::list<CacheEntry>::iterator, CacheEntry>> cache_;
    std::list<CacheEntry> lru_list_;
    CacheMetrics metrics_;
};
