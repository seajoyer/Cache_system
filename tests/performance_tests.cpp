#include <iostream>
#include <chrono>
#include "../include/cache.hpp"

void testPerformancePutAndGet() {
    Cache cache(std::chrono::seconds(60));

    const int num_elements = 100000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_elements; ++i) {
        cache.put("key" + std::to_string(i), {{"value", i}});
    }

    auto mid = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_elements; ++i) {
        auto result = cache.get("key" + std::to_string(i));
        if (!result || result->at("value").get<int>() != i) {
            std::cerr << "Data mismatch at key" << i << "\n";
            std::exit(1);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Put time: " << std::chrono::duration_cast<std::chrono::milliseconds>(mid - start).count() << " ms\n";
    std::cout << "Get time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - mid).count() << " ms\n";
}

void testMemoryUsage() {
    Cache cache(std::chrono::seconds(60));

    const int num_elements = 100000;

    for (int i = 0; i < num_elements; ++i) {
        cache.put("key" + std::to_string(i), {{"value", i}});
    }

    std::cout << "Cache size after inserting " << num_elements << " elements: " << cache.size() << "\n";
    // Для оценки памяти используйте инструменты ОС (например, top, htop).
}

int main() {
    std::cout << "Running performance tests...\n";

    testPerformancePutAndGet();
    std::cout << "Performance put/get test passed\n";

    testMemoryUsage();
    std::cout << "Memory usage test completed\n";

    return 0;
}
