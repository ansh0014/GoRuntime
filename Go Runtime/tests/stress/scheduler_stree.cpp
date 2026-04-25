#include "goruntime/runtime.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace {

int test_scheduler_high_volume() {
  goruntime::Runtime rt(8);

  constexpr int kTasks = 50000;
  std::atomic<int> done{0};

  for (int i = 0; i < kTasks; ++i) {
    rt.submit([&] { done.fetch_add(1, std::memory_order_relaxed); });
  }

  const auto deadline = std::chrono::steady_clock::now() + 10s;
  while (done.load(std::memory_order_relaxed) < kTasks &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(1ms);
  }

  rt.shutdown();

  if (done.load(std::memory_order_relaxed) != kTasks) {
    std::cerr << "scheduler stress failed: incomplete execution\n";
    return 1;
  }

  return 0;
}
int test_scheduler_multi_producer() {
  goruntime::Runtime rt(8);

  constexpr int kProducers = 8;
  constexpr int kPerProducer = 8000;
  constexpr int kExpected = kProducers * kPerProducer;

  std::atomic<int> done{0};
  std::vector<std::thread> producers;
  producers.reserve(kProducers);

  for (int p = 0; p < kProducers; ++p) {
    producers.emplace_back([&] {
      for (int i = 0; i < kPerProducer; ++i) {
        rt.submit([&] { done.fetch_add(1, std::memory_order_relaxed); });
      }
    });
  }

  for (auto &t : producers) {
    t.join();
  }

  const auto deadline = std::chrono::steady_clock::now() + 10s;
  while (done.load(std::memory_order_relaxed) < kExpected &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(1ms);
  }

  rt.shutdown();

  if (done.load(std::memory_order_relaxed) != kExpected) {
    std::cerr << "multi-producer stress failed\n";
    return 1;
  }

  return 0;
}

} // namespace

int main() {
  int failures = 0;
  failures += test_scheduler_high_volume();
  failures += test_scheduler_multi_producer();

  if (failures == 0) {
    std::cout << "All scheduler stress tests passed\n";
    return 0;
  }

  std::cerr << failures << " scheduler stress test(s) failed\n";
  return 1;
}