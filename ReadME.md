# Go Runtime in C++

A production-oriented learning project that recreates core ideas from Go concurrency in C++.

## What This Project Builds
- Lightweight task execution model (goroutine-like tasks)
- Thread-pool scheduler (runtime-managed execution)
- Buffered and unbuffered channels
- Select-style multi-channel waiting
- Deterministic shutdown and lifecycle handling

## Why This Exists
Go provides goroutines, scheduling, and channels natively.  
C++ gives low-level building blocks, but these abstractions must be built manually.  
This project helps you deeply understand runtime internals by implementing them directly.

## Current Repository Layout
- Go Runtime/
- Go Runtime/include/goruntime
- Go Runtime/src/runtime
- Go Runtime/src/channel
- Go Runtime/src/core
- Go Runtime/tests/unit
- Go Runtime/tests/integration
- Go Runtime/tests/stress
- Go Runtime/benchmarks
- Go Runtime/docs
- Go Runtime/scripts

## Planned Feature Set
1. Runtime + scheduler + worker pool
2. Thread-safe task queue
3. Buffered channel
4. Unbuffered channel
5. Non-blocking operations and select
6. Metrics and diagnostic hooks
7. Stress testing and benchmarking

## Engineering Standards
- Correctness first, then optimization
- No data races in sanitizer runs
- Predictable shutdown behavior
- Explicit synchronization boundaries
- Tests for core concurrency paths

## Build (Planned CMake Flow)
```powershell
cmake -S "Go Runtime" -B "Go Runtime/build"
cmake --build "Go Runtime/build" --config Release
ctest --test-dir "Go Runtime/build" --output-on-failure
