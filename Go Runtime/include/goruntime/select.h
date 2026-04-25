#pragma once
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <thread>
#include <vector>
namespace goruntime {
enum class SelectRecvState { Value, NoneReady, AllClosed, Timeout };
template <typename Channel, typename T>
SelectRecvState select_recv(const std::vector<Channel *> &channels,
                            std::size_t &cursor, std::size_t &select_index,
                            T &out) {
  if (channels.empty()) {
    throw std::invalid_argument("SelectRecv requires at least one channel");
  }
  const std::size_t n = channels.size();
  if (cursor >= n) {
    cursor = 0;
  }
  bool any_open = false;
  for (std::size_t i = 0; i < n; ++i) {
    const std::size_t index = (cursor + i) % n;
    Channel *ch = channels[index];
    if (ch->try_recv(out)) {
      select_index = index;
      cursor = (index + 1) % n;
      return SelectRecvState::Value;
    }
    if (!ch->is_closed()) {
      any_open = true;
    }
  }
  return any_open ? SelectRecvState::NoneReady : SelectRecvState::AllClosed;
}
template <typename Channel, typename T>
SelectRecvState select_recv_blocking(
    const std::vector<Channel *> &channels, std::size_t &cursor,
    std::size_t &select_index, T &out,
    std::chrono::milliseconds poly_delay = std::chrono::milliseconds(1)) {
  while (true) {
    const SelectRecvState state =
        select_recv(channels, cursor, select_index, out);
    if (state != SelectRecvState::NoneReady) {
      return state;
    }
    std::this_thread::sleep_for(poly_delay);
  }
}
template <typename Channel, typename T>
SelectRecvState select_recv_for(
    const std::vector<Channel *> &channels, std::size_t &cursor,
    std::size_t &select_index, T &out, std::chrono::milliseconds timeout,
    std::chrono::milliseconds poll_delay = std::chrono::milliseconds(1)) {
  const auto deadline = std::chrono::steady_clock::now() + timeout;

  while (true) {
    const SelectRecvState state =
        select_recv(channels, cursor, select_index, out);
    if (state == SelectRecvState::Value ||
        state == SelectRecvState::AllClosed) {
      return state;
    }

    if (std::chrono::steady_clock::now() >= deadline) {
      return SelectRecvState::Timeout;
    }

    std::this_thread::sleep_for(poll_delay);
  }
}
} // namespace goruntime
