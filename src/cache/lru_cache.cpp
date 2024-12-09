#include "cache/lru_cache.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

LRUCache::LRUCache(size_t capacity) : capacity_(capacity) {
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
    auto& pair = it->second;
    auto& list_it = pair.first;
    auto& entry = pair.second;

    lru_list_.splice(lru_list_.begin(), lru_list_, list_it);

    auto duration = std::chrono::steady_clock::now() - start;
    metrics_.record_read(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));

    return entry.item;
}

size_t LRUCache::calculate_item_memory_size(const CacheItem& item) const {
    size_t string_content =
        item.faculty.size() * sizeof(char) +
        item.course.size() * sizeof(char) +
        item.title.size() * sizeof(char) +
        item.description.size() * sizeof(char) +
        item.telegramGroupLink.size() * sizeof(char);

    size_t string_capacity_overhead =
        (item.faculty.capacity() - item.faculty.size()) * sizeof(char) +
        (item.course.capacity() - item.course.size()) * sizeof(char) +
        (item.title.capacity() - item.title.size()) * sizeof(char) +
        (item.description.capacity() - item.description.size()) * sizeof(char) +
        (item.telegramGroupLink.capacity() - item.telegramGroupLink.size()) * sizeof(char);

    constexpr size_t string_obj_overhead = sizeof(std::string);
    size_t total_string_overhead = string_obj_overhead * 5;

    constexpr size_t base_size =
        sizeof(CacheItem) +
        sizeof(std::pair<typename std::list<CacheEntry>::iterator, CacheEntry>) +
        sizeof(CacheEntry);

    return base_size + string_content + string_capacity_overhead + total_string_overhead;
}

size_t LRUCache::calculate_string_memory(const CacheItem& item) const {
    return item.faculty.capacity() * sizeof(char) +
           item.course.capacity() * sizeof(char) +
           item.title.capacity() * sizeof(char) +
           item.description.capacity() * sizeof(char) +
           item.telegramGroupLink.capacity() * sizeof(char);
}

void LRUCache::update_metrics_for_item(const CacheItem& item, bool adding) {
    size_t item_size = calculate_item_memory_size(item);
    size_t string_mem = calculate_string_memory(item);

    if (adding) {
        metrics_.update_memory_usage(static_cast<ssize_t>(item_size));
        metrics_.update_string_memory(static_cast<ssize_t>(string_mem));
        metrics_.update_item_count(1);
    } else {
        metrics_.update_memory_usage(-static_cast<ssize_t>(item_size));
        metrics_.update_string_memory(-static_cast<ssize_t>(string_mem));
        metrics_.update_item_count(-1);
    }
}

void LRUCache::put_internal(int key, CacheItem value) {
    size_t new_item_size = calculate_item_memory_size(value);

    auto old_it = cache_.find(key);
    if (old_it != cache_.end()) {
        size_t old_size = calculate_item_memory_size(old_it->second.second.item);
        metrics_.update_memory_usage(-static_cast<ssize_t>(old_size));
        lru_list_.erase(old_it->second.first);
        cache_.erase(old_it);
    }

    evict_if_needed();

    CacheEntry entry{std::move(value), std::chrono::steady_clock::now()};
    lru_list_.push_front(entry);
    cache_[key] = {lru_list_.begin(), entry};
    metrics_.update_memory_usage(static_cast<ssize_t>(new_item_size));
}

size_t LRUCache::calculate_actual_memory_usage() const {
    size_t total = 0;
    for (const auto& pair : cache_) {
        total += calculate_item_memory_size(pair.second.second.item);
    }
    return total;
}

void LRUCache::validate_metrics() {
    std::lock_guard<std::mutex> lock(mutex_);

    size_t actual_memory = calculate_actual_memory_usage();
    size_t reported_memory = metrics_.get_memory_usage();

    if (actual_memory != reported_memory) {
        ssize_t correction = static_cast<ssize_t>(actual_memory) -
                           static_cast<ssize_t>(reported_memory);
        metrics_.update_memory_usage(correction);
    }

    size_t actual_count = cache_.size();
    metrics_.update_item_count(
        static_cast<ssize_t>(actual_count) -
        static_cast<ssize_t>(metrics_.get_item_count()));
}

void LRUCache::put(int key, CacheItem value) {
    auto start = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);

    auto old_it = cache_.find(key);
    if (old_it != cache_.end()) {
        update_metrics_for_item(old_it->second.second.item, false);
        lru_list_.erase(old_it->second.first);
        cache_.erase(old_it);
    }

    while (cache_.size() >= capacity_) {
        if (!lru_list_.empty()) {
            auto last = std::prev(lru_list_.end());
            auto it = std::find_if(cache_.begin(), cache_.end(),
                [&](const auto& pair) { return pair.second.first == last; });

            if (it != cache_.end()) {
                update_metrics_for_item(it->second.second.item, false);
                cache_.erase(it);
            }
            lru_list_.pop_back();
        }
    }

    CacheEntry entry{std::move(value), std::chrono::steady_clock::now()};
    update_metrics_for_item(entry.item, true);

    lru_list_.push_front(entry);
    cache_[key] = {lru_list_.begin(), entry};

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

    metrics_.update_memory_usage(-static_cast<ssize_t>(metrics_.get_memory_usage()));
    metrics_.update_string_memory(-static_cast<ssize_t>(metrics_.get_string_memory()));
    metrics_.update_item_count(-static_cast<ssize_t>(metrics_.get_item_count()));

    metrics_.reset_counters();
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
            put_internal(key, value);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load cache: " + std::string(e.what()));
    }
}
