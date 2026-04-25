cmake_minimum_required(VERSION 3.20)
project(goruntime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(goruntime STATIC
    src/runtime/runtime.cpp
    src/runtime/task_queue.cpp
    src/channel/buffered_channel.cpp
    src/channel/unbuffered_channel.cpp
    src/channel/select.cpp
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
target_link_libraries(runtime_tests PRIVATE goruntime)
add_test(NAME runtime_tests COMMAND runtime_tests)

add_executable(task_queue_tests
    tests/unit/task_queue_tests.cpp
)
target_link_libraries(task_queue_tests PRIVATE goruntime)
add_test(NAME task_queue_tests COMMAND task_queue_tests)

add_executable(channel_tests
    tests/unit/channel_tests.cpp
)
target_link_libraries(channel_tests PRIVATE goruntime)
add_test(NAME channel_tests COMMAND channel_tests)

add_executable(select_tests
    tests/unit/select_tests.cpp
)
target_link_libraries(select_tests PRIVATE goruntime)
add_test(NAME select_tests COMMAND select_tests)

add_executable(e2e_tests
    tests/integration/E2E.cpp
)
target_link_libraries(e2e_tests PRIVATE goruntime)
add_test(NAME e2e_tests COMMAND e2e_tests)

add_executable(scheduler_stress_tests
    tests/stress/scheduler_stree.cpp
)
target_link_libraries(scheduler_stress_tests PRIVATE goruntime)
add_test(NAME scheduler_stress_tests COMMAND scheduler_stress_tests)

add_executable(channel_stress_tests
    tests/stress/channel_stress.cpp
)
target_link_libraries(channel_stress_tests PRIVATE goruntime)
add_test(NAME channel_stress_tests COMMAND channel_stress_tests)

add_executable(scheduler_bench
    benchmarks/scheduler_bench.cpp
)
target_link_libraries(scheduler_bench PRIVATE goruntime)

add_executable(channel_bench
    benchmarks/channel_bench.cpp
)
target_link_libraries(channel_bench PRIVATE goruntime)