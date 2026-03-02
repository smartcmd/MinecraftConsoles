# MinecraftConsoles

[![Discord](https://img.shields.io/badge/Discord-Join%20Server-5865F2?logo=discord&logoColor=white)](https://discord.gg/5CSzhc9t)

![img.png](.github/IMG_8725.png)

## Introduction

This project contains the source code of Minecraft Legacy Console Edition v1.3.0494.0 from https://archive.org/details/minecraft-legacy-console-edition-source-code, with some fixes and improvements applied

[Nightly Build](https://github.com/smartcmd/MinecraftConsoles/releases/tag/nightly)

## Features

- Fixed compilation and execution in both Debug and Release mode on Windows using Visual Studio 2022
- Added support for keyboard and mouse input
- Added fullscreen mode support (toggle using F11)
- Disabled V-Sync for better performance
- Added a high-resolution timer path on Windows for smoother high-FPS gameplay timing
- Device's screen resolution will be used as the game resolution instead of using a fixed resolution (1920x1080)
- Added Windows64 LAN & Target Server multiplayer (host/join via in-game Join Game menu)

## Multiplayer (Windows64)

The multiplayer implementation was added with reference to:

- [LCEMP/LCEMP](https://github.com/LCEMP/LCEMP)

### Command Line Options

- `-name <PlayerName>`: Set the local player name (max 16 chars)
- `-host <IP:PORT>`: Set host bind address/port when creating a game (default port: `25565`)
- `-target <IP:PORT>`: Add a direct server candidate to the Join Game list (default port: `25565`)

### Usage Example

Host:

```powershell
Minecraft.Client.exe -name HostPlayer -host 0.0.0.0:25565
```

Join:

```powershell
Minecraft.Client.exe -name ClientPlayer -target 192.168.0.10:25565
```

Then open **Play Game** -> **Join Game** in-game and select the discovered/target session.

## Controls (Keyboard & Mouse)

- **Movement**: `W` `A` `S` `D`
- **Jump / Fly (Up)**: `Space`
- **Sneak / Fly (Down)**: `Shift` (Hold)
- **Sprint**: `Ctrl` (Hold) or Double-tap `W`
- **Inventory**: `E`
- **Drop Item**: `Q`
- **Crafting**: `C`
- **Toggle View (FPS/TPS)**: `F5`
- **Fullscreen**: `F11`
- **Pause Menu**: `Esc`
- **Toggle Mouse Capture**: `Left Alt` (for debugging)
- **Attack / Destroy**: `Left Click`
- **Use / Place**: `Right Click`
- **Select Item**: `Mouse Wheel` or keys `1` to `9`
- **Accept or Decline Tutorial hints**: `Enter` to accept and `B` to decline
- **Host Options**: `TAB`
- **Toggle Debug Info**: `F3`
- **Open Debug Overlay**: `F4`

## Build & Run

1. Install Visual Studio 2022
2. Clone the repository
3. Open the project by double-clicking `MinecraftConsoles.sln`
4. Make sure `Minecraft.Client` is set as the Startup Project
5. Set the build configuration to **Debug** (Release is also OK but has some bugs) and the target platform to **Windows64**, then build and run

### CMake (Windows x64)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --target MinecraftClient
```

For more informatio, see [COMPILE.md](COMPILE.md)

## Known Issues

- Builds for other platforms have not been tested and are most likely non-functional
- There are some render bugs in the Release mode build

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=smartcmd/MinecraftConsoles&type=date&legend=top-left)](https://www.star-history.com/?spm=a2c6h.12873639.article-detail.7.7b9d7fabjNxTRk#smartcmd/MinecraftConsoles&type=date&legend=top-left)
