#pragma once
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

#include "goruntime/task.h"
#include "goruntime/task_queue.h"
namespace goruntime {
class Runtime {
public:
  explicit Runtime(std::size_t worker_count = 0);
  ~Runtime();
  Runtime(const Runtime &) = delete;
  Runtime &operator=(const Runtime &) = delete;
  void submit(Task task);
  void shutdown();
  std::size_t worker_count() const noexcept;
  std::size_t pending_tasks() const;

private:
  void worker_loop();
  std::vector<std::thread> workers_;
  TaskQueue task_queue_;
  std::atomic<bool> stopping_{false};
};
} 
