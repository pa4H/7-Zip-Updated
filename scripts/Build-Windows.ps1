# Added 2026-10-27. GNU LGPL 2.1 or later.
[CmdletBinding()]
param([switch]$Rebuild)
$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path -Parent $PSScriptRoot
$taskVswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $taskVswhere)) {
    throw 'Install Visual Studio Build Tools with Desktop development with C++ and a Windows SDK. See FORK.md.'
}
$taskVsPath = & $taskVswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $taskVsPath) { throw 'The MSVC x64/x86 tools are missing. See FORK.md.' }
$taskVcvars = Join-Path $taskVsPath 'VC\Auxiliary\Build\vcvars64.bat'
foreach ($taskPath in @($taskRoot, $taskVcvars)) {
    if ($taskPath -match '[&|<>^%!"\r\n]') { throw 'The build path contains unsupported cmd.exe metacharacters.' }
}
$taskNmakeFlags = if ($Rebuild) { '/nologo /A PLATFORM=x64' } else { '/nologo PLATFORM=x64' }
$taskTargets = @(
    @{ Path = 'CPP\7zip\Bundles\Format7zF'; File = '7z.dll' },
    @{ Path = 'CPP\7zip\UI\FileManager'; File = '7zFM.exe' },
    @{ Path = 'CPP\7zip\UI\GUI'; File = '7zG.exe' },
    @{ Path = 'CPP\7zip\UI\Explorer'; File = '7-zip.dll' },
    @{ Path = 'CPP\7zip\UI\Console'; File = '7z.exe' }
)
foreach ($taskTarget in $taskTargets) {
    Push-Location (Join-Path $taskRoot $taskTarget.Path)
    try {
        $taskCommand = 'call "{0}" >nul && nmake {1}' -f $taskVcvars, $taskNmakeFlags
        & $env:ComSpec /d /s /c $taskCommand
        if ($LASTEXITCODE -ne 0) { throw "Build failed: $($taskTarget.Path)" }
    } finally { Pop-Location }
}

# Use the production object files, built with the same compiler and flags.
$taskTestDir = Join-Path $taskRoot 'build\tests'
New-Item -ItemType Directory -Path $taskTestDir -Force | Out-Null
$taskFmObjects = Join-Path $taskRoot 'CPP\7zip\UI\FileManager\x64'
$taskTestObjects = @('StdAfx', 'MyString', 'MyVector', 'IntToString', 'StringConvert', 'UTFConvert', 'FileName', 'FileDir', 'FileIO', 'FileFind', 'FileSystem', 'Alloc', 'PropVariantConv', 'TimeUtils', 'ExtractingFilePath')
$taskObjectArguments = ($taskTestObjects | ForEach-Object { '"' + (Join-Path $taskFmObjects ($_.ToString() + '.obj')) + '"' }) -join ' '
Push-Location $taskTestDir
try {
    $taskCommand = 'call "{0}" >nul && cl /nologo /EHsc /W4 /WX /O2 /MT /DUNICODE /D_UNICODE "{1}" {2} /Fe:ForkBehaviorTests.exe /link /OPT:REF oleaut32.lib ole32.lib user32.lib' -f $taskVcvars, (Join-Path $taskRoot 'tests\ForkBehaviorTests.cpp'), $taskObjectArguments
    & $env:ComSpec /d /s /c $taskCommand
    if ($LASTEXITCODE -ne 0) { throw 'Behavior test compilation failed.' }
    & (Join-Path $taskTestDir 'ForkBehaviorTests.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Behavior tests failed.' }
} finally { Pop-Location }

$taskStamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$taskPackageRoot = Join-Path $taskRoot ("build\7zip-fork-1.0-x64-$taskStamp")
New-Item -ItemType Directory -Path $taskPackageRoot | Out-Null
foreach ($taskTarget in $taskTargets) {
    Copy-Item -LiteralPath (Join-Path $taskRoot ($taskTarget.Path + '\x64\' + $taskTarget.File)) -Destination $taskPackageRoot
}
New-Item -ItemType Directory -Path (Join-Path $taskPackageRoot 'Lang') | Out-Null
Copy-Item -LiteralPath (Join-Path $taskRoot 'Lang\ru.txt') -Destination (Join-Path $taskPackageRoot 'Lang\ru.txt')
foreach ($taskLicense in @('License.txt', 'copying.txt', 'unRarLicense.txt')) {
    Copy-Item -LiteralPath (Join-Path $taskRoot ('DOC\' + $taskLicense)) -Destination $taskPackageRoot
}
Copy-Item -LiteralPath (Join-Path $taskRoot 'FORK.md') -Destination $taskPackageRoot
Copy-Item -LiteralPath (Join-Path $taskRoot 'THIRD_PARTY_NOTICES.txt') -Destination $taskPackageRoot
Copy-Item -LiteralPath (Join-Path $taskRoot 'licenses') -Destination $taskPackageRoot -Recurse

# Accompany binaries with the exact working sources used for this build,
# including pending changes and new files; never use HEAD as a substitute.
Add-Type -AssemblyName System.IO.Compression.FileSystem
$taskSourceZip = $taskPackageRoot + '-source.zip'
$taskZip = [System.IO.Compression.ZipFile]::Open($taskSourceZip, [System.IO.Compression.ZipArchiveMode]::Create)
Push-Location $taskRoot
try {
    $taskSources = & git -c core.quotepath=false ls-files --cached --others --exclude-standard
    if ($LASTEXITCODE -ne 0) { throw 'Cannot enumerate corresponding sources.' }
    foreach ($taskSource in $taskSources) {
        # Keep the extracted Windows Up icon a local redrawing reference.
        if ($taskSource.Replace('\', '/') -like 'assets/up-reference/*') { continue }
        $taskSourcePath = Join-Path $taskRoot $taskSource
        if (Test-Path -LiteralPath $taskSourcePath -PathType Leaf) {
            [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($taskZip, $taskSourcePath, $taskSource.Replace('\', '/')) | Out-Null
        }
    }
} finally {
    Pop-Location
    $taskZip.Dispose()
}
Compress-Archive -LiteralPath $taskPackageRoot -DestinationPath ($taskPackageRoot + '-binaries.zip')
Write-Output "Behavior tests passed. Local package: $taskPackageRoot"
Write-Output "Corresponding sources: $taskSourceZip"
Write-Output 'Local build and corresponding sources are ready; see FORK.md for distribution notices.'
