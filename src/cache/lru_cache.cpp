#include "cache/lru_cache.hpp"
#include <fstream>

LRUCache::LRUCache(size_t capacity) : capacity_(capacity) {}

std::optional<CacheItem> LRUCache::get(int key) {
    auto start = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return std::nullopt;
    }

    auto& [list_it, entry] = it->second;
    entry.last_access = std::chrono::steady_clock::now();

    lru_list_.erase(list_it);
    lru_list_.push_front(entry);
    it->second.first = lru_list_.begin();

    auto duration = std::chrono::steady_clock::now() - start;
    metrics_.record_read(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));

    return entry.item;
}

void LRUCache::put_internal(int key, CacheItem value) {
    evict_if_needed();

    CacheEntry entry{std::move(value), std::chrono::steady_clock::now()};
    lru_list_.push_front(entry);
    cache_[key] = {lru_list_.begin(), entry};

    metrics_.update_memory_usage(sizeof(CacheEntry) +
        sizeof(std::pair<int, std::pair<std::list<CacheEntry>::iterator, CacheEntry>>));
}

void LRUCache::put(int key, CacheItem value) {
    auto start = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);
    put_internal(key, value);
    auto duration = std::chrono::steady_clock::now() - start;
    metrics_.record_write(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
}

void LRUCache::evict_if_needed() {
    while (cache_.size() >= capacity_ && !lru_list_.empty()) {
        auto last = std::prev(lru_list_.end());
        auto it = std::find_if(cache_.begin(), cache_.end(),
            [&](const auto& pair) { return pair.second.first == last; });

        if (it != cache_.end()) {
            cache_.erase(it);
            metrics_.update_memory_usage(-(sizeof(CacheEntry) + sizeof(std::pair<int, std::pair<std::list<CacheEntry>::iterator, CacheEntry>>)));
        }
        lru_list_.pop_back();
    }
}

void LRUCache::remove(int key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        lru_list_.erase(it->second.first);
        cache_.erase(it);
    }
}

void LRUCache::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
    lru_list_.clear();
}

size_t LRUCache::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.size();
}

void LRUCache::save_to_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& [key, value] : cache_) {
            j.push_back({
                {"key", key},
                {"value", value.second.item.to_json()}
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
