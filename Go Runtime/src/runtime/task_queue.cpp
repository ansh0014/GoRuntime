#include "goruntime/task_queue.h"
#include <stdexcept>
#include <utility>
namespace goruntime {
void TaskQueue::push(Task task) {
  if (!task) {
    throw std::invalid_argument("TaskQueue: push received empty task");
  }
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) {
      throw std::runtime_error("TaskQueue is closed");
    }
    queue_.push(std::move(task));
  }
  cv_.notify_one();
}
bool TaskQueue::pop(Task &out) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (queue_.empty()) {
    return false;
  }
  out = std::move(queue_.front());
  queue_.pop();
  return true;
}
bool TaskQueue::wait_pop(Task &out) {
  std::unique_lock<std::mutex> lock(mutex_);
  cv_.wait(lock, [this] { return closed_ || !queue_.empty(); });
  if (queue_.empty()) {
    return false;
  }
  out = std::move(queue_.front());
  queue_.pop();
  return true;
}
void TaskQueue::shutdown() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    closed_ = true;
  }
  cv_.notify_all();
}
std::size_t TaskQueue::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}
bool TaskQueue::empty() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}
}