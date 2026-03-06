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

## Building on Linux

In order to build on linux you'll need the following dependencies (These pkg names are for arch):

```
git
cmake
extra-cmake-modules
python
wine
msitools
ca-certificates
libwbclient
```

Once you've installed those you can run this after cloning the repository which will build via [msvc-wine](https://github.com/mstorsjo/msvc-wine) 

```sh
export WINE=$(command -v wine64 || command -v wine || false)
$WINE wineboot
git clone https://github.com/mstorsjo/msvc-wine
./msvc-wine/vsdownload.py --accept-license --dest $(pwd)/my_msvc/opt/msvc
./msvc-wine/install.sh $(pwd)/my_msvc/opt/msvc
export PATH=$(pwd)/my_msvc/opt/msvc/bin/x64:$PATH
CC=cl CXX=cl cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Windows
make -j$(nproc)
```
