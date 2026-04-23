# Memory Model Notes

## 1. Purpose
This document explains the memory visibility and ordering guarantees required for correctness in the runtime.  
It defines when data is safe to read and write across threads.

## 2. Why It Matters
- Threads can execute out of order on modern CPUs.
- Compilers can reorder instructions.
- Unsynchronized shared access leads to undefined behavior in C++.
- Correct synchronization is mandatory for scheduler and channel logic.

## 3. Primitive Usage Policy
- Use mutex for shared mutable structures and compound state updates.
- Use condition_variable for blocking and waking threads.
- Use atomic only for simple flags and counters.
- Do not replace lock-protected structures with atomics prematurely.

## 4. Happens-Before Rules Used
- Unlocking a mutex happens-before a later lock of the same mutex.
- State written before unlock is visible after successful lock by another thread.
- Condition variable wait uses lock release and reacquire semantics.
- Notification does not replace predicate checks; predicate must still be true.

## 5. Task Queue Visibility Contract
- Push and pop operations happen under one queue mutex.
- Queue size and empty checks are performed under the same lock.
- Worker only executes task after lock is released.
- Submit path notifies after enqueue is complete.

## 6. Worker Coordination Contract
- Workers wait on condition_variable with predicate.
- Predicate includes:
- queue not empty
- shutdown requested
- Worker wakes can be spurious, so predicate is always rechecked.
- Shutdown broadcasts wake all waiting workers.

## 7. Channel Visibility Contract
- Buffer state and close state are guarded by channel mutex.
- Send and receive both check conditions in predicate loops.
- Close state transition wakes blocked senders and receivers.
- Buffered data movement and wake decisions are done under lock discipline.

## 8. Atomic State Contract
- Typical atomics:
- runtime stopping flag
- metrics counters where strict lock coupling is unnecessary
- If multiple fields must change consistently, use mutex instead.
- Prefer default sequential consistency first; relax only with benchmark proof.

## 9. Spurious Wakeup Handling
- All waits must use predicate-based waiting.
- Never assume a wakeup means work is available.
- Never use single if-check before wait in concurrent paths.
- Always re-evaluate state after wake.

## 10. Data Race Prevention Checklist
- Every shared mutable field has a clear owner or lock.
- No read/write path bypasses synchronization.
- No lock held while invoking unknown user callback.
- Shutdown and close transitions notify all relevant waiters.
- Tests cover shutdown while under load.

## 11. Performance Guidance
- Optimize lock scope before changing memory order.
- Keep critical sections small.
- Avoid false sharing in hot counters where possible.
- Add benchmarks before and after concurrency changes.

## 12. Validation Strategy
- Run unit tests repeatedly with high iteration counts.
- Run stress tests with many producers and consumers.
- Run sanitizer builds where available.
- Add regression tests for every discovered race or ordering bug.