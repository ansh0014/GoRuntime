#include "goruntime/channel.h"
#include "goruntime/select.h"
#include <iostream>
#include <vector>
namespace {
int test_select_recv_no_ready() {
  goruntime::BufferedChannel<int> ch1(1);
  goruntime::BufferedChannel<int> ch2(2);
  ch2.send(20);
  std::vector<goruntime::BufferedChannel<int> *> channels{&ch1, &ch2};
  std::size_t cursor = 0;
  std::size_t selected = 0;
  int out = 0;
  const auto state = goruntime::select_recv(channels, cursor, selected, out);
  if (state != goruntime::SelectRecvState::Value || selected != 1 ||
      out != 20) {
    std::cerr << "select_recv failed to receive value\n";
    return 1;
  }
  if (selected != 1 || out != 20) {
    std::cerr << "select_recv selected wrong channel or value\n";
    return 1;
  }
  return 0;
}

int test_select_round_robin_when_multiple_ready() {
  goruntime::BufferedChannel<int> ch1(2);
  goruntime::BufferedChannel<int> ch2(2);
  ch1.send(1);
  ch2.send(2);
  std::vector<goruntime::BufferedChannel<int> *> channels{&ch1, &ch2};
  std::size_t cursor = 0;
  std::size_t selected = 0;
  int out = 0;
  auto state = goruntime::select_recv(channels, cursor, selected, out);
  if (state != goruntime::SelectRecvState::Value || selected != 0 || out != 1) {
    std::cerr << "first selection mismatch\n";
    return 1;
  }
  state = goruntime::select_recv(channels, cursor, selected, out);
  if (state != goruntime::SelectRecvState::Value || selected != 1 || out != 2) {
    std::cerr << "second selection mismatch\n";
    return 1;
  }
  return 0;
}
int test_select_none_ready() {
  goruntime::BufferedChannel<int> ch1(1);
  goruntime::BufferedChannel<int> ch2(1);

  std::vector<goruntime::BufferedChannel<int> *> channels{&ch1, &ch2};
  std::size_t cursor = 0;
  std::size_t selected = 0;
  int out = 0;

  const auto state = goruntime::select_recv(channels, cursor, selected, out);
  if (state != goruntime::SelectRecvState::NoneReady) {
    std::cerr << "expected NoneReady state\n";
    return 1;
  }

  return 0;
}
int test_select_all_closed() {
  goruntime::BufferedChannel<int> ch1(1);
  goruntime::BufferedChannel<int> ch2(1);
  ch1.close();
  ch2.close();
  std::vector<goruntime::BufferedChannel<int> *> channels{&ch1, &ch2};
  std::size_t cursor = 0;
  std::size_t selected = 0;
  int out = 0;
  const auto state = goruntime::select_recv(channels, cursor, selected, out);
  if (state != goruntime::SelectRecvState::AllClosed) {
    std::cerr << "expected AllClosed state\n";
    return 1;
  }

  return 0;
}
} // namespace
int main() {
  int failures = 0;
  failures += test_select_recv_no_ready();
  failures += test_select_round_robin_when_multiple_ready();
  failures += test_select_none_ready();
  failures += test_select_all_closed();
  if (failures == 0) {
    std::cout << "All select tests passed\n";
    return 0;
  }
  std::cerr << failures << " select test(s) failed\n";
  return 1;
}