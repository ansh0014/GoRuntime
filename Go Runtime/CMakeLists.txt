cmake_minimum_required(VERSION 3.20)
project(goruntime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(goruntime STATIC
    src/runtime/runtime.cpp
)

target_include_directories(goruntime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (MSVC)
    target_compile_options(goruntime PRIVATE /W4)
else()
    target_compile_options(goruntime PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(runtime_tests
    tests/unit/runtime_tests.cpp
)

target_link_libraries(runtime_tests
    PRIVATE
        goruntime
)

add_test(NAME runtime_tests COMMAND runtime_tests)