#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "cache/lru_cache.hpp"

namespace py = pybind11;

PYBIND11_MODULE(cache_system, m) {
    m.doc() = "C++ LRU Cache system with Python bindings";

    py::class_<CacheItem>(m, "CacheItem")
        .def(py::init<>())
        .def_readwrite("id", &CacheItem::id)
        .def_readwrite("faculty", &CacheItem::faculty)
        .def_readwrite("course", &CacheItem::course)
        .def_readwrite("title", &CacheItem::title)
        .def_readwrite("description", &CacheItem::description)
        .def_readwrite("votes_count", &CacheItem::votesCount)
        .def_readwrite("telegram_group_link", &CacheItem::telegramGroupLink)
        .def_readwrite("user_id", &CacheItem::userId);

    py::class_<CacheMetrics>(m, "CacheMetrics")
        .def("get_avg_read_time", &CacheMetrics::get_avg_read_time)
        .def("get_avg_write_time", &CacheMetrics::get_avg_write_time)
        .def("get_memory_usage", &CacheMetrics::get_memory_usage);

    py::class_<LRUCache>(m, "LRUCache")
        .def(py::init<size_t>())
        .def("get", &LRUCache::get)
        .def("put", &LRUCache::put)
        .def("remove", &LRUCache::remove)
        .def("clear", &LRUCache::clear)
        .def("size", &LRUCache::size)
        .def("save_to_file", &LRUCache::save_to_file)
        .def("load_from_file", &LRUCache::load_from_file)
        .def("get_metrics", &LRUCache::get_metrics);
}
