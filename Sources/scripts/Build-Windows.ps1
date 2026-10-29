# Added 2026-10-27. GNU LGPL 2.1 or later.
# Native installer packaging added 2026-10-28.
# Installer creation made explicit opt-in on 2026-10-29.
[CmdletBinding()]
param([switch]$Rebuild, [switch]$Installer)
$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path -Parent $PSScriptRoot
New-Item -ItemType Directory -Path (Join-Path $taskRoot '.cache') -Force | Out-Null
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
$taskTargets = @(
    @{ Path = 'CPP\7zip\Bundles\Format7zF'; File = '7z.dll' },
    @{ Path = 'CPP\7zip\UI\FileManager'; File = '7zFM.exe' },
    @{ Path = 'CPP\7zip\UI\GUI'; File = '7zG.exe' },
    @{ Path = 'CPP\7zip\UI\Explorer'; File = '7-zip.dll' },
    @{ Path = 'CPP\7zip\UI\Console'; File = '7z.exe' },
    @{ Path = 'CPP\7zip\UI\Explorer'; File = '7-zip32.dll'; Output = '7-zip.dll'; Architecture = 'x86' },
    @{ Path = 'C\Util\7zipUninstall'; File = 'Uninstall.exe'; Output = '7zipUninstall.exe'; Installer = $true },
    @{ Path = 'C\Util\7zipInstall'; File = '7zipInstall.exe'; Stub = $true; Installer = $true }
)
if (-not $Installer) {
    $taskTargets = @($taskTargets | Where-Object { -not $_.Installer })
}
foreach ($taskTarget in $taskTargets) {
    $taskArchitecture = if ($taskTarget.Architecture) { $taskTarget.Architecture } else { 'x64' }
    $taskTargetVcvars = if ($taskArchitecture -eq 'x86') { Join-Path $taskVsPath 'VC\Auxiliary\Build\vcvars32.bat' } else { $taskVcvars }
    $taskNmakeFlags = if ($Rebuild) { '/nologo /A PLATFORM=' + $taskArchitecture } else { '/nologo PLATFORM=' + $taskArchitecture }
    Push-Location (Join-Path $taskRoot $taskTarget.Path)
    try {
        $taskCommand = 'call "{0}" >nul && nmake {1}' -f $taskTargetVcvars, $taskNmakeFlags
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
    if ($taskTarget.Stub) { continue }
    $taskArchitecture = if ($taskTarget.Architecture) { $taskTarget.Architecture } else { 'x64' }
    $taskOutputFile = if ($taskTarget.Output) { $taskTarget.Output } else { $taskTarget.File }
    Copy-Item -LiteralPath (Join-Path $taskRoot ($taskTarget.Path + '\' + $taskArchitecture + '\' + $taskOutputFile)) -Destination (Join-Path $taskPackageRoot $taskTarget.File)
}
New-Item -ItemType Directory -Path (Join-Path $taskPackageRoot 'Lang') | Out-Null
Copy-Item -LiteralPath (Join-Path $taskRoot 'Lang\ru.txt') -Destination (Join-Path $taskPackageRoot 'Lang\ru.txt')
foreach ($taskLicense in @('License.txt', 'copying.txt', 'unRarLicense.txt')) {
    Copy-Item -LiteralPath (Join-Path $taskRoot ('DOC\' + $taskLicense)) -Destination $taskPackageRoot
}
Copy-Item -LiteralPath (Join-Path $taskRoot 'FORK.md') -Destination $taskPackageRoot
Copy-Item -LiteralPath (Join-Path $taskRoot 'THIRD_PARTY_NOTICES.txt') -Destination $taskPackageRoot
Copy-Item -LiteralPath (Join-Path $taskRoot 'licenses') -Destination $taskPackageRoot -Recurse

# The native C installer supports LZMA (not LZMA2); disable automatic filters
# and header compression so the exact payload is readable by its decoder.
if ($Installer) {
    $taskPayloadPath = Join-Path $taskRoot ('.cache\installer-' + $taskStamp + '.7z')
    $taskSetupPath = $taskPackageRoot + '-setup.exe'
    Push-Location $taskPackageRoot
    try {
        & (Join-Path $taskPackageRoot '7z.exe') a -t7z -m0=LZMA -mf=off -mhc=off -mx=9 -ms=on $taskPayloadPath '.\*'
        if ($LASTEXITCODE -ne 0) { throw 'Installer payload compression failed.' }
    } finally { Pop-Location }
    $taskSetupStream = [IO.File]::Create($taskSetupPath)
    try {
        foreach ($taskPart in @((Join-Path $taskRoot 'C\Util\7zipInstall\x64\7zipInstall.exe'), $taskPayloadPath)) {
            $taskPartStream = [IO.File]::OpenRead($taskPart)
            try { $taskPartStream.CopyTo($taskSetupStream) } finally { $taskPartStream.Dispose() }
        }
    } finally { $taskSetupStream.Dispose() }
    & (Join-Path $taskPackageRoot '7z.exe') t -t7z $taskSetupPath
    if ($LASTEXITCODE -ne 0) { throw 'Embedded installer archive verification failed.' }
    & (Join-Path $PSScriptRoot 'Test-Installer.ps1') -PackagePath $taskPackageRoot -PayloadPath $taskPayloadPath
}

# Accompany binaries with the exact working sources used for this build,
# including pending changes and new files; never use HEAD as a substitute.
Add-Type -AssemblyName System.IO.Compression.FileSystem
$taskSourceZip = $taskPackageRoot + '-source.zip'
$taskZip = [System.IO.Compression.ZipFile]::Open($taskSourceZip, [System.IO.Compression.ZipArchiveMode]::Create)
$taskRepositoryRoot = if ((Split-Path -Leaf $taskRoot) -eq 'Sources') { Split-Path -Parent $taskRoot } else { $taskRoot }
Push-Location $taskRepositoryRoot
try {
    $taskHasRepository = $false
    if (Get-Command git -ErrorAction SilentlyContinue) {
        try {
            $taskGitRoot = & git rev-parse --show-toplevel 2>$null
            $taskHasRepository = $LASTEXITCODE -eq 0 -and [IO.Path]::GetFullPath($taskGitRoot) -eq [IO.Path]::GetFullPath($taskRepositoryRoot)
        } catch { $taskHasRepository = $false }
    }
    if ($taskHasRepository) {
        $taskSources = & git -c core.quotepath=false ls-files --cached --others --exclude-standard
        if ($LASTEXITCODE -ne 0) { throw 'Cannot enumerate corresponding sources.' }
    } else {
        # A downloaded source archive has no .git directory. Keep it buildable
        # and avoid collecting compiler outputs or unrelated outer repositories.
        $taskSources = Get-ChildItem -LiteralPath $taskRepositoryRoot -Recurse -File -Force | ForEach-Object {
            $taskRelative = $_.FullName.Substring($taskRepositoryRoot.Length).TrimStart('\').Replace('\', '/')
            if ($taskRelative -notmatch '(^|/)(\.git|\.cache|build|dist)(/|$)' -and
                $taskRelative -notmatch '(^|/)assets/up-reference/' -and
                $_.Extension -notin @('.obj','.exe','.dll','.lib','.exp','.res','.pch','.pdb','.ilk')) {
                $taskRelative
            }
        }
    }
    foreach ($taskSource in $taskSources) {
        # Keep the extracted Windows Up icon a local redrawing reference.
        if ($taskSource.Replace('\', '/') -match '(^|/)assets/up-reference/') { continue }
        $taskSourcePath = Join-Path $taskRepositoryRoot $taskSource
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
if ($Installer) { Write-Output "Native installer (replaces 7-Zip): $taskSetupPath" }
Write-Output 'Local build and corresponding sources are ready; see FORK.md for distribution notices.'
