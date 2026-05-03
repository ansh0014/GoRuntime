param(
  [ValidateSet("Debug","Release")]
  [string]$Config = "Debug",
  [string]$SourceDir = "d:/Go Runtime/GoRuntime/Go Runtime",
  [string]$BuildDir = "d:/Go Runtime/GoRuntime/Go Runtime/build"
)

$ErrorActionPreference = "Stop"

Write-Host "== Configure =="
cmake -S "$SourceDir" -B "$BuildDir"

Write-Host "== Build ($Config) =="
cmake --build "$BuildDir" --config $Config

Write-Host "== Test ($Config) =="
ctest --test-dir "$BuildDir" -C $Config --output-on-failure

Write-Host "== Done =="