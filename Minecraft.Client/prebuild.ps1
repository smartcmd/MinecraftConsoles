$sha = (git rev-parse --short=7 HEAD)
$ref = (git symbolic-ref HEAD)
$build = -1
$suffix = ""

# If we are running in GitHub Actions, use the run number as the build number
if ($env:GITHUB_RUN_NUMBER) {
    $build = $env:GITHUB_RUN_NUMBER
}

# If we have uncommitted changes, add a suffix to the version string
if (git status --porcelain) {
    $suffix = "-dev"
}

@"
#pragma once

#define VER_PRODUCTBUILD $build
#define VER_PRODUCTVERSION_STR_W L"$sha$suffix ($ref)"
#define VER_FILEVERSION_STR_W VER_PRODUCTVERSION_STR_W
#define VER_NETWORK VER_PRODUCTBUILD
"@ | Set-Content "Common/BuildVer.h"
