#include "cache/lru_cache.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

LRUCache::LRUCache(size_t capacity) : capacity_(capacity) {
    // Pre-reserve space to avoid rehashing
    cache_.reserve(capacity);
}

std::experimental::optional<CacheItem> LRUCache::get(int key) {
    auto start = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        metrics_.record_cache_miss();
        auto duration = std::chrono::steady_clock::now() - start;
        metrics_.record_read(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
        return std::experimental::nullopt;
    }

    metrics_.record_cache_hit();

    // Update access time and move to front of LRU list
    auto& pair = it->second;
    auto& list_it = pair.first;
    auto& entry = pair.second;
    entry.last_access = std::chrono::steady_clock::now();
    lru_list_.splice(lru_list_.begin(), lru_list_, list_it);

    auto duration = std::chrono::steady_clock::now() - start;
    metrics_.record_read(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));

    return entry.item;
}

size_t LRUCache::calculate_item_memory_size(const CacheItem& item) const {
    // Calculate memory for strings (actual capacity)
    size_t string_memory =
        item.faculty.capacity() +
        item.course.capacity() +
        item.title.capacity() +
        item.description.capacity() +
        item.telegramGroupLink.capacity();

    // Add memory for integers and string objects themselves
    // Base size includes:
    // 1. CacheItem structure
    // 2. Map entry overhead (pair of iterator and CacheEntry)
    // 3. Approximate list node overhead (3 pointers: prev, next, and data)
    constexpr size_t base_size =
        sizeof(CacheItem) +   // Base structure
        sizeof(std::pair<typename std::list<CacheEntry>::iterator, CacheEntry>) + // Map entry
        3 * sizeof(void*);  // Approximate list node overhead

    return base_size + string_memory;
}

void LRUCache::put_internal(int key, CacheItem value) {
    size_t new_item_size = calculate_item_memory_size(value);

    // Remove old item's memory if it exists
    auto old_it = cache_.find(key);
    if (old_it != cache_.end()) {
        size_t old_size = calculate_item_memory_size(old_it->second.second.item);
        metrics_.update_memory_usage(-static_cast<ssize_t>(old_size));
        lru_list_.erase(old_it->second.first);
        cache_.erase(old_it);
    }

    evict_if_needed();

    // Add new entry
    CacheEntry entry{std::move(value), std::chrono::steady_clock::now()};
    lru_list_.push_front(entry);
    cache_[key] = {lru_list_.begin(), entry};
    metrics_.update_memory_usage(static_cast<ssize_t>(new_item_size));
}

void LRUCache::put(int key, CacheItem value) {
    auto start = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);
    put_internal(key, std::move(value));
    auto duration = std::chrono::steady_clock::now() - start;
    metrics_.record_write(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
}

void LRUCache::evict_if_needed() {
    while (cache_.size() >= capacity_ && !lru_list_.empty()) {
        auto last = std::prev(lru_list_.end());
        auto it = std::find_if(cache_.begin(), cache_.end(),
            [&](const auto& pair) { return pair.second.first == last; });

        if (it != cache_.end()) {
            size_t item_size = calculate_item_memory_size(it->second.second.item);
            metrics_.update_memory_usage(-static_cast<ssize_t>(item_size));
            cache_.erase(it);
        }
        lru_list_.pop_back();
    }
}

void LRUCache::remove(int key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        size_t item_size = calculate_item_memory_size(it->second.second.item);
        metrics_.update_memory_usage(-static_cast<ssize_t>(item_size));
        lru_list_.erase(it->second.first);
        cache_.erase(it);
    }
}

void LRUCache::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
    lru_list_.clear();
    metrics_ = CacheMetrics(); // Reset metrics
}

size_t LRUCache::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.size();
}

size_t LRUCache::capacity() const {
    return capacity_;
}

void LRUCache::save_to_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& entry : cache_) {
            j.push_back({
                {"key", entry.first},
                {"value", entry.second.second.item.to_json()}
            });
        }

        std::ofstream file(filename, std::ios::out | std::ios::trunc);
        if (!file) {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }

        file << j.dump(4);
        file.close();

        if (file.fail()) {
            throw std::runtime_error("Failed to write to file: " + filename);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to save cache: " + std::string(e.what()));
    }
}

void LRUCache::load_from_file(const std::string& filename) {
    nlohmann::json j;
    {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Failed to open file for reading: " + filename);
        }
        file >> j;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    try {
        cache_.clear();
        lru_list_.clear();

        for (const auto& item : j) {
            int key = item["key"].get<int>();
            CacheItem value = CacheItem::from_json(item["value"]);
            put_internal(key, value);  // Use internal method to avoid recursive locking
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load cache: " + std::string(e.what()));
    }
}
