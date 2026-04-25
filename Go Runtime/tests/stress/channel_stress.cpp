#include "goruntime/channel.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace {

int test_buffered_channel_many_producers_one_consumer() {
  goruntime::BufferedChannel<int> ch(1024);

  constexpr int kProducers = 6;
  constexpr int kPerProducer = 10000;
  constexpr int kTotal = kProducers * kPerProducer;

  std::atomic<int> sent{0};
  std::atomic<int> received{0};
  std::atomic<long long> sum{0};

  std::vector<std::thread> producers;
  producers.reserve(kProducers);

  for (int p = 0; p < kProducers; ++p) {
    producers.emplace_back([&, p] {
      for (int i = 1; i <= kPerProducer; ++i) {
        if (ch.send(i + p)) {
          sent.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }

  std::thread consumer([&] {
    int value = 0;
    while (ch.recv(value)) {
      received.fetch_add(1, std::memory_order_relaxed);
      sum.fetch_add(value, std::memory_order_relaxed);
    }
  });

  for (auto &t : producers) {
    t.join();
  }
  ch.close();
  consumer.join();

  if (sent.load(std::memory_order_relaxed) != kTotal) {
    std::cerr << "not all sends succeeded\n";
    return 1;
  }
  if (received.load(std::memory_order_relaxed) != kTotal) {
    std::cerr << "not all values were received\n";
    return 1;
  }
  if (sum.load(std::memory_order_relaxed) <= 0) {
    std::cerr << "invalid sum\n";
    return 1;
  }

  return 0;
}

int test_unbuffered_channel_close_during_wait() {
  goruntime::UnbufferedChannel<int> ch;
  std::atomic<bool> unblocked{false};

  std::thread receiver([&] {
    int out = 0;
    const bool ok = ch.recv(out);
    unblocked.store(!ok, std::memory_order_release);
  });

  std::this_thread::sleep_for(50ms);
  ch.close();
  receiver.join();

  if (!unblocked.load(std::memory_order_acquire)) {
    std::cerr << "receiver did not unblock after close\n";
    return 1;
  }

  return 0;
}

} // namespace

int main() {
  int failures = 0;
  failures += test_buffered_channel_many_producers_one_consumer();
  failures += test_unbuffered_channel_close_during_wait();

  if (failures == 0) {
    std::cout << "All channel stress tests passed\n";
    return 0;
  }

  std::cerr << failures << " channel stress test(s) failed\n";
  return 1;
}