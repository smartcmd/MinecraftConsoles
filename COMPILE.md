# Compile Instructions
# Compile Instructions
## Preperation
1. Download D3D11SDKLayers.dll on place it in system32
2. Install the Windows 10 sdk on the 
offical microsoft website
`https://learn.microsoft.com/en-us/windows/apps/windows-sdk/downloads`
3. Open powershell and type the following command `DISM /Online /Add-Capability /CapabilityName:Tools.Graphics.DirectX~~~~0.0.1.0`
## Visual Studio (`.sln`)

1. Open `MinecraftConsoles.sln` in Visual Studio 2022.
2. Set `Minecraft.Client` as the Startup Project.
3. Select configuration:
   - `Debug` (recommended), or
   - `Release`
4. Select platform: `Windows64`.
5. Build and run:
   - `Build > Build Solution` (or `Ctrl+Shift+B`)
   - Start debugging with `F5`.

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

Run executable:

```powershell
cd .\build\Debug
.\MinecraftClient.exe
```

Notes:
- The CMake build is Windows-only and x64-only.
- Contributors on macOS or Linux need a Windows machine or VM to build the project. Running the game via Wine is separate from having a supported build environment.
- Post-build asset copy is automatic for `MinecraftClient` in CMake (Debug and Release variants).
- The game relies on relative paths (for example `Common\Media\...`), so launching from the output directory is required.
