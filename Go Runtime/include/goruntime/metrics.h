#pragma once

#include <atomic>
#include <cstdint>

namespace goruntime {

struct Metrics {
    std::atomic<std::uint64_t> tasks_submitted{0};
    std::atomic<std::uint64_t> tasks_executed{0};
    std::atomic<std::uint64_t> tasks_failed{0};

    void reset() {
        tasks_submitted.store(0, std::memory_order_relaxed);
        tasks_executed.store(0, std::memory_order_relaxed);
        tasks_failed.store(0, std::memory_order_relaxed);
    }
};

} // namespace goruntime