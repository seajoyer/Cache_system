#pragma once

#include <list>
#include <unordered_map>
#include <string>
#include <optional>

class LRUCache {
public:
    explicit LRUCache(size_t capacity);
    ~LRUCache() = default;

    std::optional<std::string> get(const std::string& key);
    void put(const std::string& key, const std::string& value);
    void remove(const std::string& key); // Переместили в public
    size_t size() const;
    void clear();

private:
    using ListIterator = std::list<std::pair<std::string, std::string>>::iterator;

    size_t _capacity;
    std::list<std::pair<std::string, std::string>> _items;
    std::unordered_map<std::string, ListIterator> _cache_map;

    void moveToFront(const std::string& key);
};
