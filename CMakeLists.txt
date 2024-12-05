cmake_minimum_required(VERSION 3.14)
project(CacheSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(pybind11 REQUIRED)
find_package(nlohmann_json REQUIRED)

# Create object library for cache items
add_library(cache_lib STATIC
    src/cache/cache_item.cpp
    src/cache/lru_cache.cpp
    src/cache/metrics.cpp
)

target_include_directories(cache_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cache_lib PUBLIC
    nlohmann_json::nlohmann_json
)

# Add Python bindings
pybind11_add_module(cache_system
    src/python_bindings.cpp
)

target_link_libraries(cache_system PRIVATE
    cache_lib
)

# Add tests
add_executable(cache_tests
    tests/test_cache.cpp
)

target_link_libraries(cache_tests PRIVATE
    cache_lib
)

enable_testing()
add_test(NAME cache_tests COMMAND cache_tests)
