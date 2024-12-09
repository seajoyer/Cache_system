#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "cache/lru_cache.hpp"

namespace py = pybind11;

// Custom type caster for std::experimental::optional
namespace pybind11 { namespace detail {
    template <typename T>
    struct type_caster<std::experimental::optional<T>> : optional_caster<std::experimental::optional<T>> {};
}}

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
            auto item = CacheItem();
            item.id = id;
            // Ensure strings are copied properly
            item.faculty = std::string(faculty);
            item.course = std::string(course);
            item.title = std::string(title);
            item.description = std::string(description);
            item.votesCount = votes_count;
            item.telegramGroupLink = std::string(telegram_group_link);
            item.userId = user_id;
            return item;
        }), py::arg("id") = 0,
            py::arg("faculty") = "",
            py::arg("course") = "",
            py::arg("title") = "",
            py::arg("description") = "",
            py::arg("votes_count") = 0,
            py::arg("telegram_group_link") = "",
            py::arg("user_id") = 0)
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
        .def("get_memory_usage", &CacheMetrics::get_memory_usage)
        .def("get_hit_rate", &CacheMetrics::get_hit_rate);

    py::class_<LRUCache>(m, "LRUCache")
        .def(py::init<size_t>())
        .def("get", &LRUCache::get, py::return_value_policy::copy)
        .def("put", &LRUCache::put)
        .def("remove", &LRUCache::remove)
        .def("clear", &LRUCache::clear)
        .def("size", &LRUCache::size)
        .def("save_to_file", &LRUCache::save_to_file)
        .def("load_from_file", &LRUCache::load_from_file)
        .def("get_metrics", &LRUCache::get_metrics);
}
