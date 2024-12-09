#include "cache/lru_cache.hpp"
#include <fstream>

LRUCache::LRUCache(size_t capacity) : capacity_(capacity) {}

std::experimental::optional<CacheItem> LRUCache::get(int key) {
    auto start = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        metrics_.record_cache_miss();  // Make sure this gets called
        auto duration = std::chrono::steady_clock::now() - start;
        metrics_.record_read(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
        return std::experimental::nullopt;
    }

    metrics_.record_cache_hit();

    auto& [list_it, entry] = it->second;
    entry.last_access = std::chrono::steady_clock::now();

    lru_list_.splice(lru_list_.begin(), lru_list_, list_it);

    auto duration = std::chrono::steady_clock::now() - start;
    metrics_.record_read(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));

    return entry.item;
}

CacheMetrics LRUCache::get_metrics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return metrics_;
}

void LRUCache::put_internal(int key, CacheItem value) {
    // Calculate exact memory size for the new item
    size_t string_memory = value.faculty.capacity() * sizeof(char) +
                          value.course.capacity() * sizeof(char) +
                          value.title.capacity() * sizeof(char) +
                          value.description.capacity() * sizeof(char) +
                          value.telegramGroupLink.capacity() * sizeof(char);

    // Include overhead for the container elements and pointers
    constexpr size_t list_node_overhead = 3 * sizeof(void*);  // Typical overhead for list node (prev/next pointers + data)
    constexpr size_t map_node_overhead = 4 * sizeof(void*);   // Typical overhead for map node (pointers + color bit)

    size_t item_total_memory = sizeof(CacheEntry) +
                              string_memory +
                              list_node_overhead +
                              map_node_overhead;

    // Remove old item's memory if it exists
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        const auto& old_item = it->second.second.item;
        size_t old_string_memory = old_item.faculty.capacity() * sizeof(char) +
                                  old_item.course.capacity() * sizeof(char) +
                                  old_item.title.capacity() * sizeof(char) +
                                  old_item.description.capacity() * sizeof(char) +
                                  old_item.telegramGroupLink.capacity() * sizeof(char);

        size_t old_total_memory = sizeof(CacheEntry) +
                                 old_string_memory +
                                 list_node_overhead +
                                 map_node_overhead;

        metrics_.update_memory_usage(-static_cast<ssize_t>(old_total_memory));
        lru_list_.erase(it->second.first);
        cache_.erase(it);
    }

    evict_if_needed();

    // Add new entry
    CacheEntry entry{std::move(value), std::chrono::steady_clock::now()};
    lru_list_.push_front(entry);
    cache_[key] = {lru_list_.begin(), entry};

    metrics_.update_memory_usage(static_cast<ssize_t>(item_total_memory));
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
