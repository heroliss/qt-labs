param([string]$QtRoot=$env:QT_ROOT,[switch]$SkipBuild)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
if(!$QtRoot){$QtRoot=Join-Path $repo '.toolchain/Qt/6.12.0/msvc2022_64'}
if(!$SkipBuild){& (Join-Path $PSScriptRoot 'build.ps1') -QtRoot $QtRoot}
$tag='Liss-QtLabs-1.0.0-win-x64-'+(Get-Date -Format 'yyyyMMdd-HHmmss')
$out=Join-Path $repo "artifacts/$tag"
if(Test-Path -LiteralPath $out){throw 'Package directory already exists; refusing to overwrite it.'}
New-Item -ItemType Directory -Path $out -Force | Out-Null
$apps=@('LissOsLab.exe','LissHuffmanLab.exe','LissCalculator.exe','QtLabsChecks.exe')
foreach($app in $apps){Copy-Item -LiteralPath (Join-Path $repo "build/Release/$app") -Destination (Join-Path $out $app)}
$env:PATH=(Join-Path $QtRoot 'bin')+';'+$env:PATH
$binaries=$apps | ForEach-Object {Join-Path $out $_}
& (Join-Path $QtRoot 'bin/windeployqt.exe') --release --no-patchqt --no-compiler-runtime --no-translations --no-system-d3d-compiler --no-system-dxc-compiler --no-opengl-sw --dir $out --plugindir (Join-Path $out 'plugins') @binaries
if($LASTEXITCODE -ne 0){throw 'Qt deployment failed'}
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$runtime=Get-ChildItem -LiteralPath (Join-Path $vs 'VC/Redist/MSVC') -Directory | Where-Object {$_.Name -match '^\d+\.'} | Sort-Object Name -Descending | Select-Object -First 1
$crt=Get-ChildItem -LiteralPath (Join-Path $runtime.FullName 'x64') -Directory -Filter '*.CRT' | Select-Object -First 1
if(!$crt){throw 'Redistributable x64 MSVC CRT not found'}
Get-ChildItem -LiteralPath $crt.FullName -Filter '*.dll' -File | Copy-Item -Destination $out
[System.IO.File]::WriteAllText((Join-Path $out 'qt.conf'),"[Paths]`nPrefix=.`nPlugins=plugins`n",[System.Text.UTF8Encoding]::new($false))
Copy-Item -LiteralPath (Join-Path $repo 'README.md'),(Join-Path $repo 'THIRD_PARTY_NOTICES.md') -Destination $out
Copy-Item -LiteralPath (Join-Path $repo 'docs') -Destination (Join-Path $out 'docs') -Recurse
Copy-Item -LiteralPath (Join-Path $repo 'third-party') -Destination (Join-Path $out 'third-party') -Recurse
$source=Join-Path $repo '.toolchain/qtbase-everywhere-src-6.12.0.tar.xz'
$expected='a951bd163c7b80fc6b8c88d7668fb56abf91c152373e13c10666763238131307'
if(!(Test-Path -LiteralPath $source) -or (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected){throw 'Run scripts/qt_notices.py to acquire and verify the exact Qt source.'}
New-Item -ItemType Directory -Path (Join-Path $out 'third-party/sources') -Force | Out-Null
Copy-Item -LiteralPath $source -Destination (Join-Path $out 'third-party/sources')
$oldPath=$env:PATH
try{
  $env:PATH="$env:WINDIR\System32;$env:WINDIR"
  $check=Start-Process -FilePath (Join-Path $out 'QtLabsChecks.exe') -WorkingDirectory $env:TEMP -WindowStyle Hidden -Wait -PassThru -RedirectStandardOutput (Join-Path $out 'core-acceptance.json') -RedirectStandardError (Join-Path $out 'core-check-stderr.txt')
  if($check.ExitCode -ne 0){throw "Packaged core checks failed: $($check.ExitCode)"}
}finally{$env:PATH=$oldPath}
$records=Get-ChildItem -LiteralPath $out -Recurse -File | ForEach-Object {[ordered]@{path=[System.IO.Path]::GetRelativePath($out,$_.FullName).Replace('\','/');bytes=$_.Length;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}}
$manifest=[ordered]@{version='1.0.0';qt='6.12.0';windows='x64';apps=$apps[0..2];files=@($records)}
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $out 'manifest.json') -Encoding utf8
$zip="$out.zip"
Compress-Archive -LiteralPath $out -DestinationPath $zip -CompressionLevel Optimal
$result=[ordered]@{version='1.0.0';directory=$out;zip=$zip;bytes=(Get-Item -LiteralPath $zip).Length;sha256=(Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant();apps=@($apps[0..2] | ForEach-Object {Join-Path $out $_})}
$result | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $repo 'artifacts/latest-package.json') -Encoding utf8
$result | ConvertTo-Json -Depth 5
