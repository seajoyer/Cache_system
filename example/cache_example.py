from cache_system import LRUCache, CacheItem

def example_usage():
    # Create cache with capacity of 1000 items
    cache = LRUCache(1000)

    # Create a cache item
    item = CacheItem()
    item.id = 1
    item.faculty = "Computer Science"
    item.course = "Algorithms"
    item.title = "Graph Theory"
    item.description = "Introduction to graph algorithms"
    item.votes_count = 10
    item.telegram_group_link = "t.me/graph_theory"
    item.user_id = 12345

    # Add item to cache
    cache.put(item.id, item)

    # Retrieve item
    retrieved_item = cache.get(item.id)
    if retrieved_item:
        print(f"Retrieved item: {retrieved_item.title}")

    # Get cache metrics
    metrics = cache.get_metrics()
    print(f"Average read time: {metrics.get_avg_read_time()} ns")
    print(f"Memory usage: {metrics.get_memory_usage()} bytes")

    # Save cache to file
    cache.save_to_file("cache_backup.json")

if __name__ == "__main__":
    example_usage()
