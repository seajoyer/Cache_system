#ifndef CACHE_HPP
#define CACHE_HPP

#include <string>
#include <unordered_map>
#include <optional>
#include <nlohmann/json.hpp>
#include <chrono>
#include <mutex>
#include <thread>

// Упрощение доступа к JSON
using json = nlohmann::json;

// Структура данных для кэшируемого элемента
struct CacheItem {
    json data;
    std::chrono::steady_clock::time_point expiry;
};

// Основной класс Cache
class Cache {
public:
    // Конструктор с указанием времени TTL
    explicit Cache(std::chrono::seconds default_ttl);

    // Деструктор
    ~Cache();

    // Основные методы
    void put(const std::string& key, const json& value);
    std::optional<json> get(const std::string& key);
    void remove(const std::string& key);
    void clear();
    size_t size();

    // Методы для персистентности
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);

private:
    // Внутреннее хранилище
    std::unordered_map<std::string, CacheItem> cache;
    std::mutex cache_mutex;
    std::chrono::seconds ttl;

    // Методы для работы с устаревшими данными
    bool isExpired(const CacheItem& item) const;
    void cleanupExpiredItems();

    // Поток очистки
    std::thread cleanup_thread;
    bool stop_flag = false;
    void startCleanupThread();
    void stopCleanupThread();
};

#endif // CACHE_HPP
