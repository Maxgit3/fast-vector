cmake_minimum_required(VERSION 3.20)
project(FastVector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for IDE autocomplete (VS Code / CLion)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define header-only library target
add_library(fast_ds INTERFACE)
target_include_directories(fast_ds INTERFACE include)

# Compiler warnings & optimization flags
if(MSVC)
    target_compile_options(fast_ds INTERFACE /W4)
else()
    target_compile_options(fast_ds INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Fetch dependencies automatically
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

# Prevent GoogleTest from overriding parent project compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.0
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest googlebenchmark)

# --- Unit Tests Target ---
enable_testing()
add_executable(run_tests tests/vector_test.cpp)
target_link_libraries(run_tests PRIVATE fast_ds GTest::gtest_main)

# Enable AddressSanitizer & UndefinedBehaviorSanitizer in Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    target_compile_options(run_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(run_tests PRIVATE -fsanitize=address,undefined)
endif()

include(GoogleTest)
gtest_discover_tests(run_tests)

# --- Benchmarks Target ---
add_executable(run_benchmarks benchmarks/vector_benchmark.cpp)
target_link_libraries(run_benchmarks PRIVATE fast_ds benchmark::benchmark_main)
