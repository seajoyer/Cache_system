#include "lru_strategy.hpp"
#include <optional>
#include <stdexcept>

LRUCache::LRUCache(size_t capacity) : _capacity(capacity) {
    if (capacity == 0) {
        throw std::invalid_argument("Capacity must be greater than zero");
    }
}

std::optional<std::string> LRUCache::get(const std::string& key) {
    auto it = _cache_map.find(key);
    if (it == _cache_map.end()) {
        return std::nullopt;
    }
    moveToFront(key);
    return it->second->second;
}

void LRUCache::put(const std::string& key, const std::string& value) {
    auto it = _cache_map.find(key);
    if (it != _cache_map.end()) {
        it->second->second = value;
        moveToFront(key);
        return;
    }
    if (_items.size() >= _capacity) {
        auto lru = _items.back();
        _cache_map.erase(lru.first);
        _items.pop_back();
    }
    _items.emplace_front(key, value);
    _cache_map[key] = _items.begin();
}

size_t LRUCache::size() const {
    return _items.size();
}

void LRUCache::clear() {
    _items.clear();
    _cache_map.clear();
}

void LRUCache::moveToFront(const std::string& key) {
    auto it = _cache_map[key];
    _items.splice(_items.begin(), _items, it);
}

void LRUCache::remove(const std::string& key) {
    auto it = _cache_map.find(key);
    if (it != _cache_map.end()) {
        _items.erase(it->second); // Удаляем элемент из списка
        _cache_map.erase(it);     // Удаляем ключ из карты
    }
}
