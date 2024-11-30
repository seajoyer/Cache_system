#include "cache.hpp"
#include <fstream>
#include <iostream>

// Конструктор
Cache::Cache(std::chrono::seconds default_ttl) : ttl(default_ttl) {
    startCleanupThread();
}

// Деструктор
Cache::~Cache() {
    stopCleanupThread();
}

// Добавление или обновление данных
void Cache::put(const std::string& key, const json& value) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    cache[key] = {value, std::chrono::steady_clock::now() + ttl};
}

// Получение данных по ключу
std::optional<json> Cache::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = cache.find(key);
    if (it != cache.end() && !isExpired(it->second)) {
        return it->second.data;
    }
    return std::nullopt;
}

// Удаление данных по ключу
void Cache::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    cache.erase(key);
}

// Полная очистка кэша
void Cache::clear() {
    std::lock_guard<std::mutex> lock(cache_mutex);
    cache.clear();
}

// Текущий размер кэша
size_t Cache::size() {
    std::lock_guard<std::mutex> lock(cache_mutex);
    return cache.size();
}

// Сохранение кэша в файл
void Cache::saveToFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    json j;
    for (const auto& [key, item] : cache) {
        if (!isExpired(item)) {
            j[key] = item.data;
        }
    }

    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file for saving: " + filename);
    }
    file << j.dump(4);
}

// Загрузка кэша из файла
void Cache::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file for loading: " + filename);
    }

    json j;
    file >> j;

    std::lock_guard<std::mutex> lock(cache_mutex);
    for (const auto& [key, value] : j.items()) {
        cache[key] = {value, std::chrono::steady_clock::now() + ttl};
    }
}

// Проверка устаревших данных
bool Cache::isExpired(const CacheItem& item) const {
    return std::chrono::steady_clock::now() > item.expiry;
}

// Поток очистки устаревших данных
void Cache::startCleanupThread() {
    cleanup_thread = std::thread([this]() {
        while (!stop_flag) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            cleanupExpiredItems();
        }
    });
}

void Cache::stopCleanupThread() {
    stop_flag = true;
    if (cleanup_thread.joinable()) {
        cleanup_thread.join();
    }
}

// Удаление устаревших элементов
void Cache::cleanupExpiredItems() {
    std::lock_guard<std::mutex> lock(cache_mutex);
    for (auto it = cache.begin(); it != cache.end();) {
        if (isExpired(it->second)) {
            it = cache.erase(it);
        } else {
            ++it;
        }
    }
}
