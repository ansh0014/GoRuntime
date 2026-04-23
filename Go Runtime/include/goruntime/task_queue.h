#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>

#include "goruntime/task.h"

namespace goruntime {

class TaskQueue {
public:
    void push(Task task);
    bool pop(Task& out);
    bool wait_pop(Task& out);
    void shutdown();

    std::size_t size() const;
    bool empty() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<Task> queue_;
    bool closed_{false};
};

} 