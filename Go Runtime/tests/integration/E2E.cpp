#include "goruntime/channel.h"
#include "goruntime/runtime.h"
#include "goruntime/select.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace {

int test_runtime_channel_pipeline() {
  goruntime::Runtime rt(4);
  goruntime::BufferedChannel<int> ch(32);

  constexpr int kItems = 1000;
  std::atomic<int> remaining{kItems};
  std::atomic<int> received_count{0};
  std::atomic<long long> received_sum{0};
  std::thread consumer([&] {
    int value = 0;
    while (ch.recv(value)) {
      received_count.fetch_add(1, std::memory_order_relaxed);
      received_sum.fetch_add(value, std::memory_order_relaxed);
    }
  });
  for (int i = 1; i <= kItems; ++i) {
    rt.submit([&, i] {
      ch.send(i);
      if (remaining.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        ch.close();
      }
    });
  }
  consumer.join();
  rt.shutdown();
  const long long expected_sum =
      static_cast<long long>(kItems) * (kItems + 1) / 2;
  if (received_count.load(std::memory_order_relaxed) != kItems) {
    std::cerr << "pipeline count mismatch\n";
    return 1;
  }
  if (received_sum.load(std::memory_order_relaxed) != expected_sum) {
    std::cerr << "pipeline sum mismatch\n";
    return 1;
  }

  return 0;
}
int test_runtime_plus_select_recv_for() {
  goruntime::Runtime rt(2);
  goruntime::BufferedChannel<int> c1(4);
  goruntime::BufferedChannel<int> c2(4);
  rt.submit([&] { c2.send(99); });
  rt.submit([&] {
    std::this_thread::sleep_for(10ms);
    c1.close();
    c2.close();
  });
  std::vector<goruntime::BufferedChannel<int> *> channels{&c1, &c2};
  std::size_t cursor = 0;
  std::size_t selected = 0;
  int out = 0;

  const auto state =
      goruntime::select_recv_for(channels, cursor, selected, out, 200ms, 1ms);
  rt.shutdown();
  if (state != goruntime::SelectRecvState::Value) {
    std::cerr << "expected select value\n";
    return 1;
  }
  if (selected != 1 || out != 99) {
    std::cerr << "wrong channel/value from select\n";
    return 1;
  }

  return 0;
}

} // namespace

int main() {
  int failures = 0;
  failures += test_runtime_channel_pipeline();
  failures += test_runtime_plus_select_recv_for();

  if (failures == 0) {
    std::cout << "All integration tests passed\n";
    return 0;
  }

  std::cerr << failures << " integration test(s) failed\n";
  return 1;
}