#pragma once

#include <atomic>

namespace goruntime {

class ShutdownToken {
public:
    void request() { requested_.store(true, std::memory_order_release); }
    bool requested() const { return requested_.load(std::memory_order_acquire); }

private:
    std::atomic<bool> requested_{false};
};

} // namespace goruntime