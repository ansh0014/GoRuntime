#include "goruntime/task_queue.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
using namespace std::chrono_literals;
namespace {
int test_push_and_wait_pop() {
  goruntime::TaskQueue q;
  std::atomic<bool> ran{false};
  q.push([&] { ran.store(true, std::memory_order_release); });
  goruntime::Task task;
  const bool ok = q.wait_pop(task);
  if (!ok) {
    std::cerr
        << "Test failed: wait_pop returned false when a task was available.\n";
    return 1;
  }
  task();
  if (!ran.load(std::memory_order_acquire)) {
    std::cerr << "Test failed: Task did not execute as expected.\n";
    return 1;
  }

  return 0;
}
int test_shutdown_unblocks_wait() {
  goruntime::TaskQueue q;
  std::atomic<bool> unblocked{false};
  std::thread waiter([&] {
    goruntime::Task task;
    const bool ok = q.wait_pop(task);
    unblocked.store(!ok, std::memory_order_release);
  });
  std::this_thread::sleep_for(50ms);
  q.shutdown();
  waiter.join();

  if (!unblocked.load(std::memory_order_acquire)) {
    std::cerr << "Test failed: shutdown did not unblock waiting thread.\n";
    return 1;
  }
  return 0;
}

} 

int main() {
  int failures = 0;
  failures += test_push_and_wait_pop();
  failures += test_shutdown_unblocks_wait();
  if (failures == 0) {
    std::cout << "All task_queue tests passed\n";
    return 0;
  }
  std::cerr << failures << " task_queue test(s) failed\n";
  return 1;
}