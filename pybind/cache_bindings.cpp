#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <optional>
#include "../include/cache.hpp"

namespace py = pybind11;

PYBIND11_MODULE(cache_module, m) {
    py::class_<Cache>(m, "Cache")
        .def(py::init<std::chrono::seconds>(), py::arg("default_ttl"))
        .def("put", &Cache::put, py::arg("key"), py::arg("value"))
        .def("get", [](Cache& self, const std::string& key) -> std::optional<json> {
            auto result = self.get(key);
            if (result) {
                return *result;
            }
            return std::nullopt;
        }, py::arg("key"))
        .def("remove", &Cache::remove, py::arg("key"))
        .def("clear", &Cache::clear)
        .def("size", &Cache::size)
        .def("save_to_file", &Cache::saveToFile, py::arg("filename"))
        .def("load_from_file", &Cache::loadFromFile, py::arg("filename"));
}
