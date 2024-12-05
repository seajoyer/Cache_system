#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"
#include "cache/lru_cache.hpp"
#include <thread>
#include <atomic>
#include <fstream>

TEST_CASE("Cache basic operations", "[cache]") {
    LRUCache cache(3);

    CacheItem item1{1, "CS", "Algorithms", "Title1", "Desc1", 10, "t.me/group1", 100};
    CacheItem item2{2, "Math", "Calculus", "Title2", "Desc2", 20, "t.me/group2", 200};

    SECTION("Adding and retrieving items") {
        cache.put(1, item1);
        cache.put(2, item2);

        auto result1 = cache.get(1);
        REQUIRE(result1.has_value());
        CHECK(result1->id == 1);
        CHECK(result1->faculty == "CS");
    }

    SECTION("Removing items") {
        cache.put(1, item1);
        cache.remove(1);
        auto result = cache.get(1);
        REQUIRE_FALSE(result.has_value());
    }
}

TEST_CASE("LRU eviction strategy", "[cache]") {
    LRUCache cache(2);

    CacheItem item1{1, "CS", "Algorithms", "Title1", "Desc1", 10, "t.me/group1", 100};
    CacheItem item2{2, "Math", "Calculus", "Title2", "Desc2", 20, "t.me/group2", 200};
    CacheItem item3{3, "Physics", "Mechanics", "Title3", "Desc3", 30, "t.me/group3", 300};

    cache.put(1, item1);
    cache.put(2, item2);
    REQUIRE(cache.get(1).has_value());

    cache.put(3, item3);  // Should evict item2 since item1 was just accessed
    REQUIRE(cache.get(1).has_value());
    REQUIRE_FALSE(cache.get(2).has_value());
    REQUIRE(cache.get(3).has_value());
}

TEST_CASE("Thread safety", "[cache]") {
    LRUCache cache(100);
    std::vector<std::thread> threads;
    std::atomic<int> completed{0};
    const int thread_count = 10;

    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&cache, i, &completed]() {
            try {
                CacheItem item{i, "Faculty" + std::to_string(i), "Course", "Title",
                             "Description", 0, "t.me/group", 100 + i};
                cache.put(i, item);

                auto result = cache.get(i);
                if (result.has_value() && result->id == i) {
                    completed++;
                }
            } catch (const std::exception& e) {
                // Log error but don't fail the test here
                INFO("Thread " << i << " failed: " << e.what());
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    CHECK(completed.load() == thread_count);
}

TEST_CASE("Cache serialization", "[cache]") {
    const std::string test_file = "test_cache.json";

    // Clean up any existing test file
    std::remove(test_file.c_str());

    SECTION("Save and load cache") {
        // Save cache to file
        {
            LRUCache cache(2);
            CacheItem item{1, "CS", "Algorithms", "Title1", "Desc1", 10, "t.me/group1", 100};
            INFO("Saving cache to file");
            cache.put(1, item);
            REQUIRE_NOTHROW(cache.save_to_file(test_file));

            // Verify file exists and is readable
            std::ifstream verify_file(test_file);
            REQUIRE(verify_file.good());
            verify_file.close();
        }

        // Load cache from file
        {
            INFO("Loading cache from file");
            LRUCache new_cache(2);
            REQUIRE_NOTHROW(new_cache.load_from_file(test_file));

            auto result = new_cache.get(1);
            REQUIRE(result.has_value());
            CHECK(result->faculty == "CS");
            CHECK(result->course == "Algorithms");
        }
    }

    // Clean up test file
    std::remove(test_file.c_str());
}
