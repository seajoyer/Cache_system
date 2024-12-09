import time
import random
import gc
from concurrent.futures import ThreadPoolExecutor
import psutil
import matplotlib.pyplot as plt
from cache_system import LRUCache, CacheItem

class CachePerformanceTester:
    def __init__(self, cache_size=10000):
        self.cache = LRUCache(cache_size)
        self.process = psutil.Process()
        self.verbose = False

    def create_test_item(self, id_num):
        """Create a test cache item with randomized but consistent test data."""
        description = f"Test description for item {id_num} " * 10
        faculty = f"Faculty_{id_num % 5}"  # Limit faculties to increase hit rate
        course = f"Course_{id_num % 10}"   # Limit courses to increase hit rate

        # Using positional arguments instead of keyword arguments
        return CacheItem(
            id_num,                     # id
            faculty,                    # faculty
            course,                     # course
            f"Title_{id_num}",          # title
            description,                # description
            random.randint(0, 1000),    # votes_count
            f"t.me/group_{id_num}",     # telegram_group_link
            random.randint(1000, 9999)  # user_id
        )

    def measure_basic_operations(self, num_operations=100000):
        """Measure basic cache operations performance."""
        print(f"\nTesting with {num_operations:,} operations")
        write_times = []
        read_times = []
        validation_success = 0

        # Create and insert items
        print("\nTesting write performance...")
        batch_size = num_operations // 20
        for i in range(num_operations):
            if i % batch_size == 0:
                print(f"Written {i:,} items ({(i/num_operations)*100:.1f}%)...")

            item = self.create_test_item(i)
            start_time = time.perf_counter()
            self.cache.put(i, item)
            write_times.append(time.perf_counter() - start_time)

            # Verify write
            result = self.cache.get(i)
            if result is not None:
                validation_success += 1

        # Test read performance
        print("\nTesting read performance...")
        num_reads = num_operations * 2  # Double the reads to test cache hits
        for i in range(num_reads):
            if i % batch_size == 0:
                print(f"Read progress: {(i/num_reads)*100:.1f}%...")

            # Read existing items with some locality of reference
            # Generate a key that's guaranteed to be within our valid range
            start = max(0, i - 1000)
            end = min(i, num_operations - 1)
            key = random.randint(start, end) if start <= end else 0
            start_time = time.perf_counter()
            self.cache.get(key)
            read_times.append(time.perf_counter() - start_time)

        metrics = self.cache.get_metrics()

        return {
            'avg_write_time': sum(write_times) / len(write_times) * 1000,
            'avg_read_time': sum(read_times) / len(read_times) * 1000,
            'cpp_avg_write_time': metrics.get_avg_write_time() / 1e6,
            'cpp_avg_read_time': metrics.get_avg_read_time() / 1e6,
            'validation_success_rate': (validation_success / num_operations) * 100,
            'cache_hit_rate': metrics.get_hit_rate(),
            'cache_size': self.cache.size()
        }

    def measure_memory_usage(self, num_items=100000):
        """Measure memory usage as items are added to the cache."""
        memory_usage = []
        base_memory = self.process.memory_info().rss
        print(f"\nBase memory usage: {base_memory / (1024 * 1024):.2f} MB")

        batch_size = num_items // 20
        for i in range(0, num_items, batch_size):
            if i % batch_size == 0:
                print(f"Added {i:,} items ({(i/num_items)*100:.1f}%)...")

            # Add batch_size items
            for j in range(batch_size):
                if i + j < num_items:
                    item = self.create_test_item(i + j)
                    self.cache.put(i + j, item)

            # Force garbage collection before measuring memory
            gc.collect()
            current_memory = self.process.memory_info().rss - base_memory
            memory_usage.append((i + batch_size, current_memory))

            # Log memory metrics
            cpp_memory = self.cache.get_metrics().get_memory_usage()
            print(f"Python measured memory: {current_memory / (1024 * 1024):.2f} MB")
            print(f"C++ reported memory: {cpp_memory / (1024 * 1024):.2f} MB")
            print(f"Current cache size: {self.cache.size()}")

        return memory_usage

    def test_concurrent_access(self, num_threads=20, operations_per_thread=50000):
        """Test cache performance under concurrent access."""
        start_time = time.perf_counter()

        def worker(worker_id):
            stats = {
                'writes': 0,
                'reads': 0,
                'hits': 0,
                'errors': []
            }

            try:
                for i in range(operations_per_thread):
                    if i % (operations_per_thread // 10) == 0:
                        print(f"Thread {worker_id}: {(i/operations_per_thread)*100:.1f}% complete")

                    # Use modulo to create key locality and increase hit rate
                    key = (worker_id * operations_per_thread + i) % (operations_per_thread * 2)

                    if random.random() < 0.4:  # 40% reads, 60% writes
                        result = self.cache.get(key)
                        stats['reads'] += 1
                        if result is not None:
                            stats['hits'] += 1
                    else:
                        item = self.create_test_item(key)
                        self.cache.put(key, item)
                        stats['writes'] += 1

            except Exception as e:
                stats['errors'].append(str(e))

            return stats

        print(f"\nStarting concurrent test with {num_threads} threads...")
        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            results = list(executor.map(worker, range(num_threads)))

        total_time = time.perf_counter() - start_time

        # Aggregate results
        total_operations = sum(r['reads'] + r['writes'] for r in results)
        total_reads = sum(r['reads'] for r in results)
        total_hits = sum(r['hits'] for r in results)
        all_errors = [e for r in results for e in r['errors']]

        metrics = self.cache.get_metrics()

        return {
            'operations_per_second': total_operations / total_time,
            'total_operations': total_operations,
            'read_hit_rate': (total_hits / total_reads * 100) if total_reads > 0 else 0,
            'cache_hit_rate': metrics.get_hit_rate(),
            'error_rate': (len(all_errors) / total_operations * 100) if total_operations > 0 else 0,
            'final_cache_size': self.cache.size(),
            'total_time': total_time
        }

    def plot_results(self, memory_usage):
        """Plot memory usage results."""
        if not memory_usage:
            print("No memory usage data to plot")
            return

        items, memory = zip(*memory_usage)
        python_memory_mb = [m / (1024 * 1024) for m in memory]

        plt.figure(figsize=(12, 6))
        plt.plot([i/1000 for i in items], python_memory_mb,
                marker='o', linestyle='-', label='Memory Usage')
        plt.xlabel('Number of Items (thousands)')
        plt.ylabel('Memory Usage (MB)')
        plt.title('Cache Memory Usage vs Number of Items')
        plt.grid(True)
        plt.legend()
        plt.savefig('cache_memory_usage.png')
        plt.close()

def main():
    # Configure smaller test sizes for faster iteration
    CACHE_SIZE = 10000
    BASIC_OPS = 100000
    MEMORY_TEST_ITEMS = 100000
    CONCURRENT_THREADS = 20
    OPS_PER_THREAD = 50000

    try:
        print("\n=== Cache Performance Test ===")
        tester = CachePerformanceTester(cache_size=CACHE_SIZE)

        # Test basic operations
        print("\n=== Basic Operations Test ===")
        basic_metrics = tester.measure_basic_operations(num_operations=BASIC_OPS)
        print(f"\nBasic Operations Results:")
        print(f"Average write time (Python): {basic_metrics['avg_write_time']:.3f} ms")
        print(f"Average read time (Python): {basic_metrics['avg_read_time']:.3f} ms")
        print(f"Average write time (C++): {basic_metrics['cpp_avg_write_time']:.3f} ms")
        print(f"Average read time (C++): {basic_metrics['cpp_avg_read_time']:.3f} ms")
        print(f"Cache hit rate: {basic_metrics['cache_hit_rate']:.2f}%")
        print(f"Final cache size: {basic_metrics['cache_size']}")

        # Test memory usage
        print("\n=== Memory Usage Test ===")
        memory_usage = tester.measure_memory_usage(num_items=MEMORY_TEST_ITEMS)
        if memory_usage:
            tester.plot_results(memory_usage)

        # Test concurrent access
        print("\n=== Concurrent Access Test ===")
        concurrent_metrics = tester.test_concurrent_access(
            num_threads=CONCURRENT_THREADS,
            operations_per_thread=OPS_PER_THREAD
        )
        print("\nConcurrent Test Results:")
        print(f"Operations per second: {concurrent_metrics['operations_per_second']:.2f}")
        print(f"Total operations: {concurrent_metrics['total_operations']:,}")
        print(f"Cache hit rate: {concurrent_metrics['cache_hit_rate']:.2f}%")
        print(f"Read hit rate: {concurrent_metrics['read_hit_rate']:.2f}%")
        print(f"Error rate: {concurrent_metrics['error_rate']:.2f}%")
        print(f"Final cache size: {concurrent_metrics['final_cache_size']}")
        print(f"Total time: {concurrent_metrics['total_time']:.2f} seconds")

    except Exception as e:
        print(f"Test failed with error: {e}")
        raise

if __name__ == "__main__":
    main()
