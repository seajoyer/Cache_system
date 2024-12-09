import time
import random
from concurrent.futures import ThreadPoolExecutor
from cache_system import LRUCache, CacheItem

class CachePerformanceTester:
    def __init__(self, cache_size=10000):
        """Initialize the cache tester with a specific cache size."""
        self.cache = LRUCache(cache_size)
        self.cache_size = cache_size

    def create_test_item(self, id_num):
        """Create a test cache item with consistent test data."""
        description = f"Test description for item {id_num} " * 10
        faculty = f"Faculty_{id_num % 5}"
        course = f"Course_{id_num % 10}"

        return CacheItem(
            id_num,
            faculty,
            course,
            f"Title_{id_num}",
            description,
            random.randint(0, 1000),
            f"t.me/group_{id_num}",
            random.randint(1000, 9999)
        )

    def print_test_header(self, test_name, total_operations):
        """Print a formatted header for each test section."""
        print("\n" + "=" * 40)
        print(f"TEST: {test_name}")
        print(f"Cache Size: {self.cache_size:,} items")
        print(f"Total Operations: {total_operations:,}")
        print("=" * 40)

    def print_metrics(self, operation="Current", progress=None):
        """Print current cache metrics in a clear format."""
        metrics = self.cache.get_metrics()

        print(f"\n{operation} Metrics:")
        if progress is not None:
            print(f"Progress: {progress:.1f}%")

        print(f"Memory Usage: {metrics.memory_usage / (1024 * 1024):.2f} MB")
        print(f">> String Memory: {metrics.string_memory / (1024 * 1024):.2f} MB "
              f"({(metrics.string_memory / metrics.memory_usage * 100):.1f}% of total)")
        print(f"Average Item Size: {metrics.average_item_size / 1024:.2f} KB")
        print(f"Hit Rate: {metrics.hit_rate:.2f}%")
        print(f"Operation Times:")
        print(f">> Read: {metrics.avg_read_time / 1e6:.3f} ms")
        print(f">> Write: {metrics.avg_write_time / 1e6:.3f} ms")

    def test_basic_operations(self, num_operations=100000):
        """Test basic cache operations with clear progress reporting."""
        self.print_test_header("Basic Operations", num_operations)

        print("\nPhase 1: Write Operations")
        for i in range(num_operations):
            if (i + 1) % (num_operations // 4) == 0:
                self.print_metrics("Write", (i + 1) / num_operations * 100)
            self.cache.put(i, self.create_test_item(i))

        print("\nPhase 2: Read Operations (2x write count)")
        num_reads = num_operations * 2
        for i in range(num_reads):
            if (i + 1) % (num_reads // 4) == 0:
                self.print_metrics("Read", (i + 1) / num_reads * 100)

            window_size = min(1000, self.cache_size)
            current_position = min(i, num_operations - 1)  # Don't exceed our written data
            start = max(0, current_position - window_size)
            end = current_position

            if start <= end:
                key = random.randint(start, end)
            else:
                key = random.randint(0, num_operations - 1)
            self.cache.get(key)

        self.print_metrics("Final Basic Operations")

    def test_concurrent_access(self, num_threads=20, operations_per_thread=50000):
        """Test cache under concurrent access with clear metrics."""
        total_operations = num_threads * operations_per_thread
        self.print_test_header("Concurrent Access", total_operations)

        def worker(worker_id):
            ops = {'writes': 0, 'reads': 0, 'hits': 0}
            for i in range(operations_per_thread):
                key = (worker_id * operations_per_thread + i) % (operations_per_thread * 2)

                if random.random() < 0.4:  # 40% reads
                    result = self.cache.get(key)
                    ops['reads'] += 1
                    if result is not None:
                        ops['hits'] += 1
                else:  # 60% writes
                    item = self.create_test_item(key)
                    self.cache.put(key, item)
                    ops['writes'] += 1

                if (i + 1) % (operations_per_thread // 2) == 0:
                    print(f"Thread {worker_id}: {((i + 1)/operations_per_thread)*100:.1f}% complete")
            return ops

        start_time = time.perf_counter()
        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            results = list(executor.map(worker, range(num_threads)))
        total_time = time.perf_counter() - start_time

        # Summarize results
        total_ops = sum(r['reads'] + r['writes'] for r in results)
        print(f"\nConcurrent Test Summary:")
        print(f"Total Time: {total_time:.2f} seconds")
        print(f"Operations/second: {total_ops/total_time:.2f}")

        self.print_metrics("Final Concurrent Operations")

def main():
    CACHE_SIZE = 10000
    BASIC_OPS = 100000
    CONCURRENT_THREADS = 20
    OPS_PER_THREAD = 50000

    try:
        tester = CachePerformanceTester(cache_size=CACHE_SIZE)

        tester.test_basic_operations(num_operations=BASIC_OPS)

        tester.cache.clear()

        tester.test_concurrent_access(
            num_threads=CONCURRENT_THREADS,
            operations_per_thread=OPS_PER_THREAD
        )

    except Exception as e:
        print(f"Test failed with error: {e}")
        raise

if __name__ == "__main__":
    main()
