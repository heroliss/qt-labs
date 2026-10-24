param([ValidateSet('Debug','Release')][string]$Configuration='Release',[string]$QtRoot=$env:QT_ROOT)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
if(!$QtRoot){$QtRoot=Join-Path $repo '.toolchain/Qt/6.12.0/msvc2022_64'}
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(!$vs){throw 'Install current Visual Studio C++ desktop tools.'}
$cmake=Join-Path $vs 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
if(!(Test-Path -LiteralPath (Join-Path $QtRoot 'lib/cmake/Qt6/Qt6Config.cmake'))){throw "Qt SDK not found at $QtRoot"}
& $cmake -S $repo -B (Join-Path $repo 'build') -G 'Visual Studio 18 2026' -A x64 "-DCMAKE_PREFIX_PATH=$QtRoot"
if($LASTEXITCODE -ne 0){throw 'CMake configuration failed'}
& $cmake --build (Join-Path $repo 'build') --config $Configuration --parallel 3
if($LASTEXITCODE -ne 0){throw 'Build failed'}
$env:PATH=(Join-Path $QtRoot 'bin')+';'+$env:PATH
& (Join-Path $repo "build/$Configuration/QtLabsChecks.exe")
if($LASTEXITCODE -ne 0){throw 'Core acceptance failed'}
