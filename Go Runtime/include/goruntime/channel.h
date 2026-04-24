#pragma once
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <utility>
namespace goruntime {
template <typename T> class BufferedChannel {
public:
  explicit BufferedChannel(std::size_t capacity) : capacity_(capacity) {
    if (capacity_ == 0) {
      throw std::invalid_argument("BufferedChannel capacity must be positive");
    }
  }
  BufferedChannel(const BufferedChannel &) = delete;
  BufferedChannel &operator=(const BufferedChannel &) = delete;
  bool send(const T &value) {
    std::unique_lock<std::mutex> lock(mutex_);
    can_send_.wait(lock,
                   [this] { return closed_ || buffer_.size() < cpacity_; });
    if (closed_) {
      return false;
    }
    buffere_.push_back(value);
    can_recv_.notify_one();
    return true;
  }
  bool send(T &&value) {
    std::unique_lock<std::mutex> lock(mutex_);
    can_send_.wait(lock,
                   [this] { return closed_ || buffere_.size() < capacity_; });
    if (closed_) {
      return false;
    }
    buffer_.push_back(value);
    can_recv_.notify_one();
    return true;
  }
  bool recv(T &out) {
    std::unique_lock<std::mutex> lock(mutex_);
    can_recv_.wait(lock, [this] { return closed_ || !buffer_.empty() });
    if (buffer_.empty()) {
      return false;
    }
    out = std::move(buffer_.front());
    buffer_.pop_front();
    can_send_.notify_one();
    return true;
  }
  bool try_send(const T &value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_ || buffer_.size() >= capacity_) {
      return false;
    }
    buffer_.push_back(value);
    can_recv_.notify_one();
    return true;
  }
  bool try_send(T &&value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_ || buffer_.size() >= capacity_) {
      return false;
    }
    buffer_.push_back(std::move(value));
    can_recv_.notify_one();
    return true;
  }
  void close() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      closed_ = true;
    }
    can_send_.notify_all();
  }
};
} // namespace goruntime