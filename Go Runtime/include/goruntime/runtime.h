#pragma once
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>
#include<windows.h>
#include "goruntime/task.h"
namespace goruntime{
    class Runtime{
        public:
        explicit Runtime(std::size_t worker_count=0);
        ~Runtime();
        Runtime(const Runtime&)=delete;
        Runtime& operator=(const Runtime&)=delete;
        void submit(Task task);
        void shutdown();
        std::size_t worker_count() const noexcept;
        std::size_t pending_task() const;
        private:
        void worker_loop();
        std::vector<std::thread> workers_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        std::queue<Task> task_queue_;
        std::atomic<bool> stooping_{false};
    };
}
