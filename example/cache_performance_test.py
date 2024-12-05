#!/usr/bin/env python3
import time
import random
import string
import statistics
import gc
from typing import List, Dict
import psutil
import matplotlib.pyplot as plt
from cache_system import LRUCache, CacheItem

class CachePerformanceTester:
    def __init__(self, cache_sizes: List[int]):
        self.cache_sizes = cache_sizes
        self.results: Dict[str, Dict[int, List[float]]] = {
            'write_times': {},
            'read_times': {},
            'memory_usage': {},
            'hit_rates': {}
        }

    def generate_random_item(self, item_id: int) -> CacheItem:
        item = CacheItem()
        item.id = item_id
        item.faculty = ''.join(random.choices(string.ascii_letters, k=10))
        item.course = ''.join(random.choices(string.ascii_letters, k=10))
        item.title = ''.join(random.choices(string.ascii_letters, k=20))
        item.description = ''.join(random.choices(string.ascii_letters, k=50))
        item.votes_count = random.randint(0, 1000)
        item.telegram_group_link = f"t.me/group_{random.randint(1000, 9999)}"
        item.user_id = random.randint(1, 10000)
        return item

    def measure_memory(self, cache: LRUCache) -> float:
        process = psutil.Process()
        gc.collect()  # Force garbage collection
        return process.memory_info().rss / 1024 / 1024  # Convert to MB

    def run_performance_test(self, cache_size: int, operations: int = 10000):
        cache = LRUCache(cache_size)
        write_times = []
        read_times = []
        hits = 0
        total_reads = 0

        # Measure initial memory
        initial_memory = self.measure_memory(cache)

        # Write test
        for i in range(operations):
            item = self.generate_random_item(i)
            start_time = time.perf_counter()
            cache.put(i, item)
            write_times.append(time.perf_counter() - start_time)

        # Read test with both hits and misses
        for _ in range(operations):
            key = random.randint(0, operations * 2)  # Deliberately include misses
            start_time = time.perf_counter()
            result = cache.get(key)
            read_times.append(time.perf_counter() - start_time)
            if result is not None:
                hits += 1
            total_reads += 1

        # Measure final memory
        final_memory = self.measure_memory(cache)
        memory_usage = final_memory - initial_memory

        # Calculate hit rate
        hit_rate = (hits / total_reads) * 100 if total_reads > 0 else 0

        # Store results
        self.results['write_times'][cache_size] = write_times
        self.results['read_times'][cache_size] = read_times
        self.results['memory_usage'][cache_size] = memory_usage
        self.results['hit_rates'][cache_size] = hit_rate

    def run_all_tests(self):
        for size in self.cache_sizes:
            print(f"Testing cache size: {size}")
            self.run_performance_test(size)

    def plot_results(self):
        plt.figure(figsize=(15, 10))

        # Plot 1: Operation Times
        plt.subplot(2, 2, 1)
        for size in self.cache_sizes:
            plt.boxplot(self.results['write_times'][size], positions=[size], widths=size/10)
        plt.title('Write Operation Times')
        plt.xlabel('Cache Size')
        plt.ylabel('Time (seconds)')
        plt.xscale('log')
        plt.yscale('log')

        # Plot 2: Read Times
        plt.subplot(2, 2, 2)
        for size in self.cache_sizes:
            plt.boxplot(self.results['read_times'][size], positions=[size], widths=size/10)
        plt.title('Read Operation Times')
        plt.xlabel('Cache Size')
        plt.ylabel('Time (seconds)')
        plt.xscale('log')
        plt.yscale('log')

        # Plot 3: Memory Usage
        plt.subplot(2, 2, 3)
        sizes = list(self.results['memory_usage'].keys())
        memory = list(self.results['memory_usage'].values())
        plt.plot(sizes, memory, 'bo-')
        plt.title('Memory Usage')
        plt.xlabel('Cache Size')
        plt.ylabel('Memory Usage (MB)')
        plt.xscale('log')

        # Plot 4: Hit Rates
        plt.subplot(2, 2, 4)
        sizes = list(self.results['hit_rates'].keys())
        rates = list(self.results['hit_rates'].values())
        plt.plot(sizes, rates, 'ro-')
        plt.title('Cache Hit Rates')
        plt.xlabel('Cache Size')
        plt.ylabel('Hit Rate (%)')
        plt.xscale('log')

        plt.tight_layout()
        plt.savefig('cache_performance_results.png')
        plt.close()

def main():
    # Test with different cache sizes
    cache_sizes = [100, 1000, 10000, 100000]
    tester = CachePerformanceTester(cache_sizes)

    print("Starting performance tests...")
    tester.run_all_tests()

    print("\nGenerating performance report...")
    tester.plot_results()

    print("\nPerformance Summary:")
    for size in cache_sizes:
        print(f"\nCache Size: {size}")
        print(f"Average Write Time: {statistics.mean(tester.results['write_times'][size]):.6f} seconds")
        print(f"Average Read Time: {statistics.mean(tester.results['read_times'][size]):.6f} seconds")
        print(f"Memory Usage: {tester.results['memory_usage'][size]:.2f} MB")
        print(f"Hit Rate: {tester.results['hit_rates'][size]:.2f}%")

if __name__ == "__main__":
    main()
