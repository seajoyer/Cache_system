#!/usr/bin/env python3
import time
import random
import threading
from concurrent.futures import ThreadPoolExecutor
import psutil
import matplotlib.pyplot as plt
from cache_system import LRUCache, CacheItem
import sys

class CachePerformanceTester:
    def __init__(self, cache_size=10000):  # Increased to 10000
        print(f"Initializing cache with size: {cache_size}")
        self.cache = LRUCache(cache_size)
        self.process = psutil.Process()

    def create_test_item(self, id_num):
        """Create a test cache item with meaningful test data."""
        try:
            # Create a description with repetitive but meaningful content
            description = f"This is a detailed description for item {id_num}. " * 50  # Creates ~2KB of text

            # Create consistent identifiers that we can validate
            faculty = f"Faculty_{id_num}"  # Keep this simple for validation
            course = f"Course_{id_num}_{'X' * 100}"  # Add padding here instead
            title = f"Title_{id_num}_{'Y' * 100}"    # And here

            item = CacheItem()
            item.id = id_num
            item.faculty = faculty  # Simple format for validation
            item.course = course
            item.title = title
            item.description = description
            item.votes_count = random.randint(0, 1000)
            item.telegram_group_link = f"t.me/group_{id_num}_{'Z' * 100}"
            item.user_id = random.randint(1000, 9999)
            return item
        except Exception as e:
            print(f"Error creating test item: {e}")
            raise

    def validate_item(self, item, expected_id):
        """Validate core properties of a cache item."""
        if item is None:
            return False

        try:
            # Use exact string matching
            expected_faculty = f"Faculty_{expected_id}"
            is_valid = (
                item.id == expected_id and
                item.faculty == expected_faculty and
                item.course.startswith(f"Course_{expected_id}_") and
                item.title.startswith(f"Title_{expected_id}_")
            )

            if not is_valid:
                if self.verbose:
                    print(f"Validation failed for item {expected_id}")
                    if item.id != expected_id:
                        print(f"Expected id: {expected_id}, got: {item.id}")
                    if item.faculty != expected_faculty:
                        print(f"Expected faculty: {expected_faculty}, got: {item.faculty}")
            return is_valid

        except Exception as e:
            if self.verbose:
                print(f"Error during validation of item {expected_id}: {e}")
            return False

    def measure_basic_operations(self, num_operations=1000000):
        """Measure the performance of basic cache operations."""
        print(f"\nTesting with {num_operations:,} operations")
        test_items = []
        write_times = []
        read_times = []
        validation_success = 0
        cache_hits = 0

        # Reduce test data size to ensure items fit in cache
        effective_cache_size = self.cache.size()  # Get actual cache size
        test_range = min(num_operations, effective_cache_size * 2)  # Use smaller range to increase hit probability

        print(f"\nWarming up cache with {effective_cache_size} items...")

        # Create initial test items
        print("\nCreating test items...")
        batch_size = test_range // 20  # Report progress every 5%
        for i in range(test_range):
            if i % batch_size == 0:
                print(f"Created {i:,} items ({(i/test_range)*100:.1f}%)...")
            try:
                item = self.create_test_item(i)
                test_items.append(item)
            except Exception as e:
                print(f"Failed to create item {i}: {e}")
                return None

        # Test write performance
        print("\nTesting write performance...")
        for i, item in enumerate(test_items):
            if i % batch_size == 0:
                print(f"Written {i:,} items ({(i/test_range)*100:.1f}%)...")
            try:
                start_time = time.perf_counter()
                self.cache.put(i, item)
                end_time = time.perf_counter()
                write_times.append(end_time - start_time)

                # Verify write immediately
                result = self.cache.get(i)
                if result is not None and self.validate_item(result, i):
                    validation_success += 1
            except Exception as e:
                print(f"Failed to write item {i}: {e}")

        # Test read performance with repeated access
        print("\nTesting read performance with repeated access...")
        read_iterations = 3  # Multiple passes to increase hit rate
        total_reads = test_range * read_iterations

        for iteration in range(read_iterations):
            for i in range(test_range):
                if i % batch_size == 0:
                    progress = (iteration * test_range + i) / total_reads * 100
                    print(f"Read progress: {progress:.1f}%...")

                try:
                    key = i % effective_cache_size  # Ensure we're hitting cached items
                    start_time = time.perf_counter()
                    result = self.cache.get(key)
                    end_time = time.perf_counter()
                    read_times.append(end_time - start_time)

                    if result is not None and self.validate_item(result, key):
                        cache_hits += 1
                except Exception as e:
                    print(f"Failed to read item {key}: {e}")

        total_time = time.perf_counter()

        # Calculate statistics
        avg_write_time = (sum(write_times) / len(write_times)) * 1000 if write_times else 0  # ms
        avg_read_time = (sum(read_times) / len(read_times)) * 1000 if read_times else 0      # ms
        success_rate = (validation_success / len(test_items)) * 100 if test_items else 0
        read_success_rate = (cache_hits / total_reads) * 100 if total_reads > 0 else 0

        metrics = self.cache.get_metrics()
        cache_hit_rate = metrics.get_hit_rate()

        print(f"\nCache hit rate (from metrics): {cache_hit_rate:.2f}%")
        print(f"Cache hit rate (from test): {read_success_rate:.2f}%")
        print(f"Cache size after operations: {self.cache.size()}")

        return {
            'avg_write_time': avg_write_time,
            'avg_read_time': avg_read_time,
            'cpp_avg_write_time': metrics.get_avg_write_time() / 1e6,  # Convert to ms
            'cpp_avg_read_time': metrics.get_avg_read_time() / 1e6,    # Convert to ms
            'validation_success_rate': success_rate,
            'cache_hit_rate': cache_hit_rate,
            'read_success_rate': read_success_rate,
            'total_time': total_time,
            'items_processed': test_range,
            'total_reads': total_reads
        }

    def measure_memory_usage(self, num_items=1000000):  # Increased to 1M
        """Measure memory usage as items are added to the cache."""
        memory_usage = []
        base_memory = self.process.memory_info().rss
        print(f"\nBase memory usage: {base_memory / (1024 * 1024):.2f} MB")

        batch_size = num_items // 100  # Record memory every 1%
        for i in range(0, num_items, batch_size):
            if i % (batch_size * 10) == 0:  # Print every 10%
                print(f"Added {i:,} items ({(i/num_items)*100:.1f}%)...")

            # Add batch_size items
            for j in range(batch_size):
                if i + j < num_items:
                    item = self.create_test_item(i + j)
                    self.cache.put(i + j, item)

            # Record memory usage
            current_memory = self.process.memory_info().rss - base_memory
            memory_usage.append((i + batch_size, current_memory))
            print(f"Current memory usage: {current_memory / (1024 * 1024):.2f} MB")

            # Force Python garbage collection to get accurate measurements
            import gc
            gc.collect()

        return memory_usage

    def test_concurrent_access(self, num_threads=20, operations_per_thread=50000):  # Increased thread count and ops
        """Test cache performance under concurrent access."""
        start_time = time.perf_counter()

        def worker(worker_id):
            local_stats = {
                'successes': 0,
                'hits': 0,
                'total_ops': 0,
                'errors': []
            }

            batch_size = operations_per_thread // 10
            for op in range(operations_per_thread):
                if op % batch_size == 0:
                    print(f"Thread {worker_id}: {(op/operations_per_thread)*100:.1f}% complete")

                try:
                    operation = random.random()
                    # Use a smaller key range to increase hit probability
                    key = random.randint(0, operations_per_thread * 2)  # Doubled key range

                    if operation < 0.6:  # 60% writes
                        item = self.create_test_item(key)
                        self.cache.put(key, item)
                        local_stats['successes'] += 1
                    else:  # 40% reads
                        result = self.cache.get(key)
                        if result is not None:
                            if self.validate_item(result, key):
                                local_stats['hits'] += 1
                                local_stats['successes'] += 1

                    local_stats['total_ops'] += 1

                except Exception as e:
                    local_stats['errors'].append(f"Operation {op}: {str(e)}")

            return local_stats

        print(f"\nStarting concurrent test with {num_threads} threads...")
        results = []

        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            futures = [executor.submit(worker, i) for i in range(num_threads)]
            results = [future.result() for future in futures]

        total_time = time.perf_counter() - start_time

        # Aggregate results
        total_successes = sum(r['successes'] for r in results)
        total_hits = sum(r['hits'] for r in results)
        total_ops = sum(r['total_ops'] for r in results)
        all_errors = [e for r in results for e in r['errors']]

        if all_errors:
            print("\nErrors encountered during concurrent testing:")
            for error in all_errors[:10]:  # Show first 10 errors
                print(error)
            if len(all_errors) > 10:
                print(f"...and {len(all_errors) - 10} more errors")

        metrics = self.cache.get_metrics()

        return {
            'operations_per_second': total_ops / total_time if total_time > 0 else 0,
            'total_operations': total_ops,
            'successful_operations': total_successes,
            'hit_rate': metrics.get_hit_rate(),
            'error_rate': (len(all_errors) / total_ops * 100) if total_ops > 0 else 0,
            'total_time': total_time
        }

    def plot_results(self, memory_usage):
        """Plot memory usage results."""
        if not memory_usage:
            print("No memory usage data to plot")
            return

        items, memory = zip(*memory_usage)

        plt.figure(figsize=(10, 6))
        plt.plot([i/1000000 for i in items], [m / (1024 * 1024) for m in memory], marker='o')
        plt.xlabel('Number of Items (millions)')
        plt.ylabel('Memory Usage (MB)')
        plt.title('Cache Memory Usage vs Number of Items')
        plt.grid(True)
        plt.savefig('cache_memory_usage.png')
        plt.close()

def main():
    try:
        print("Starting Cache Performance Test")
        print("Python version:", sys.version)

        # Initialize tester with larger cache size
        cache_size = 10000
        print(f"\nInitializing Cache Performance Tester (cache size: {cache_size:,})...")
        tester = CachePerformanceTester(cache_size=cache_size)

        # Test basic operations
        print("\n=== Basic Operations Performance ===")
        basic_metrics = tester.measure_basic_operations(num_operations=1000000)
        if basic_metrics:
            print(f"\nResults:")
            print(f"Average write time (Python measured): {basic_metrics['avg_write_time']:.3f} ms")
            print(f"Average read time (Python measured): {basic_metrics['avg_read_time']:.3f} ms")
            print(f"Average write time (C++ measured): {basic_metrics['cpp_avg_write_time']:.3f} ms")
            print(f"Average read time (C++ measured): {basic_metrics['cpp_avg_read_time']:.3f} ms")
            print(f"Validation success rate: {basic_metrics['validation_success_rate']:.2f}%")

        # Test memory usage
        print("\n=== Memory Usage Analysis ===")
        memory_usage = tester.measure_memory_usage(num_items=1000000)
        if memory_usage:
            tester.plot_results(memory_usage)
            print(f"Final cache memory usage: {memory_usage[-1][1] / (1024 * 1024):.2f} MB")
            print(f"Memory per item: {memory_usage[-1][1] / memory_usage[-1][0] / 1024:.2f} KB")

        # Test concurrent access
        print("\n=== Concurrent Access Performance ===")
        concurrent_metrics = tester.test_concurrent_access(
            num_threads=20,
            operations_per_thread=50000
        )
        print(f"\nConcurrent test results:")
        print(f"Operations per second: {concurrent_metrics['operations_per_second']:.2f}")
        print(f"Total operations: {concurrent_metrics['total_operations']:,}")
        print(f"Successful operations: {concurrent_metrics['successful_operations']:,}")
        print(f"Cache hit rate: {concurrent_metrics['hit_rate']:.2f}%")
        print(f"Error rate: {concurrent_metrics['error_rate']:.2f}%")
        print(f"Total time for concurrent operations: {concurrent_metrics['total_time']:.2f} seconds")

    except Exception as e:
        print(f"Test failed with error: {e}")
        raise

if __name__ == "__main__":
    main()
