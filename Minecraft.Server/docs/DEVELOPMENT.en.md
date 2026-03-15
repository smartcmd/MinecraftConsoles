# Minecraft.Server Developer Guide (English)

This document is for contributors who are new to `Minecraft.Server` and need a practical map for adding or modifying features safely.

## 1. What This Server Does

`Minecraft.Server` is the dedicated-server executable entry for this codebase.

Core responsibilities:
- Switch the process working directory to the executable folder before relative file I/O
- Load, normalize, and repair `server.properties`
- Initialize dedicated runtime systems, connection logging, and access control
- Load or create the target world and keep `level-id` aligned with the actual save destination
- Run the dedicated main loop (network tick, XUI actions, autosave, CLI input)
- Maintain operator-facing access files such as `banned-players.json` and `banned-ips.json`
- Perform an initial save for newly created worlds and then shut down safely

## 2. Important Files

### Startup and Runtime
- `Windows64/ServerMain.cpp`
  - `PrintUsage()` and `ParseCommandLine()`
  - `SetExeWorkingDirectory()`
  - Runtime setup and shutdown flow
  - Initial save path for newly created worlds
  - Main loop, autosave scheduler, and CLI polling

### World Selection and Save Load
- `WorldManager.h`
- `WorldManager.cpp`
  - Finds matching save by `level-id` first, then world-name fallback
  - Applies storage title + save ID consistently
  - Wait helpers for async storage/server action completion

### Server Properties
- `ServerProperties.h`
- `ServerProperties.cpp`
  - Default values and normalization ranges
  - Parse/repair/write `server.properties`
  - Exposes `ServerPropertiesConfig`
  - `SaveServerPropertiesConfig()` rewrites `level-name`, `level-id`, and `white-list`

### Access Control, Ban, and Whitelist Storage
- `Access/Access.h`
- `Access/Access.cpp`
  - Process-wide access-control facade
  - Published snapshot model used by console commands and login checks
- `Access/BanManager.h`
- `Access/BanManager.cpp`
  - Reads/writes `banned-players.json` and `banned-ips.json`
  - Normalizes identifiers and filters expired entries from snapshots
- `Access/WhitelistManager.h`
- `Access/WhitelistManager.cpp`
  - Reads/writes `whitelist.json`
  - Normalizes XUID-based whitelist entries used by login validation and CLI commands

### Logging and Connection Audit
- `ServerLogger.h`
- `ServerLogger.cpp`
  - Log level parsing
  - Colored/timestamped console logs
  - General categories such as `startup`, `world-io`, `console`, `access`, `network`, and `shutdown`
- `ServerLogManager.h`
- `ServerLogManager.cpp`
  - Accepted/rejected TCP connection logs
  - Login/disconnect audit logs
  - Remote-IP cache used by `ban-ip <player>`

### Console Command System
- `Console/ServerCli.cpp` (facade)
- `Console/ServerCliInput.cpp` (linenoise input thread + completion bridge)
- `Console/ServerCliParser.cpp` (tokenization, quoted args, completion context)
- `Console/ServerCliEngine.cpp` (dispatch, completion, helpers)
- `Console/ServerCliRegistry.cpp` (command registration + lookup)
- `Console/commands/*` (individual commands)

## 3. End-to-End Startup Flow

Main flow in `Windows64/ServerMain.cpp`:
1. `SetExeWorkingDirectory()` switches the current directory to the executable folder.
2. Load and normalize `server.properties` via `LoadServerPropertiesConfig()`.
3. Copy config into `DedicatedServerConfig`, then apply CLI overrides (`-port`, `-ip`/`-bind`, `-name`, `-maxplayers`, `-seed`, `-loglevel`, `-help`/`--help`/`-h`).
4. Initialize process state, `ServerLogManager`, and `Access::Initialize(".")`.
5. Initialize window/device/profile/network/thread-local systems.
6. Set host/game options from `ServerPropertiesConfig`.
7. Bootstrap world with `BootstrapWorldForServer(...)`.
8. If world bootstrap resolves a different normalized save ID, persist it with `SaveServerPropertiesConfig()`.
9. Start hosted game thread (`RunNetworkGameThreadProc`).
10. If a brand-new world was created, explicitly request one initial save.
11. Enter the main loop:
   - `TickCoreSystems()`
   - `HandleXuiActions()`
   - `serverCli.Poll()`
   - autosave scheduling
12. On shutdown:
   - stop CLI input
   - request save-on-exit / halt server
   - wait for network shutdown completion
   - terminate log, access, network, and device systems

## 4. Current Operator Surface

### 4.1 Launch Arguments
- `-port <1-65535>`
- `-ip <addr>` or `-bind <addr>`
- `-name <name>` (runtime max 16 chars)
- `-maxplayers <1-8>`
- `-seed <int64>`
- `-loglevel <debug|info|warn|error>`
- `-help`, `--help`, `-h`

Notes:
- CLI overrides affect only the current process.
- The only values currently written back by the server are `level-name` and `level-id`, and that happens when world bootstrap resolves identity changes.

### 4.2 Built-in Console Commands
- `help` / `?`
- `stop`
- `list`
- `ban <player> [reason ...]`
  - currently requires the target player to be online
- `ban-ip <address|player> [reason ...]`
  - accepts a literal IPv4/IPv6 address or an online player's current remote IP
- `pardon <player>`
- `pardon-ip <address>`
  - only accepts a literal address
- `banlist`
- `tp <player> <target>` / `teleport`
- `gamemode <survival|creative|0|1> [player]` / `gm`

CLI behavior notes:
- Command parsing accepts both `cmd` and `/cmd`.
- Quoted arguments are supported by `ServerCliParser`.
- Completion is implemented per command via `Complete(...)`.

### 4.3 Files Written Next to the Executable
- `server.properties`
- `banned-players.json`
- `banned-ips.json`

This follows from `SetExeWorkingDirectory()`, so these files are resolved relative to `Minecraft.Server.exe`, not the shell directory you launched from.

## 5. Common Development Tasks

### 5.1 Add a New CLI Command

Use this pattern when adding commands like `/kick`, `/time`, etc.

1. Add files under `Console/commands/`
   - `CliCommandYourCommand.h`
   - `CliCommandYourCommand.cpp`
2. Implement `IServerCliCommand`
   - `Name()`, `Usage()`, `Description()`, `Execute(...)`
   - optional: `Aliases()` and `Complete(...)`
3. Register the command in `ServerCliEngine::RegisterDefaultCommands()`.
4. Add source/header to build definitions:
   - `CMakeLists.txt` (`MINECRAFT_SERVER_SOURCES`)
   - `Minecraft.Server/Minecraft.Server.vcxproj` (`<ClCompile>` / `<ClInclude>`)
5. Manual verify:
   - command appears in `help`
   - command executes correctly
   - completion works for both `cmd` and `/cmd`
   - quoted arguments behave as expected

Implementation references:
- `CliCommandHelp.cpp` for a simple no-arg command
- `CliCommandTp.cpp` for multi-arg + completion + runtime checks
- `CliCommandGamemode.cpp` for argument parsing and aliases
- `CliCommandBanIp.cpp` for access-backed behavior with connection metadata

### 5.2 Add or Change a `server.properties` Key

1. Add/update the field in `ServerPropertiesConfig` (`ServerProperties.h`).
2. Add a default entry to `kServerPropertyDefaults` (`ServerProperties.cpp`).
3. Load and normalize the value in `LoadServerPropertiesConfig()`.
   - Use existing helpers for bool/int/string/int64/log level/level type.
4. If this value should be written back, update `SaveServerPropertiesConfig()`.
   - Note: today that function intentionally only persists world identity.
5. Apply it to runtime where needed:
   - `ApplyServerPropertiesToDedicatedConfig(...)`
   - host options in `ServerMain.cpp` (`app.SetGameHostOption(...)`)
   - `PrintUsage()` / `ParseCommandLine()` if the key also gets a CLI override
6. Manual verify:
   - missing key regeneration
   - invalid value normalization
   - clamped ranges still make sense
   - runtime behavior reflects the new value

Normalization details worth remembering:
- `level-id` is normalized to a safe save ID and length-limited.
- `server-name` is capped to 16 runtime chars.
- `max-players` is clamped to `1..8`.
- `autosave-interval` is clamped to `5..3600`.
- `level-type` normalizes to `default` or `flat`.

### 5.3 Change Ban / Access Behavior

Primary code lives in `Access/Access.cpp`, `Access/BanManager.cpp`, and `ServerLogManager.cpp`.

When changing this area:
- Keep `BanManager` responsible for storage/caching, not live-network policy.
- Keep the clone-and-publish snapshot pattern in `Access.cpp` so readers never block on disk I/O.
- Remember that `ban-ip <player>` depends on `ServerLogManager::TryGetConnectionRemoteIp(...)`.
- Keep expired entries out of `SnapshotBannedPlayers()` / `SnapshotBannedIps()` output.
- Verify:
  - clean boot creates empty ban files when missing
  - `ban`, `ban-ip`, `pardon`, `pardon-ip`, and `banlist` still work
  - online bans disconnect live targets immediately
  - manual edits still reload safely if you later add or extend reload paths

### 5.4 Change World Load/Create Behavior

Primary code is in `WorldManager.cpp`.

Current matching policy:
1. Match by `level-id` (`UTF8SaveFilename`) first.
2. Fall back to world-name match on title/file name.

When changing this logic:
- Keep `ApplyWorldStorageTarget(...)` usage consistent (title + save ID together).
- Preserve periodic ticking in wait loops (`tickProc`) to avoid async deadlocks.
- Keep timeout/error logs specific enough for diagnosis.
- Verify:
  - existing world is reused correctly
  - no accidental new save directory creation
  - shutdown save still succeeds
  - newly created worlds still get the explicit initial save from `ServerMain.cpp`

### 5.5 Add Logging for New Feature Work

Use `ServerLogger` helpers:
- `LogDebug`, `LogInfo`, `LogWarn`, `LogError`
- formatted variants `LogDebugf`, `LogInfof`, etc.

Use `ServerLogManager` when the event is specifically part of the transport/login/disconnect lifecycle.

Recommended categories:
- `startup` for init/shutdown lifecycle
- `world-io` for save/world operations
- `console` for CLI command handling
- `access` for ban/access control state
- `network` for connection/login audit

## 6. Build and Run

From repository root:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --target MinecraftServer
cd .\build\Debug
.\Minecraft.Server.exe -port 25565 -bind 0.0.0.0 -maxplayers 8 -name DedicatedServer
```

Notes:
- The process switches its working directory to the executable directory at startup.
- `server.properties`, `banned-players.json`, and `banned-ips.json` are therefore read/written next to the executable.
- For Visual Studio workflow, see root `COMPILE.md`.

## 7. Safety Checklist Before Commit

- the server starts without crash when `server.properties` is missing or sparse
- missing access files are recreated on a clean boot
- existing world loads by expected `level-id`
- new world creation still performs the explicit initial save
- CLI input and completion remain responsive
- `banlist` output stays sane after adding/removing bans
- no busy-wait path removed from async wait loops
- both CMake and `.vcxproj` include newly added source files

## 8. Quick Troubleshooting

- Unknown command:
  - check `RegisterDefaultCommands()` and build-file entries
- `server.properties` or ban files seem to load from the wrong folder:
  - remember `SetExeWorkingDirectory()` moves the working directory to the executable folder
- Autosave or shutdown save timing out:
  - confirm wait loops still call `TickCoreSystems()` and `HandleXuiActions()` where required
- World not reused on restart:
  - inspect `level-id` normalization and matching logic in `WorldManager.cpp`
- `ban-ip <player>` cannot resolve an address:
  - confirm the player is currently online and `ServerLogManager` has a cached remote IP for that connection
- Settings not applied:
  - confirm the value is loaded into `ServerPropertiesConfig`, optionally copied into `DedicatedServerConfig`, and then applied in `ServerMain.cpp`
