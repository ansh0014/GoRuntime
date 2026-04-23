# Deadlock Guidelines

## 1. Purpose
This guide defines practical rules to prevent deadlocks in runtime, scheduler, and channel code.

## 2. Common Deadlock Sources
- Circular waiting between two or more locks.
- Waiting on condition variable with incorrect predicate.
- Holding a lock while invoking external or user callback.
- Blocking send or receive with no counterpart operation.
- Shutdown path that forgets to notify waiters.

## 3. Global Rules
- Use one mutex per shared structure whenever possible.
- Avoid nested locking across modules.
- Never call user code while holding runtime internal locks.
- Always wait with predicate loops.
- Always notify waiters on state transitions.

## 4. Lock Ordering Policy
- If multiple locks are unavoidable, enforce a fixed order.
- Document lock order near implementation.
- Validate lock order in code review.
- Refactor if lock order cannot be kept simple.

## 5. Condition Variable Rules
- Always use:
- wait(lock, predicate)
- Predicate must include close and shutdown states.
- Re-check predicate after wake due to spurious wakeups.
- Notify after state change is visible under lock discipline.

## 6. Channel-Specific Rules
- Send and receive must observe close state consistently.
- Close operation must wake both sender and receiver wait sets.
- Unbuffered rendezvous must not hold lock across user-visible handoff logic.
- Buffered channel must update queue and notifications atomically by lock scope.

## 7. Runtime Shutdown Rules
- Mark shutdown state once.
- Reject new submissions after shutdown starts.
- Wake all workers waiting for tasks.
- Join workers after wake broadcast.
- Ensure no worker can sleep forever during shutdown.

## 8. Safe Review Checklist
- Does any function hold lock and call unknown code?
- Can any wait predicate remain false forever?
- Are all waiters notified on close or shutdown?
- Is there any inconsistent lock acquisition order?
- Can producer and consumer both block forever by design?

## 9. Testing for Deadlocks
- Add stress tests with many producers and consumers.
- Add randomized timing and short sleeps in tests.
- Add bounded test timeouts to detect hangs.
- Run sanitizer-enabled builds where available.
- Include shutdown-during-load scenarios.

## 10. Incident Response
- Capture thread stacks during hang.
- Identify lock owner and waiting threads.
- Verify predicate and notification paths.
- Reproduce with minimal scenario.
- Add regression test before applying fix.