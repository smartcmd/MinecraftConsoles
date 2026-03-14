# Compile Instructions

## Visual Studio

1. Clone or download the repository
1. Open the repo folder in Visual Studio 2022+.
2. Wait for cmake to configure the project and load all assets (this may take a few minutes on the first run).
3. Right click a folder in the solution explorer and switch to the 'CMake Targets View'
4. Select platform and configuration from the dropdown. EG: `Windows - Debug` or `Windows - Release`
5. Ensure `Minecraft.Client.exe` is set as the startup item, using the dropdown on the debug targets dropdown
6. Build and run the project:
   - `Build > Build Solution` (or `Ctrl+Shift+B`)
   - Start debugging with `F5`.

## CMake (Windows x64)

Configure (use your VS Community instance explicitly):

Open `Developer PowerShell for VS` and run:

```powershell
cmake --preset windows64
```

Build Debug:

```powershell
cmake --build --preset windows64-debug --target Minecraft.Client
```

Build Release:

```powershell
cmake --build --preset windows64-release --target Minecraft.Client
```

Run executable:

```powershell
cd .\build\windows64\Minecraft.Client\Debug
.\Minecraft.Client.exe
```

Notes:
- The CMake build is Windows-only and x64-only.
- Contributors on macOS or Linux need a Windows machine or VM to build the project. Running the game via Wine is separate from having a supported build environment.
- Post-build asset copy is automatic for `Minecraft.Client` in CMake (Debug and Release variants).
- The game relies on relative paths (for example `Common\Media\...`), so launching from the output directory is required.
