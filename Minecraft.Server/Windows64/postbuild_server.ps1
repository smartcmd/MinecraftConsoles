param(
    [string]$OutDir,
    [string]$ProjectRoot,
    [string]$Configuration
)

if ([string]::IsNullOrWhiteSpace($OutDir)) {
    throw "OutDir is required."
}

if ([string]::IsNullOrWhiteSpace($ProjectRoot)) {
    $ProjectRoot = Resolve-Path (Join-Path $PSScriptRoot "..\\..")
}

if ([string]::IsNullOrWhiteSpace($Configuration)) {
    $Configuration = "Debug"
}

$OutDir = [System.IO.Path]::GetFullPath($OutDir)
$ProjectRoot = [System.IO.Path]::GetFullPath($ProjectRoot)
$ClientRoot = Join-Path $ProjectRoot "Minecraft.Client"

Write-Host "Server post-build started. OutDir: $OutDir"

function Ensure-Dir([string]$path) {
    if (-not (Test-Path $path)) {
        New-Item -ItemType Directory -Path $path -Force | Out-Null
    }
}

function Copy-Tree-IfExists([string]$src, [string]$dst) {
    if (Test-Path $src) {
        Ensure-Dir $dst
        xcopy /q /y /i /s /e /d "$src" "$dst" 2>$null | Out-Null
    }
}

function Copy-File-IfExists([string]$src, [string]$dst) {
    if (Test-Path $src) {
        $dstDir = Split-Path -Parent $dst
        Ensure-Dir $dstDir
        xcopy /q /y /d "$src" "$dstDir" 2>$null | Out-Null
    }
}

function Copy-FirstExisting([string[]]$candidates, [string]$dstFile) {
    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            Copy-File-IfExists $candidate $dstFile
            return
        }
    }
}

# Dedicated server only needs core resources for current startup path.
Copy-File-IfExists (Join-Path $ClientRoot "Common\\Media\\MediaWindows64.arc") (Join-Path $OutDir "Common\\Media\\MediaWindows64.arc")
Copy-Tree-IfExists (Join-Path $ClientRoot "Common\\res") (Join-Path $OutDir "Common\\res")
Copy-Tree-IfExists (Join-Path $ClientRoot "Windows64\\GameHDD") (Join-Path $OutDir "Windows64\\GameHDD")

# Runtime DLLs.
Copy-FirstExisting @(
    (Join-Path $ClientRoot "Windows64\\Iggy\\lib\\redist64\\iggy_w64.dll"),
    (Join-Path $ProjectRoot ("x64\\{0}\\iggy_w64.dll" -f $Configuration))
) (Join-Path $OutDir "iggy_w64.dll")

