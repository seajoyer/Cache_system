cmake_minimum_required(VERSION 3.10)

# Project name and version
project(CacheSystem VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add source files
set(SOURCES
    src/cache.cpp
    src/lru_strategy.cpp
)

# Add header files
set(HEADERS
    include/cache.hpp
    include/lru_strategy.hpp
)

# Include directories
include_directories(include)

# Add the main library target
add_library(CacheSystem ${SOURCES} ${HEADERS})

# Enable testing
enable_testing()

# Add Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add the test executable
add_executable(CacheTests tests/cache_tests.cpp)
target_link_libraries(CacheTests PRIVATE CacheSystem GTest::GTest GTest::Main)

# Add test to CTest
add_test(NAME CacheTests COMMAND CacheTests)
