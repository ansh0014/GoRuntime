# Concurrency Model

## 1. Overview
This runtime follows a cooperative task model backed by OS threads.  
Tasks are user-level work units executed by a fixed worker pool.

## 2. Core Entities
- Task: callable work item.
- Worker: dedicated thread executing tasks.
- Scheduler: handoff mechanism between submitters and workers.
- Queue: shared pending-task buffer.
- Channel: synchronization and communication primitive.

## 3. Scheduling Semantics
- Tasks are FIFO at queue level in the baseline implementation.
- No strict fairness guarantee across producers.
- Worker wakeups are condition-variable based.
- Task execution order may differ under contention.
- Long-running tasks can reduce effective throughput.

## 4. Synchronization Rules
- Queue access is guarded by one mutex.
- Condition variable coordinates sleep and wake.
- Shutdown flag is atomic for fast state checks.
- Complex shared state uses lock + condition variable, not atomics alone.
- Locks must be held for shortest possible duration.

## 5. Happens-Before Guarantees
- Queue push under lock happens-before worker pop under same lock.
- Condition variable notification synchronizes waiting worker state transitions.
- Channel send and receive synchronize through channel mutex and wait conditions.
- Proper lock acquisition and release establish memory visibility.

## 6. Task Lifecycle
1. Task created by caller.
2. Task submitted to scheduler.
3. Task enqueued.
4. Worker dequeues task.
5. Task executes.
6. Task completes or throws.
7. Completion metrics updated.

## 7. Channel Interaction Model
- Buffered send:
- Succeeds immediately if buffer has space.
- Otherwise blocks until space is available or channel closes.
- Buffered receive:
- Succeeds immediately if buffer has data.
- Otherwise blocks until data arrives or channel closes.
- Unbuffered operations:
- Send and receive rendezvous directly.

## 8. Backpressure Behavior
- Buffered channels provide bounded capacity.
- Full buffer forces sender slowdown.
- This naturally controls producer burst rate.
- Queue and channel sizes should be configured based on workload.

## 9. Cancellation and Timeouts (Planned)
- Base model is blocking until success or close.
- Future extension:
- timeout-aware send and receive
- cancellation tokens
- deadline-based select options

## 10. Deadlock and Liveness Notes
- Avoid cyclic wait dependencies between tasks.
- Avoid nested locking across independent structures.
- Prefer single ownership of synchronization per module.
- Keep blocking waits bounded where possible.

## 11. Practical Guidelines for Contributors
- Never run user task while holding internal lock.
- Keep lock scope small and explicit.
- Validate shutdown and close paths first.
- Add tests for race-prone branches before optimization.