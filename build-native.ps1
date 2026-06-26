$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $Root "build-native"
$OutputDir = Join-Path $Root "Release"

Set-Location $Root

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

cmake -S $Root -B $BuildDir -DCMAKE_BUILD_TYPE=Release
cmake --build $BuildDir --config Release

if (-not (Test-Path (Join-Path $OutputDir "ClipCleaner.exe"))) {
    throw "Build finished, but Release\ClipCleaner.exe was not found."
}

Write-Host "Built $OutputDir\ClipCleaner.exe"
