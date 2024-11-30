cmake_minimum_required(VERSION 3.10)

project(Cache_system)

set(CMAKE_CXX_STANDARD 17)

# Основной исполняемый файл
add_executable(cache_tests tests/cache_tests.cpp src/cache.cpp)

# Путь к заголовочным файлам
target_include_directories(cache_tests PRIVATE include)

# Подключение JSON-библиотеки
find_package(nlohmann_json REQUIRED)
target_link_libraries(cache_tests PRIVATE nlohmann_json::nlohmann_json)


add_executable(performance_tests tests/performance_tests.cpp src/cache.cpp)
target_include_directories(performance_tests PRIVATE include)
target_link_libraries(performance_tests PRIVATE nlohmann_json::nlohmann_json)


# Подключаем pybind11
find_package(pybind11 REQUIRED)

# Добавляем модуль Python
add_library(cache_module MODULE pybind/cache_bindings.cpp src/cache.cpp)
target_include_directories(cache_module PRIVATE include)
target_link_libraries(cache_module PRIVATE pybind11::module nlohmann_json::nlohmann_json)

# Установка имени файла модуля
set_target_properties(cache_module PROPERTIES PREFIX "" SUFFIX ".so")
