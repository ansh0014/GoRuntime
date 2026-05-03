#include "goruntime/runtime.h"
#include <stdexcept>
namespace goruntime {
Runtime::Runtime(std::size_t worker_count) {
  if (worker_count == 0) {
    worker_count = std::thread::hardware_concurrency();
    if (worker_count == 0) {
      worker_count = 4;
    }
  }
  start_workers(worker_count);
}

Runtime::Runtime(const RuntimeConfig &config) {
  std::size_t worker_count = config.worker_count;
  if (worker_count == 0) {
    worker_count = std::thread::hardware_concurrency();
    if (worker_count == 0) {
      worker_count = 4;
    }
  }
  start_workers(worker_count);
}

Runtime::~Runtime() { shutdown(); }

void Runtime::start_workers(std::size_t worker_count) {
  workers_.reserve(worker_count);
  for (std::size_t i = 0; i < worker_count; ++i) {
    workers_.emplace_back([this] { worker_loop(); });
  }
}

void Runtime::shutdown() {
  bool expected = false;
  if (!stopping_.compare_exchange_strong(expected, true,
                                         std::memory_order_acq_rel)) {
    return;
  }

  task_queue_.shutdown();

  for (auto &worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

std::size_t Runtime::worker_count() const noexcept { return workers_.size(); }

std::size_t Runtime::pending_tasks() const { return task_queue_.size(); }

} // namespace goruntime