param(
  [string]$BuildDir = "d:/Go Runtime/GoRuntime/Go Runtime/build",
  [int]$Workers = 8,
  [int]$Tasks = 500000,
  [int]$Capacity = 1024,
  [int]$Producers = 4,
  [int]$PerProducer = 200000
)

$ErrorActionPreference = "Stop"

$Sched = Join-Path $BuildDir "Release/scheduler_bench.exe"
$Chan = Join-Path $BuildDir "Release/channel_bench.exe"

Write-Host "== Scheduler bench =="
& $Sched $Workers $Tasks

Write-Host "== Channel bench =="
& $Chan $Capacity $Producers $PerProducer

Write-Host "== Done =="