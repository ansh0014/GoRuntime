#include "goruntime/channel.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
using namespace std::chrono_literals;
namespace {
int test_buffered_send_then_recv() {
  goruntime::BufferedChannel<int> ch(2);

  if (!ch.send(10)) {
    std::cerr << "buffered send failed unexpectedly\n";
    return 1;
  }

  int out = 0;
  if (!ch.recv(out) || out != 10) {
    std::cerr << "buffered recv mismatch\n";
    return 1;
  }

  return 0;
}

int test_buffered_close_rejects_send() {
  goruntime::BufferedChannel<int> ch(1);
  ch.close();

  if (ch.send(5)) {
    std::cerr << "buffered send should fail after close\n";
    return 1;
  }

  return 0;
}

int test_unbuffered_send_recv() {
  goruntime::UnbufferedChannel<int> ch;
  std::atomic<bool> ok{false};

  std::thread receiver([&] {
    int out = 0;
    if (ch.recv(out) && out == 42) {
      ok.store(true, std::memory_order_release);
    }
  });

  std::this_thread::sleep_for(20ms);
  const bool sent = ch.send(42);
  receiver.join();

  if (!sent || !ok.load(std::memory_order_acquire)) {
    std::cerr << "unbuffered send/recv failed\n";
    return 1;
  }

  return 0;
}

int test_unbuffered_try_send_without_receiver_fails() {
  goruntime::UnbufferedChannel<int> ch;
  if (ch.try_send(7)) {
    std::cerr << "try_send should fail without waiting receiver\n";
    return 1;
  }
  return 0;
}

int test_unbuffered_close_unblocks_receiver() {
  goruntime::UnbufferedChannel<int> ch;
  std::atomic<bool> unblocked{false};

  std::thread receiver([&] {
    int out = 0;
    const bool got = ch.recv(out);
    unblocked.store(!got, std::memory_order_release);
  });

  std::this_thread::sleep_for(50ms);
  ch.close();
  receiver.join();

  if (!unblocked.load(std::memory_order_acquire)) {
    std::cerr << "close should unblock waiting receiver\n";
    return 1;
  }

  return 0;
}

} // namespace

int main() {
  int failures = 0;
  failures += test_buffered_send_then_recv();
  failures += test_buffered_close_rejects_send();
  failures += test_unbuffered_send_recv();
  failures += test_unbuffered_try_send_without_receiver_fails();
  failures += test_unbuffered_close_unblocks_receiver();

  if (failures == 0) {
    std::cout << "All channel tests passed\n";
    return 0;
  }

  std::cerr << failures << " channel test(s) failed\n";
  return 1;
}