# Compile Instructions

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
- Post-build asset copy is automatic for `MinecraftClient` in CMake (Debug and Release variants).
- The game relies on relative paths (for example `Common\Media\...`), so launching from the output directory is required.
