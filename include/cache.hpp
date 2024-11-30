#pragma once

#include <string>
#include <optional>
#include <memory>

class Cache {
public:
    virtual ~Cache() = default;

    virtual std::optional<std::string> get(const std::string& key) = 0;
    virtual void put(const std::string& key, const std::string& value) = 0;
    virtual void remove(const std::string& key) = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
};

// Factory method for creating Cache instances
std::unique_ptr<Cache> createCache(size_t capacity);
