#pragma once
#include <atomic>
#include <cstddef>
#include <thread>
#include <vector>
#include "goruntime/config.h"
#include "goruntime/task.h"
#include "goruntime/task_queue.h"
namespace goruntime {
class Runtime {
public:
    explicit Runtime(std::size_t worker_count = 0);
    explicit Runtime(const RuntimeConfig& config);
    ~Runtime();
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    void submit(Task task);
    void shutdown();
    std::size_t worker_count() const noexcept;
    std::size_t pending_tasks() const;
private:
    void start_workers(std::size_t worker_count);
    void worker_loop();
    std::vector<std::thread> workers_;
    TaskQueue task_queue_;
    std::atomic<bool> stopping_{false};
};

} // namespace goruntime