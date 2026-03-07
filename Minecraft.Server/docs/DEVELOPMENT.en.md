# Minecraft.Server Developer Guide (English)

This document is for contributors who are new to `Minecraft.Server` and need a practical map for adding or modifying features safely.

## 1. What This Server Does

`Minecraft.Server` is the dedicated-server executable entry for this codebase.

Core responsibilities:
- Read and normalize `server.properties`
- Initialize Windows/network/runtime systems
- Load or create the target world
- Run the dedicated main loop (network tick, XUI actions, autosave, CLI command processing)
- Perform safe save and shutdown

## 2. Important Files

### Startup and Runtime
- `Windows64/ServerMain.cpp`
  - Process entry (`main`)
  - CLI argument parsing
  - Runtime setup and shutdown flow
  - Main loop + autosave scheduler

### World Selection and Save Load
- `WorldManager.h`
- `WorldManager.cpp`
  - Finds matching save by `level-id` first, then world-name fallback
  - Applies storage title + save ID consistently
  - Wait helpers for async storage/server action completion

### Server Properties
- `ServerProperties.h`
- `ServerProperties.cpp`
  - Default values
  - Parse/normalize/write `server.properties`
  - Exposes `ServerPropertiesConfig`

### Logging
- `ServerLogger.h`
- `ServerLogger.cpp`
  - Log level parsing
  - Colored/timestamped console logs
  - Standard categories (`startup`, `world-io`, `console`, etc.)

### Console Command System
- `Console/ServerCli.cpp` (facade)
- `Console/ServerCliInput.cpp` (linenoise input thread + completion bridge)
- `Console/ServerCliParser.cpp` (tokenization/quoted args/completion context)
- `Console/ServerCliEngine.cpp` (dispatch, completion, helpers)
- `Console/ServerCliRegistry.cpp` (command registration + lookup)
- `Console/commands/*` (individual commands)

## 3. End-to-End Startup Flow

Main flow in `Windows64/ServerMain.cpp`:
1. Load `server.properties` via `LoadServerPropertiesConfig()`
2. Apply CLI argument overrides (`-port`, `-bind`, `-name`, `-seed`, `-loglevel`)
3. Initialize runtime systems (window/device/profile/network/thread storage)
4. Set host/game options from `ServerPropertiesConfig`
5. Bootstrap world with `BootstrapWorldForServer(...)`
6. Start hosted game thread (`RunNetworkGameThreadProc`)
7. Enter main loop:
   - `TickCoreSystems()`
   - `HandleXuiActions()`
   - `serverCli.Poll()`
   - autosave scheduling
8. On shutdown:
   - wait for action idle
   - request final save
   - halt server and terminate network/device subsystems

## 4. Common Development Tasks

### 4.1 Add a New CLI Command

Use this pattern when adding commands like `/kick`, `/time`, etc.

1. Add files under `Console/commands/`
   - `CliCommandYourCommand.h`
   - `CliCommandYourCommand.cpp`
2. Implement `IServerCliCommand`
   - `Name()`, `Usage()`, `Description()`, `Execute(...)`
   - Optional: `Aliases()` and `Complete(...)`
3. Register command in `ServerCliEngine::RegisterDefaultCommands()`
4. Add source/header to build definitions:
   - `CMakeLists.txt` (`MINECRAFT_SERVER_SOURCES`)
   - `Minecraft.Server/Minecraft.Server.vcxproj` (`<ClCompile>` / `<ClInclude>`)
5. Manual verify:
   - command appears in `help`
   - command executes correctly
   - completion behavior is correct for both `cmd` and `/cmd`

Implementation references:
- `CliCommandHelp.cpp` for simple no-arg command
- `CliCommandTp.cpp` for multi-arg + completion + runtime checks
- `CliCommandGamemode.cpp` for argument parsing and mode validation

### 4.2 Add or Change a `server.properties` Key

1. Add/update field in `ServerPropertiesConfig` (`ServerProperties.h`)
2. Add default value to `kServerPropertyDefaults` (`ServerProperties.cpp`)
3. Load and normalize value in `LoadServerPropertiesConfig()`
   - Use existing read helpers for bool/int/string/int64/log level
4. If this value should be persisted on save, update `SaveServerPropertiesConfig()`
5. Apply to runtime where needed:
   - `ApplyServerPropertiesToDedicatedConfig(...)`
   - host options in `ServerMain.cpp` (`app.SetGameHostOption(...)`)
6. Manual verify:
   - file regeneration when key is missing
   - invalid values are normalized
   - runtime behavior reflects the new value

### 4.3 Change World Load/Create Behavior

Primary code is in `WorldManager.cpp`.

Current matching policy:
1. Match by `level-id` (`UTF8SaveFilename`) first
2. Fallback to world-name match on title/file name

When changing this logic:
- Keep `ApplyWorldStorageTarget(...)` usage consistent (title + save ID together)
- Preserve periodic ticking in wait loops (`tickProc`) to avoid async deadlocks
- Keep timeout/error logs specific enough for diagnosis
- Verify:
  - existing world is reused correctly
  - no accidental new save directory creation
  - shutdown save still succeeds

### 4.4 Add Logging for New Feature Work

Use `ServerLogger` helpers:
- `LogDebug`, `LogInfo`, `LogWarn`, `LogError`
- or formatted variants `LogInfof`, etc.

Recommended categories:
- `startup` for init/shutdown lifecycle
- `world-io` for save/world operations
- `console` for CLI command handling

## 5. Build and Run

From repository root:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --target MinecraftServer
cd .\build\Debug
.\Minecraft.Server.exe -port 25565 -bind 0.0.0.0 -name DedicatedServer
```

Notes:
- Launch from output directory so relative assets/files resolve correctly
- `server.properties` is loaded from current working directory
- For Visual Studio workflow, see root `COMPILE.md`

## 6. Safety Checklist Before Commit

- The server starts without crash on a clean `server.properties`
- Existing world loads by expected `level-id`
- New world creation path still performs initial save
- CLI still accepts input and completion is responsive
- No busy wait path removed from async wait loops
- Both CMake and `.vcxproj` include newly added source files

## 7. Quick Troubleshooting

- Unknown command not found:
  - check `RegisterDefaultCommands()` and build-file entries
- Autosave or shutdown save timing out:
  - confirm wait loops still call `TickCoreSystems()` and `HandleXuiActions()` where required
- World not reused on restart:
  - inspect `level-id` normalization and matching logic in `WorldManager.cpp`
- Settings not applied:
  - confirm value is loaded into `ServerPropertiesConfig` and then applied in `ServerMain.cpp`
