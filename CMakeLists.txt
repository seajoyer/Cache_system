cmake_minimum_required(VERSION 3.14)
project(CacheSystem)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Include FetchContent for downloading dependencies
include(FetchContent)

# Configure pybind11
if(NOT TARGET pybind11::pybind11)
    message(STATUS "Fetching pybind11...")
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG        v2.10.4
    )
    FetchContent_MakeAvailable(pybind11)
endif()

# Configure nlohmann_json
message(STATUS "Fetching nlohmann_json...")
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
    GIT_SHALLOW TRUE
)

FetchContent_GetProperties(nlohmann_json)
if(NOT nlohmann_json_POPULATED)
    FetchContent_Populate(nlohmann_json)
    add_subdirectory(${nlohmann_json_SOURCE_DIR} ${nlohmann_json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Define include directories
set(CACHE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Cache library source files
set(CACHE_LIB_SOURCES
    src/cache/cache_item.cpp
    src/cache/lru_cache.cpp
    src/cache/metrics.cpp
)

# Create object library
add_library(cache_lib_obj OBJECT ${CACHE_LIB_SOURCES})
target_include_directories(cache_lib_obj
    PUBLIC
        ${CACHE_INCLUDE_DIRS}
)
target_link_libraries(cache_lib_obj
    PUBLIC nlohmann_json::nlohmann_json
)
set_property(TARGET cache_lib_obj PROPERTY POSITION_INDEPENDENT_CODE ON)

# Create shared library
add_library(cache_lib SHARED $<TARGET_OBJECTS:cache_lib_obj>)
target_include_directories(cache_lib
    PUBLIC
        ${CACHE_INCLUDE_DIRS}
)
target_link_libraries(cache_lib
    PUBLIC nlohmann_json::nlohmann_json
)

# Python bindings
pybind11_add_module(cache_system
    src/python_bindings.cpp
    $<TARGET_OBJECTS:cache_lib_obj>
)

target_include_directories(cache_system
    PRIVATE
        ${CACHE_INCLUDE_DIRS}
)
target_link_libraries(cache_system
    PRIVATE nlohmann_json::nlohmann_json
)

# Copy Python module to example directory
add_custom_command(TARGET cache_system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:cache_system>
    ${CMAKE_SOURCE_DIR}/example/$<TARGET_FILE_NAME:cache_system>
)

# Tests
enable_testing()
add_executable(cache_tests tests/test_cache.cpp)
target_include_directories(cache_tests
    PRIVATE
        ${CACHE_INCLUDE_DIRS}
)
target_link_libraries(cache_tests
    PRIVATE cache_lib
)
add_test(NAME cache_tests COMMAND cache_tests)
