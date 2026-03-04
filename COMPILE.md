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

## CMake (Linux)

MinecraftConsoles does not support Linux but can cross-compile a Windows build from Linux.

You will need a Windows MSVC SDK. You can get one from [msvc-wine](https://github.com/mstorsjo/msvc-wine).

Configure:
```
MinecraftConsoles $ mkdir build && cd build
MinecraftConsoles/build $ cmake .. -B . -G Ninja -DCMAKE_TOOLCHAIN_FILE=../cmake/LinuxCrosscompile.cmake -DCMAKE_BUILD_TYPE=<Debug or Release>
```

Build:
```
MinecraftConsoles/build $ ninja
```

Run:
```
MinecraftConsoles/build $ wine MinecraftClient.exe
```

Notes:
- The CMake build will only compile a Windows binary, and non-win32 systems will cross-compile one.
- Contributors on macOS or Linux should be able to cross-compile a windows build. Running the game via Wine is separate from having a supported build environment.
- Post-build asset copy is automatic for `MinecraftClient` in CMake (Debug and Release variants).
- The game relies on relative paths (for example `Common\Media\...`), so launching from the output directory is required.
