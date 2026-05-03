param(
  [ValidateSet("Debug","Release")]
  [string]$Config = "Debug",
  [string]$SourceDir = "d:/Go Runtime/GoRuntime/Go Runtime",
  [string]$BuildDir = "d:/Go Runtime/GoRuntime/Go Runtime/build"
)

$ErrorActionPreference = "Stop"

Write-Host "== Configure =="
cmake -S "$SourceDir" -B "$BuildDir"

Write-Host "== Build with warnings ($Config) =="
cmake --build "$BuildDir" --config $Config

Write-Host "== Done =="