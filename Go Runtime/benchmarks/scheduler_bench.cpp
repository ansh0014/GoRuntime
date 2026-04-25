#include "goruntime/runtime.h"
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
using Clock = std::chrono::steady_clock;
int main(int argc, char **argv) {
  const int workers = (argc > 1) ? std::atoi(argv[1]) : 8;
  const int tasks = (argc > 2) ? std::atoi(argv[2]) : 500000;

  if (workers <= 0 || tasks <= 0) {
    std::cerr << "Usage: scheduler_bench <workers> <tasks>\n";
    return 1;
  }
  goruntime::Runtime rt(static_cast<std::size_t>(workers));
  std::atomic<int> done{0};

  const auto start = Clock::now();

  for (int i = 0; i < tasks; ++i) {
    rt.submit([&] { done.fetch_add(1, std::memory_order_relaxed); });
  }

  while (done.load(std::memory_order_relaxed) < tasks) {
    std::this_thread::yield();
  }
  const auto end = Clock::now();
  rt.shutdown();

  const auto us =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();
  const double sec = static_cast<double>(us) / 1e6;
  const double throughput = static_cast<double>(tasks) / sec;

  std::cout << "scheduler_bench\n";
  std::cout << "workers=" << workers << ", tasks=" << tasks << "\n";
  std::cout << "time_us=" << us << "\n";
  std::cout << "throughput_tasks_per_sec=" << static_cast<long long>(throughput)
            << "\n";

  return 0;
}