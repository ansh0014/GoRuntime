param(
  [string]$SourceDir = "d:/Go Runtime/GoRuntime/Go Runtime"
)

$ErrorActionPreference = "Stop"

Write-Host "== clang-format =="
Get-ChildItem -Path $SourceDir -Recurse -Include *.h,*.hpp,*.cpp | ForEach-Object {
  clang-format -i $_.FullName
}

Write-Host "== Done =="