#include <gtest/gtest.h>
#include "cache.hpp"

// Test fixture for Cache tests
class CacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        cache = createCache(3); // Create a cache with capacity 3
    }

    std::unique_ptr<Cache> cache;
};

// Test case for adding and retrieving items
TEST_F(CacheTest, PutAndGet) {
    cache->put("key1", "value1");
    cache->put("key2", "value2");

    EXPECT_EQ(cache->get("key1").value(), "value1");
    EXPECT_EQ(cache->get("key2").value(), "value2");
}

// Test case for LRU eviction policy
TEST_F(CacheTest, LRUReplacement) {
    cache->put("key1", "value1");
    cache->put("key2", "value2");
    cache->put("key3", "value3");

    // Access key1 to make it recently used
    EXPECT_EQ(cache->get("key1").value(), "value1");

    // Add a new item, which should evict the least recently used item (key2)
    cache->put("key4", "value4");

    EXPECT_FALSE(cache->get("key2").has_value());
    EXPECT_EQ(cache->get("key1").value(), "value1");
}

// Test case for removing an item
TEST_F(CacheTest, RemoveItem) {
    cache->put("key1", "value1");
    cache->remove("key1");

    EXPECT_FALSE(cache->get("key1").has_value());
}

// Test case for clearing the cache
TEST_F(CacheTest, ClearCache) {
    cache->put("key1", "value1");
    cache->put("key2", "value2");
    cache->clear();

    EXPECT_EQ(cache->size(), 0);
    EXPECT_FALSE(cache->get("key1").has_value());
}

// Test case for size reporting
TEST_F(CacheTest, CacheSize) {
    cache->put("key1", "value1");
    cache->put("key2", "value2");

    EXPECT_EQ(cache->size(), 2);

    cache->put("key3", "value3");
    EXPECT_EQ(cache->size(), 3);
}
