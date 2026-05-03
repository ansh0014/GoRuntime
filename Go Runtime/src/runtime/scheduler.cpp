#include "goruntime/runtime.h"

#include <stdexcept>

namespace goruntime {

void Runtime::submit(Task task) {
    if (stopping_.load(std::memory_order_acquire)) {
        throw std::runtime_error("Runtime is stopping, cannot submit new tasks");
    }
    task_queue_.push(std::move(task));
}

} // namespace goruntime