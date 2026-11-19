$ErrorActionPreference = "Stop"
Set-Location (Split-Path -Parent $MyInvocation.MyCommand.Path)

$mingw = "D:\Qt\Tools\mingw730_64\bin"
$qt = "D:\Qt\5.14.2\mingw73_64"
if (-not (Test-Path "$qt\bin\qmake.exe")) {
  throw "未找到 Qt 5.14 MinGW kit：$qt"
}

$env:PATH = "$mingw;$qt\bin;" + $env:PATH
$env:CMAKE_SH = "CMAKE_SH-NOTFOUND"

cmake --preset mingw
if ($LASTEXITCODE -ne 0) { throw "configure failed" }
cmake --build build --parallel
if ($LASTEXITCODE -ne 0) { throw "build failed" }
& "$qt\bin\windeployqt.exe" --no-translations --compiler-runtime "build\cs2_predictor.exe"
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed" }
Write-Host "可运行：build\cs2_predictor.exe"
