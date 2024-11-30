#include <iostream>
#include <cassert>
#include <thread>
#include <filesystem>
#include "../include/cache.hpp"

// Утилита для проверки равенства с выводом сообщения
template <typename T>
void assertEqual(const T& a, const T& b, const std::string& message = "") {
    if (a != b) {
        std::cerr << "Assertion failed: " << message << "\n";
        std::cerr << "Expected: " << b << ", Got: " << a << "\n";
        std::exit(1);
    }
}

void testBasicOperations() {
    Cache cache(std::chrono::seconds(5));
    cache.put("key1", {{"value", 123}});
    cache.put("key2", {{"value", 456}});

    auto result1 = cache.get("key1");
    auto result2 = cache.get("key2");
    auto result3 = cache.get("key3"); // Должно вернуть пустое значение

    assert(result1.has_value());
    assert(result2.has_value());
    assert(!result3.has_value());

    assertEqual(result1.value()["value"].get<int>(), 123, "Key1 value mismatch");
    assertEqual(result2.value()["value"].get<int>(), 456, "Key2 value mismatch");
}

void testTTL() {
    Cache cache(std::chrono::seconds(3));
    cache.put("key1", {{"value", 123}});

    std::this_thread::sleep_for(std::chrono::seconds(4));
    auto result = cache.get("key1");

    assert(!result.has_value()); // Данные должны устареть
}

void testThreadSafety() {
    Cache cache(std::chrono::seconds(5));

    auto writer = [&cache]() {
        for (int i = 0; i < 1000; ++i) {
            cache.put("key" + std::to_string(i), {{"value", i}});
        }
    };

    auto reader = [&cache]() {
        for (int i = 0; i < 1000; ++i) {
            cache.get("key" + std::to_string(i));
        }
    };

    std::thread writer_thread(writer);
    std::thread reader_thread(reader);

    writer_thread.join();
    reader_thread.join();

    assertEqual(cache.size(), static_cast<size_t>(1000), "Thread safety test size mismatch");
}

void testPersistence() {

    Cache cache(std::chrono::seconds(5));
    cache.put("key1", {{"value", 123}});
    cache.put("key2", {{"value", 456}});

    cache.saveToFile("../data/cache_test.json");

    Cache new_cache(std::chrono::seconds(5));
    new_cache.loadFromFile("../data/cache_test.json");

    auto result1 = new_cache.get("key1");
    auto result2 = new_cache.get("key2");

    assert(result1.has_value());
    assert(result2.has_value());

    assertEqual(result1.value()["value"].get<int>(), 123, "Persistence key1 value mismatch");
    assertEqual(result2.value()["value"].get<int>(), 456, "Persistence key2 value mismatch");
}

int main() {
    std::cout << "Running tests...\n";

    testBasicOperations();
    std::cout << "TestBasicOperations passed\n";

    testTTL();
    std::cout << "TestTTL passed\n";

    testThreadSafety();
    std::cout << "TestThreadSafety passed\n";

    testPersistence();
    std::cout << "TestPersistence passed\n";

    std::cout << "All tests passed successfully!\n";
    return 0;
}
