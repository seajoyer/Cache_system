cmake_minimum_required(VERSION 3.14)
project(CacheSystem)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add this section to help find pybind11
set(Python_ADDITIONAL_VERSIONS 3.8 3.9 3.10 3.11)
find_package(PythonLibs REQUIRED)
find_package(pybind11 QUIET)
if(NOT pybind11_FOUND)
    message(STATUS "pybind11 not found via find_package, trying alternative methods...")
    # Try to find pybind11 via Python
    execute_process(
        COMMAND python3 -c "import pybind11; print(pybind11.get_include())"
        OUTPUT_VARIABLE PYBIND11_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(EXISTS "${PYBIND11_INCLUDE_DIR}")
        message(STATUS "Found pybind11 via Python at: ${PYBIND11_INCLUDE_DIR}")
        add_library(pybind11::pybind11 INTERFACE IMPORTED)
        set_target_properties(pybind11::pybind11 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${PYBIND11_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR "Could not find pybind11. Please install it via your package manager or pip.")
    endif()
endif()

# Find required packages
find_package(nlohmann_json REQUIRED)

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
        ${PYBIND11_INCLUDE_DIR}  # Add this line
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
        ${PYBIND11_INCLUDE_DIR}  # Add this line
)
target_link_libraries(cache_lib
    PUBLIC nlohmann_json::nlohmann_json
)

# Python bindings module
add_library(cache_system MODULE
    src/python_bindings.cpp
    $<TARGET_OBJECTS:cache_lib_obj>
)
target_include_directories(cache_system
    PRIVATE
        ${CACHE_INCLUDE_DIRS}
        ${PYBIND11_INCLUDE_DIR}  # Add this line
        ${PYTHON_INCLUDE_DIRS}   # Add this line
)
target_link_libraries(cache_system
    PRIVATE
        nlohmann_json::nlohmann_json
        ${PYTHON_LIBRARIES}      # Add this line
)
set_target_properties(cache_system PROPERTIES PREFIX "")
if(APPLE)
    set_target_properties(cache_system PROPERTIES SUFFIX ".so")
endif()

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
