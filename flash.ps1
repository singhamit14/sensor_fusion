# ******************************************************************************
#  flash.ps1 - build + flash sensor_fusion to the NUCLEO-F767ZI over ST-Link (CN1)
#
#  Usage:
#     .\flash.ps1              # build Debug, flash, verify, reset
#     .\flash.ps1 -Release     # same, Release preset
#     .\flash.ps1 -NoBuild     # flash existing ELF only
#     .\flash.ps1 -Test        # run host tests (test/host) first; abort on failure
#     .\flash.ps1 -Erase       # full chip erase before programming
# ******************************************************************************
param(
  [switch]$Release,
  [switch]$NoBuild,
  [switch]$Test,
  [switch]$Erase
)

$ErrorActionPreference = "Stop"
Set-Location -Path $PSScriptRoot

$Preset     = if ($Release) { "Release" } else { "Debug" }
$BuildDir   = "build\$Preset"
$Elf        = "$BuildDir\sensor_fusion.elf"
$Programmer = "C:\ST\STM32CubeCLT_1.23.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
$MinGW      = "C:\MinGW\bin"
$Conn       = @("-c", "port=SWD", "freq=4000")

if (-not (Test-Path $Programmer)) { throw "STM32_Programmer_CLI not found: $Programmer" }

# --- Host tests (optional) ----------------------------------------------------
if ($Test) {
  Write-Host "==> Host tests..." -ForegroundColor Cyan
  $oldPath = $env:Path
  $env:Path = "$MinGW;$env:Path"
  Push-Location test\host
  try {
    & "$MinGW\mingw32-make.exe"
    if ($LASTEXITCODE -ne 0) { throw "Host tests FAILED - not flashing" }
  } finally {
    Pop-Location
    $env:Path = $oldPath
  }
}

# --- Build --------------------------------------------------------------------
if (-not $NoBuild) {
  # Configure only when the build dir is missing (first run or after deleting build\)
  if (-not (Test-Path "$BuildDir\build.ninja")) {
    Write-Host "==> Configuring ($Preset)..." -ForegroundColor Cyan
    cmake --preset $Preset
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed (exit $LASTEXITCODE)" }
  }
  Write-Host "==> Building ($Preset)..." -ForegroundColor Cyan
  cmake --build --preset $Preset
  if ($LASTEXITCODE -ne 0) { throw "Build failed (exit $LASTEXITCODE)" }
}

if (-not (Test-Path $Elf)) { throw "Firmware not found: $Elf  (run without -NoBuild first)" }

# --- Erase (optional) ---------------------------------------------------------
if ($Erase) {
  Write-Host "==> Full chip erase..." -ForegroundColor Yellow
  & $Programmer @Conn -e all
  if ($LASTEXITCODE -ne 0) { throw "Erase failed (exit $LASTEXITCODE)" }
}

# --- Flash + verify + reset ---------------------------------------------------
Write-Host "==> Flashing $Elf ..." -ForegroundColor Cyan
& $Programmer @Conn -w $Elf -v -rst
if ($LASTEXITCODE -ne 0) { throw "Flash failed (exit $LASTEXITCODE) - board on CN1? another tool holding ST-Link?" }

Write-Host "==> Done. Firmware flashed and running." -ForegroundColor Green