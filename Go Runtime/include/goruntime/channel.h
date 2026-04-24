#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

namespace goruntime {

template <typename T>
class BufferedChannel {
public:
    explicit BufferedChannel(std::size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("BufferedChannel capacity must be > 0");
        }
    }

    BufferedChannel(const BufferedChannel&) = delete;
    BufferedChannel& operator=(const BufferedChannel&) = delete;

    bool send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        can_send_.wait(lock, [this] { return closed_ || buffer_.size() < capacity_; });

        if (closed_) {
            return false;
        }

        buffer_.push_back(value);
        can_recv_.notify_one();
        return true;
    }

    bool send(T&& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        can_send_.wait(lock, [this] { return closed_ || buffer_.size() < capacity_; });

        if (closed_) {
            return false;
        }

        buffer_.push_back(std::move(value));
        can_recv_.notify_one();
        return true;
    }

    bool recv(T& out) {
        std::unique_lock<std::mutex> lock(mutex_);
        can_recv_.wait(lock, [this] { return closed_ || !buffer_.empty(); });

        if (buffer_.empty()) {
            return false;
        }

        out = std::move(buffer_.front());
        buffer_.pop_front();
        can_send_.notify_one();
        return true;
    }

    bool try_send(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_ || buffer_.size() >= capacity_) {
            return false;
        }

        buffer_.push_back(value);
        can_recv_.notify_one();
        return true;
    }

    bool try_send(T&& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_ || buffer_.size() >= capacity_) {
            return false;
        }

        buffer_.push_back(std::move(value));
        can_recv_.notify_one();
        return true;
    }

    bool try_recv(T& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (buffer_.empty()) {
            return false;
        }

        out = std::move(buffer_.front());
        buffer_.pop_front();
        can_send_.notify_one();
        return true;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        can_send_.notify_all();
        can_recv_.notify_all();
    }

    bool is_closed() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_;
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.size();
    }

    std::size_t capacity() const noexcept {
        return capacity_;
    }

private:
    const std::size_t capacity_;
    mutable std::mutex mutex_;
    std::condition_variable can_send_;
    std::condition_variable can_recv_;
    std::deque<T> buffer_;
    bool closed_{false};
};

template <typename T>
class UnbufferedChannel {
public:
    UnbufferedChannel() = default;

    UnbufferedChannel(const UnbufferedChannel&) = delete;
    UnbufferedChannel& operator=(const UnbufferedChannel&) = delete;

    bool send(const T& value) {
        return send_impl(value);
    }

    bool send(T&& value) {
        return send_impl(std::move(value));
    }

    bool recv(T& out) {
        std::unique_lock<std::mutex> lock(mutex_);

        receiver_waiting_ = true;
        cv_sender_.notify_one();

        cv_receiver_.wait(lock, [this] { return closed_ || slot_.has_value(); });

        receiver_waiting_ = false;

        if (!slot_.has_value()) {
            return false;
        }

        out = std::move(*slot_);
        slot_.reset();
        cv_sender_.notify_one();
        return true;
    }

    bool try_send(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_ || !receiver_waiting_ || slot_.has_value()) {
            return false;
        }

        slot_.emplace(value);
        cv_receiver_.notify_one();
        return true;
    }

    bool try_send(T&& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_ || !receiver_waiting_ || slot_.has_value()) {
            return false;
        }

        slot_.emplace(std::move(value));
        cv_receiver_.notify_one();
        return true;
    }

    bool try_recv(T& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!slot_.has_value()) {
            return false;
        }

        out = std::move(*slot_);
        slot_.reset();
        cv_sender_.notify_one();
        return true;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
            slot_.reset();
        }
        cv_sender_.notify_all();
        cv_receiver_.notify_all();
    }

    bool is_closed() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_;
    }

private:
    template <typename V>
    bool send_impl(V&& value) {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_sender_.wait(lock, [this] {
            return closed_ || (receiver_waiting_ && !slot_.has_value());
        });

        if (closed_) {
            return false;
        }

        slot_.emplace(std::forward<V>(value));
        cv_receiver_.notify_one();

        cv_sender_.wait(lock, [this] { return closed_ || !slot_.has_value(); });

        return !closed_;
    }

    mutable std::mutex mutex_;
    std::condition_variable cv_sender_;
    std::condition_variable cv_receiver_;

    std::optional<T> slot_;
    bool receiver_waiting_{false};
    bool closed_{false};
};

} // namespace goruntime