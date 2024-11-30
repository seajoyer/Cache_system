#include "cache.hpp"
#include "lru_strategy.hpp"

class CacheImpl : public Cache {
public:
    explicit CacheImpl(size_t capacity) : _lruCache(capacity) {}

    std::optional<std::string> get(const std::string& key) override {
        return _lruCache.get(key);
    }

    void put(const std::string& key, const std::string& value) override {
        _lruCache.put(key, value);
    }

    void remove(const std::string& key) override {
        _lruCache.remove(key);
    }

    void clear() override { _lruCache.clear(); }

    size_t size() const override {
        return _lruCache.size();
    }

private:
    LRUCache _lruCache;
};

// Factory method for creating a Cache instance
std::unique_ptr<Cache> createCache(size_t capacity) {
    return std::make_unique<CacheImpl>(capacity);
}
