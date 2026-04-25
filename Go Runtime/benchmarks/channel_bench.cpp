#include "goruntime/channel.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

using Clock = std::chrono::steady_clock;

int main(int argc, char **argv) {
  const int capacity = (argc > 1) ? std::atoi(argv[1]) : 1024;
  const int producers = (argc > 2) ? std::atoi(argv[2]) : 4;
  const int per_producer = (argc > 3) ? std::atoi(argv[3]) : 200000;

  if (capacity <= 0 || producers <= 0 || per_producer <= 0) {
    std::cerr << "Usage: channel_bench <capacity> <producers> <per_producer>\n";
    return 1;
  }

  goruntime::BufferedChannel<int> ch(static_cast<std::size_t>(capacity));
  const int total = producers * per_producer;

  std::atomic<int> received{0};
  std::atomic<long long> sum{0};

  std::thread consumer([&] {
    int value = 0;
    while (ch.recv(value)) {
      received.fetch_add(1, std::memory_order_relaxed);
      sum.fetch_add(value, std::memory_order_relaxed);
    }
  });
  std::vector<std::thread> prod_threads;
  prod_threads.reserve(static_cast<std::size_t>(producers));

  const auto start = Clock::now();

  for (int p = 0; p < producers; ++p) {
    prod_threads.emplace_back([&, p] {
      for (int i = 1; i <= per_producer; ++i) {
        ch.send(i + p);
      }
    });
  }

  for (auto &t : prod_threads) {
    t.join();
  }

  ch.close();
  consumer.join();

  const auto end = Clock::now();

  if (received.load(std::memory_order_relaxed) != total) {
    std::cerr << "Mismatch: received=" << received.load()
              << ", expected=" << total << "\n";
    return 1;
  }

  const auto us =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();
  const double sec = static_cast<double>(us) / 1e6;
  const double throughput = static_cast<double>(total) / sec;

  std::cout << "channel_bench\n";
  std::cout << "capacity=" << capacity << ", producers=" << producers
            << ", per_producer=" << per_producer << "\n";
  std::cout << "messages=" << total << ", sum=" << sum.load() << "\n";
  std::cout << "time_us=" << us << "\n";
  std::cout << "throughput_msgs_per_sec=" << static_cast<long long>(throughput)
            << "\n";

  return 0;
}