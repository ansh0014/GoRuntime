#include "goruntime/channel.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
using namespace std::chrono_literals;
namespace {
int test_send_then_recv() {
  goruntime::BufferedChannel<int> ch(2);
  if (!ch.send(10)) {
    std::cerr << "send failed" << std::endl;
    return 1;
  }
  int out = 0;
  if (!ch.recv(out)) {
    std::cerr << "recv failed" << std::endl;
    return 1;
  }
  if (out != 10) {
    std::cerr << "recv wrong value: " << out << std::endl;
    return 1;
  }
  return 0;
}
int test_try_send_capacity_limit() {
  goruntime::BufferedChannel<int> ch(1);
  if (!ch.try_send(1)) {
    std::cerr << "try_send failed on empty channel" << std::endl;
    return 1;
  }
  if (ch.try_send(2)) {
    std::cerr << "second try_send should have failed due to capacity limit" << std::endl;
    return 1;
  }
  return 0;
}
int test_close_unblocks_receiver() {
  goruntime::BufferedChannel<std::string> ch(1);
  std::atomic<bool> finished{false};
  std::thread t([&] {
    std::string out;
    const bool ok = ch.recv(out);
    finished.store(!ok, std::memory_order_release);
  });
  std::this_thread::sleep_for(50ms);
  ch.close();
  t.join();
  if (!finished.load(std::memory_order_acquire)) {
    std::cerr << "recv should have been unblocked by close" << std::endl;
    return 1;
  }
  return 0;
}
int test_close_rejects_send() {
  goruntime::BufferedChannel<int> ch(2);
  ch.close();

  if (ch.send(5)) {
    std::cerr << "send should fail after close\n" << std::endl;
    return 1;
  }

  return 0;
}

}
int main() {
  int failed = 0;
  failed += test_send_then_recv();
  failed += test_try_send_capacity_limit();
  failed += test_close_unblocks_receiver();
  failed += test_close_rejects_send();
  if (failed == 0) {
    std::cout << "All tests passed!" << std::endl;
  } else {
    std::cerr << failed << " tests failed." << std::endl;
  }
  return failed;
}