# Compile Instructions

## Visual Studio (`.sln`)

1. Open `MinecraftConsoles.sln` in Visual Studio 2022.
2. Set Startup Project:
   - Client: `Minecraft.Client`
   - Dedicated server: `Minecraft.Server`
3. Select configuration:
   - `Debug` (recommended), or
   - `Release`
4. Select platform: `Windows64`.
5. Build and run:
   - `Build > Build Solution` (or `Ctrl+Shift+B`)
   - Start debugging with `F5`.

### Dedicated server debug arguments

- Default debugger arguments for `Minecraft.Server`:
  - `-port 25565 -bind 0.0.0.0 -name DedicatedServer`
- You can override arguments in:
  - `Project Properties > Debugging > Command Arguments`
- `Minecraft.Server` post-build copies only the dedicated-server asset set:
  - `Common/Media/MediaWindows64.arc`
  - `Common/res`
  - `Windows64/GameHDD`

## CMake (Windows x64)

Configure (use your VS Community instance explicitly):

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_GENERATOR_INSTANCE="C:/Program Files/Microsoft Visual Studio/2022/Community"
```

Build Debug:

```powershell
cmake --build build --config Debug --target MinecraftClient
```

Build Release:

```powershell
cmake --build build --config Release --target MinecraftClient
```

Build Dedicated Server (Debug):

```powershell
cmake --build build --config Debug --target MinecraftServer
```

Build Dedicated Server (Release):

```powershell
cmake --build build --config Release --target MinecraftServer
```

Run executable:

```powershell
cd .\build\Debug
.\MinecraftClient.exe
```

Run dedicated server:

```powershell
cd .\build\Debug
.\Minecraft.Server.exe -port 25565 -bind 0.0.0.0 -name DedicatedServer
```

Notes:
- The CMake build is Windows-only and x64-only.
- Contributors on macOS or Linux need a Windows machine or VM to build the project. Running the game via Wine is separate from having a supported build environment.
- Post-build asset copy is automatic for `MinecraftClient` in CMake (Debug and Release variants).
- The game relies on relative paths (for example `Common\Media\...`), so launching from the output directory is required.
