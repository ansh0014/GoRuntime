# Go Runtime in C++

A production-oriented, educational runtime that recreates core concurrency ideas from Go in modern C++.
It provides a task scheduler, buffered and unbuffered channels, and a select mechanism with timeouts.

## What This Is
- A small runtime that executes user tasks over a fixed worker pool.
- A channel system for safe inter-task communication.
- A learning-first but production-grade implementation with tests, stress tests, and benchmarks.

## What This Is Not
- A full Go runtime replacement.
- A work-stealing scheduler, GC, or preemptive scheduling system.
- A goroutine stack model with dynamic growth.

## Similarities to Go Runtime
- Goroutine-like tasks executed by a runtime scheduler.
- Channels with buffered and unbuffered semantics.
- Select across multiple channels with timeout support.
- Deterministic shutdown behavior.

## Differences from Go Runtime
- Uses OS threads directly (no goroutine stacks).
- No garbage collector or runtime-managed memory.
- No P/M/G scheduler architecture or work stealing.
- Select is polling-based rather than fully blocking.

## Features
- Fixed-size worker pool scheduler
- Thread-safe task queue
- Buffered and unbuffered channels
- Select with timeout support
- Unit, integration, and stress tests
- Benchmarks and automation scripts

## Build and Test

### Debug
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

### Release
```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

#### Stress

for ($i=1; $i -le 10; $i++) {
  Write-Host "Stress run $i"
  ctest --test-dir build -C Debug -R stress --output-on-failure
}

### Benchmarks
## Scheduler
workers=8, tasks=500000
time_us=1133741
throughput_tasks_per_sec=441017

## Channel

capacity=1024, producers=4, per_producer=200000
messages=800000, sum=80001600000
time_us=366392
throughput_msgs_per_sec=2183453

![alt text](<Screenshot 2026-05-03 154513.png>)

### Scripts

powershell -ExecutionPolicy Bypass -File "scripts/test.ps1" -Config Debug
powershell -ExecutionPolicy Bypass -File "scripts/test.ps1" -Config Release
powershell -ExecutionPolicy Bypass -File "scripts/bench.ps1"
powershell -ExecutionPolicy Bypass -File "scripts/lint.ps1" -Config Debug
powershell -ExecutionPolicy Bypass -File "scripts/format.ps1"

Status
All core features implemented and verified with unit, integration, stress, and benchmark runs.