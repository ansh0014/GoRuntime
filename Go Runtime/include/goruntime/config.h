#pragma once

#include <cstddef>

namespace goruntime {

struct RuntimeConfig {
    std::size_t worker_count{0};  
    std::size_t queue_capacity{0}; 
    bool enable_metrics{true};
};

} // namespace goruntime