# Go-like Concurrency Runtime in C++

## 1. Purpose

The goal of this project is to deeply understand how concurrency systems like Go's runtime work by building a similar system manually in C++.

This project focuses on:
- Understanding thread management at a low level
- Designing a scheduler using a thread pool
- Implementing communication using channels
- Handling synchronization safely
- Avoiding race conditions and deadlocks
- Exploring performance and scalability

This is not just a coding exercise, but a systems-level learning project.

---

## 2. Motivation

In Go:
- Goroutines are lightweight
- Scheduling is handled by the runtime
- Channels simplify communication

In C++:
- Threads map directly to OS threads
- No built-in scheduler
- No native channels

This project bridges that gap by building these abstractions manually.

---

## 3. Core Concepts

### 3.1 Task (Goroutine Equivalent)

A task is a unit of work:
- Represented as a function
- Executed asynchronously
- Managed by the runtime

Equivalent to:
- Go → goroutine
- C++ → function wrapped in task queue

---

### 3.2 Runtime (Scheduler)

The runtime is responsible for:
- Managing worker threads
- Scheduling tasks
- Executing tasks safely
- Handling shutdown

Internally:
- Uses a task queue
- Uses worker threads to process tasks
- Uses condition variables for coordination

---

### 3.3 Worker Threads

Worker threads:
- Run in an infinite loop
- Wait for tasks
- Execute tasks when available

Key idea:
- Threads are reused
- No thread creation per task

---

### 3.4 Task Queue

The task queue:
- Stores pending tasks
- Is shared across threads
- Must be thread-safe

Implementation considerations:
- Protected by mutex
- Access controlled using condition_variable

---

### 3.5 Channel (Communication System)

Channels provide communication between tasks.

Types:

#### Unbuffered Channel
- Direct synchronization between sender and receiver
- Sender blocks until receiver is ready

#### Buffered Channel
- Has capacity
- Sender blocks only when buffer is full
- Receiver blocks when empty

Internally:
- Implemented using queue
- Uses mutex + condition_variable

---

### 3.6 Select Mechanism

Select allows waiting on multiple channels.

Purpose:
- Avoid blocking on a single channel
- React to whichever channel is ready first

Implementation approach:
- Try non-blocking operations
- Loop until one succeeds
- Yield CPU to avoid busy waiting

---

## 4. How It Works (Execution Flow)

1. User submits a task:
   - Task is added to queue

2. Worker thread wakes up:
   - Picks task from queue

3. Task executes:
   - May send/receive data via channel

4. Channel:
   - Synchronizes sender and receiver

5. System continues:
   - Until shutdown signal

---

## 5. Synchronization Model

### Tools Used:
- mutex → protect shared data
- condition_variable → block/wake threads
- atomic → simple flags (e.g., stop signal)

### Why not only atomic?
- Atomics work for simple variables
- Complex structures (queues) require mutex

---

## 6. Deadlock Considerations

Deadlocks occur when:
- Threads wait indefinitely
- Circular dependencies exist

Examples:
- Receiver waits but no sender exists
- Multiple locks acquired in different order

Prevention strategies:
- Use single lock per structure
- Avoid nested locks
- Avoid holding lock while waiting
- Prefer message passing (channels)

---

## 7. Memory Model Understanding

Important concepts:
- Threads share memory
- CPU may reorder instructions
- Cache inconsistency can occur

Guarantees needed:
- Proper synchronization
- Happens-before relationships
- Safe visibility across threads

---

## 8. Limitations Compared to Go

This system does NOT include:
- Work-stealing scheduler
- Dynamic stack resizing
- Preemptive scheduling
- Garbage collection

This is a simplified educational runtime.

---

## 9. Future Improvements

- Add thread-local queues
- Implement work stealing
- Add priority scheduling
- Add timeout support for channels
- Implement select with blocking support
- Add futures/promises integration

---

## 10. Learning Outcomes

After completing this project, you will understand:

- Difference between threads and goroutines
- How scheduling works internally
- How channels are implemented
- Why race conditions occur
- How deadlocks happen and how to avoid them
- How to design concurrent systems from scratch

---

## 11. Summary

This project recreates core ideas of Go’s concurrency model in C++:

- Task → goroutine equivalent
- Runtime → scheduler
- Channel → communication
- Thread pool → execution engine

The focus is not only on implementation but on understanding how concurrency works at a systems level.