#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include "cache/lru_cache.hpp"
#include "cache/metrics.hpp"

namespace py = pybind11;

PYBIND11_MODULE(cache_system, m) {
    m.doc() = "C++ LRU Cache system with Python bindings";

    py::class_<CacheItem>(m, "CacheItem")
        .def(py::init<>())
        .def(py::init([](int id,
                        const std::string& faculty,
                        const std::string& course,
                        const std::string& title,
                        const std::string& description,
                        int votes_count,
                        const std::string& telegram_group_link,
                        int user_id) {
            CacheItem item;
            item.id = id;
            item.faculty = faculty;
            item.course = course;
            item.title = title;
            item.description = description;
            item.votesCount = votes_count;
            item.telegramGroupLink = telegram_group_link;
            item.userId = user_id;
            return item;
        }))
        .def_readwrite("id", &CacheItem::id)
        .def_readwrite("faculty", &CacheItem::faculty)
        .def_readwrite("course", &CacheItem::course)
        .def_readwrite("title", &CacheItem::title)
        .def_readwrite("description", &CacheItem::description)
        .def_readwrite("votes_count", &CacheItem::votesCount)
        .def_readwrite("telegram_group_link", &CacheItem::telegramGroupLink)
        .def_readwrite("user_id", &CacheItem::userId);

    py::class_<CacheMetrics>(m, "CacheMetrics")
        .def(py::init<>())
        .def_property_readonly("avg_read_time", &CacheMetrics::get_avg_read_time)
        .def_property_readonly("avg_write_time", &CacheMetrics::get_avg_write_time)
        .def_property_readonly("memory_usage", &CacheMetrics::get_memory_usage)
        .def_property_readonly("peak_memory_usage", &CacheMetrics::get_peak_memory_usage)
        .def_property_readonly("item_count", &CacheMetrics::get_item_count)
        .def_property_readonly("string_memory", &CacheMetrics::get_string_memory)
        .def_property_readonly("average_item_size", &CacheMetrics::get_average_item_size)
        .def_property_readonly("hit_rate", &CacheMetrics::get_hit_rate);

    py::class_<LRUCache>(m, "LRUCache")
        .def(py::init<size_t>())
        .def("get", &LRUCache::get, py::return_value_policy::move)
        .def("put", &LRUCache::put)
        .def("remove", &LRUCache::remove)
        .def("clear", &LRUCache::clear)
        .def("size", &LRUCache::size)
        .def("save_to_file", &LRUCache::save_to_file)
        .def("load_from_file", &LRUCache::load_from_file)
        .def("get_metrics", &LRUCache::get_metrics, py::return_value_policy::reference_internal);
}
