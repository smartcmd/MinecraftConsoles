# MinecraftConsoles

[![Discord](https://img.shields.io/badge/Discord-Join%20Server-5865F2?logo=discord&logoColor=white)](https://discord.gg/5CSzhc9t)

![img.png](img.png)

## Introduction

This project contains the source code of Minecraft Legacy Console Edition v1.3.0494.0, with some fixes and improvements applied.

## Features

- Fixed compilation and execution in both Debug and Release mode on Windows using Visual Studio 2022
- Added support for keyboard and mouse input
- Added fullscreen mode support (toggle using F11)
- Disabled V-Sync for better performance
- Auto-detect native monitor resolution with DPI awareness, resulting in sharper visuals on high-resolution displays

## Build & Run

1. Install Visual Studio 2022
2. Clone the repository
3. Open the project by double-clicking `MinecraftConsoles.sln`
4. Make sure `Minecraft.Client` is set as the Startup Project
5. Set the build configuration to **Debug** (Release is also OK but has some bugs) and the target platform to **Windows64**, then build and run

## Known Issues

- Builds for other platforms have not been tested and are most likely non-functional
- There are some render bugs in the Release mode build
