# Added 2026-10-28. GNU LGPL 2.1 or later.
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$PackagePath,
    [Parameter(Mandatory)][string]$PayloadPath
)
$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path -Parent $PSScriptRoot
$taskBuildRoot = [IO.Path]::GetFullPath((Join-Path $taskRoot 'build')) + '\'
$taskPackage = (Resolve-Path -LiteralPath $PackagePath).Path
$taskPayload = (Resolve-Path -LiteralPath $PayloadPath).Path
if (-not $taskPackage.StartsWith($taskBuildRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Installer tests require a package inside this workspace build directory.'
}
$taskVswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$taskVsPath = & $taskVswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $taskVsPath) { throw 'MSVC x64/x86 tools are required.' }
$taskVcvars = Join-Path $taskVsPath 'VC\Auxiliary\Build\vcvars64.bat'
$taskTestRoot = Join-Path $taskRoot ('build\installer-checks-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
foreach ($taskPath in @($taskRoot, $taskVcvars, $taskTestRoot)) {
    if ($taskPath -match '[&|<>^%!"\r\n]') { throw 'Unsupported cmd.exe metacharacters in test path.' }
}
foreach ($taskSubdir in @('', 'Installed\Lang', 'CommonStartMenu\7-Zip', 'UserStartMenu')) {
    New-Item -ItemType Directory -Path (Join-Path $taskTestRoot $taskSubdir) -Force | Out-Null
}
foreach ($taskRelative in @('Lang\de.txt', 'Lang\fr.txt', 'Lang\en.ttt', '7-zip.chm')) {
    Set-Content -LiteralPath (Join-Path $taskTestRoot ('Installed\' + $taskRelative)) -Value 'old upstream file'
}
$taskReadOnly = Get-Item -LiteralPath (Join-Path $taskTestRoot 'Installed\Lang\de.txt')
$taskReadOnly.IsReadOnly = $true
Set-Content -LiteralPath (Join-Path $taskTestRoot 'Installed\user-file.txt') -Value 'preserve user file'
Set-Content -LiteralPath (Join-Path $taskTestRoot 'CommonStartMenu\7-Zip\7-Zip Help.lnk') -Value 'old help link'

foreach ($taskMode in @('Install', 'Uninstall')) {
    $taskExtraDefine = if ($taskMode -eq 'Uninstall') { '/DZ7_TEST_UNINSTALL' } else { '' }
    $taskObjects = if ($taskMode -eq 'Install') {
        (Get-ChildItem -LiteralPath (Join-Path $taskRoot 'C\Util\7zipInstall\x64') -Filter '*.obj' |
            Where-Object Name -ne '7zipInstall.obj' | ForEach-Object { '"' + $_.FullName + '"' }) -join ' '
    } else { '' }
    $taskRunner = Join-Path $taskTestRoot ($taskMode + 'Integration.exe')
    $taskCommand = 'call "{0}" >nul && cl /nologo /W4 /WX /O2 /MT /Gy /D_WIN32_WINNT=0x0A00 /D_CRT_SECURE_NO_WARNINGS {1} "{2}" {3} /Fe:"{4}" /Fo:"{5}" /link /SUBSYSTEM:CONSOLE /OPT:REF /INCREMENTAL:NO ole32.lib oleaut32.lib user32.lib advapi32.lib shell32.lib uuid.lib' -f $taskVcvars, $taskExtraDefine, (Join-Path $taskRoot 'tests\InstallerIntegrationTests.c'), $taskObjects, $taskRunner, (Join-Path $taskTestRoot ($taskMode + 'Integration.obj'))
    & $env:ComSpec /d /s /c $taskCommand
    if ($LASTEXITCODE -ne 0) { throw "$taskMode integration runner compilation failed." }
    if ($taskMode -eq 'Install') {
        $taskStream = [IO.File]::Open($taskRunner, [IO.FileMode]::Append)
        $taskInput = [IO.File]::OpenRead($taskPayload)
        try { $taskInput.CopyTo($taskStream) } finally { $taskInput.Dispose(); $taskStream.Dispose() }
    }
    & $taskRunner $taskTestRoot
    if ($LASTEXITCODE -ne 0) { throw "$taskMode integration tests failed." }
    foreach ($taskFile in (Get-ChildItem -LiteralPath $taskPackage -Recurse -File)) {
        $taskRelative = $taskFile.FullName.Substring($taskPackage.Length).TrimStart('\')
        $taskInstalledFile = Join-Path $taskTestRoot ('Installed\' + $taskRelative)
        if ($taskMode -eq 'Install') {
            if (-not (Test-Path -LiteralPath $taskInstalledFile -PathType Leaf) -or
                (Get-FileHash -LiteralPath $taskInstalledFile).Hash -ne (Get-FileHash -LiteralPath $taskFile.FullName).Hash) {
                throw "Installed content differs: $taskRelative"
            }
        } elseif (Test-Path -LiteralPath $taskInstalledFile) {
            throw "Uninstaller left a packaged file: $taskRelative"
        }
    }
    foreach ($taskRelative in @('Lang\de.txt', 'Lang\fr.txt', 'Lang\en.ttt', '7-zip.chm')) {
        if (Test-Path -LiteralPath (Join-Path $taskTestRoot ('Installed\' + $taskRelative))) {
            throw "Obsolete upstream file remains: $taskRelative"
        }
    }
    if ((Get-Content -LiteralPath (Join-Path $taskTestRoot 'Installed\user-file.txt') -Raw).Trim() -ne 'preserve user file') {
        throw 'A user file was changed or removed.'
    }
}
Write-Output 'Native installer tests passed: payload hashes, upgrade cleanup, registry, local shortcuts, uninstallation, and user-file preservation.'
Write-Output "Isolated test files: $taskTestRoot"
