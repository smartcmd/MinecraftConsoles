$sha = (git rev-parse --short=7 HEAD)

if ($env:GITHUB_REPOSITORY) {
    $ref = "$env:GITHUB_REPOSITORY/$(git symbolic-ref --short HEAD)"
} else {
    $remoteUrl = (git remote get-url origin)
    # handle github urls only, can't predict other origins behavior
    if ($remoteUrl -match '(?:github\.com[:/])([^/:]+/[^/]+?)(?:\.git)?$') { 
        $ref = "$($matches[1])/$(git symbolic-ref --short HEAD)"
    }else{
        # fallback to just symbolic ref in case remote isnt what we expect
        $ref = "UNKNOWN/$(git symbolic-ref --short HEAD)"
    }
}

$build = 560 # Note: Build/network has to stay static for now, as without it builds wont be able to play together. We can change it later when we have a better versioning scheme in place.
$suffix = ""

# TODO Re-enable
# If we are running in GitHub Actions, use the run number as the build number
# if ($env:GITHUB_RUN_NUMBER) {
#     $build = $env:GITHUB_RUN_NUMBER
# }

# If we have uncommitted changes, add a suffix to the version string
if (git status --porcelain) {
    $suffix = "-dev"
}

@"
#pragma once

#define VER_PRODUCTBUILD $build
#define VER_PRODUCTVERSION_STR_W L"$sha$suffix"
#define VER_FILEVERSION_STR_W VER_PRODUCTVERSION_STR_W
#define VER_BRANCHVERSION_STR_W L"$ref"
#define VER_NETWORK VER_PRODUCTBUILD
"@ | Set-Content "Common/BuildVer.h"
