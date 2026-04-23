# Go Runtime Architecture

## 1. Purpose
This document defines the architecture of a Go-like concurrency runtime implemented in C++.  
The system is designed for correctness first, then performance.

## 2. Goals
- Execute many lightweight tasks on a fixed worker pool.
- Provide channel-based communication between tasks.
- Support deterministic startup and shutdown.
- Avoid data races and common deadlock patterns.
- Keep internals modular and testable.

## 3. Non-Goals (Current Version)
- Full Go-equivalent runtime behavior.
- Work stealing and per-worker local queues.
- Preemptive scheduling.
- Garbage collection.
- Automatic stack growth.

## 4. High-Level Components
- Runtime Controller: starts and stops the system.
- Scheduler: accepts tasks and dispatches them.
- Worker Pool: long-lived threads executing tasks.
- Task Queue: thread-safe queue for pending tasks.
- Channel Layer: buffered and unbuffered communication.
- Select Layer: waits across multiple channel operations.
- Core Utilities: errors, metrics, lifecycle helpers.

## 5. Module Structure
- include/goruntime
- include/goruntime/runtime.h
- include/goruntime/task.h
- include/goruntime/channel.h
- include/goruntime/select.h
- include/goruntime/config.h
- include/goruntime/error.h
- src/runtime
- src/runtime/runtime.cpp
- src/runtime/scheduler.cpp
- src/runtime/worker.cpp
- src/runtime/task_queue.cpp
- src/channel
- src/channel/channel_base.cpp
- src/channel/buffered_channel.cpp
- src/channel/unbuffered_channel.cpp
- src/channel/select.cpp
- src/core
- src/core/error.cpp
- src/core/metrics.cpp
- src/core/shutdown.cpp

## 6. Runtime Data Flow
1. Client submits a task.
2. Scheduler validates runtime state.
3. Scheduler pushes task into queue.
4. Worker wakes and pops task.
5. Worker executes task outside queue lock.
6. Task may interact with channel APIs.
7. Runtime drains or stops depending on shutdown mode.

## 7. Threading Model
- Workers are created once at startup.
- Workers block on condition variable when no tasks are available.
- Shared structures are protected by mutex.
- Lifecycle flags use atomics where lock-free state checks are enough.
- Task callbacks must never execute while queue mutex is held.

## 8. Channel Model
- Buffered channel:
- Stores values up to fixed capacity.
- Send blocks when full.
- Receive blocks when empty.
- Unbuffered channel:
- Send and receive synchronize directly.
- No internal value queue.
- Close semantics:
- Send after close fails.
- Receive continues until buffered values are drained, then reports closed.

## 9. Select Model
- Initial implementation supports non-blocking attempts.
- Selection checks operations in a fairness-preserving order.
- If no operation is ready, caller may retry or back off.
- Future revision may provide fully blocking select with wait registration.

## 10. Error Handling Strategy
- Public APIs return explicit status values or typed errors.
- Invalid lifecycle actions are surfaced immediately.
- Error messages prioritize operational clarity.
- Internal invariants use defensive assertions in debug builds.

## 11. Observability
- Metrics to track:
- queue depth
- submitted tasks
- completed tasks
- worker idle time
- channel send and receive wait counts
- Logging should be optional and low overhead.
- Debug verbosity must be configurable.

## 12. Testing Strategy Alignment
- Unit tests validate each component in isolation.
- Integration tests validate end-to-end task and channel behavior.
- Stress tests run high concurrency scenarios.
- Benchmarks track scheduler and channel throughput and latency.

## 13. Milestones
1. Runtime lifecycle and queue-based scheduler.
2. Buffered channel with close behavior.
3. Unbuffered channel handshake behavior.
4. Select with non-blocking operations.
5. Metrics and error model hardening.
6. Stress and benchmark tuning.