# Repository Map — MinecraftConsoles

## What This Project Is

Source code for **Minecraft Legacy Console Edition v1.3.0494.0** (originally by 4J Studios), a C++ Minecraft implementation targeting multiple console platforms. The community fork adds Windows 64-bit support with keyboard/mouse input, fullscreen toggle (F11), DPI-aware rendering, and V-Sync control.

**Language:** C++ | **Build:** Visual Studio 2022 (.sln) + CMake (Windows x64 only) | **Graphics:** Direct3D 11

---

## Top-Level Structure

```
MinecraftConsoles/
├── Minecraft.Client/         # Rendering, UI, models, particles, input, platform layers (~515 source files)
├── Minecraft.World/          # Game logic: entities, tiles, items, world gen, networking (~1560 source files)
├── cmake/                    # CMake build helpers (3 files)
├── x64/                      # Pre-built runtime DLLs (Debug + Release)
├── .github/                  # CI workflows + PR template
├── CMakeLists.txt            # Root CMake config — builds MinecraftWorld (static lib) + MinecraftClient (exe)
├── MinecraftConsoles.sln     # VS solution with Minecraft.World + Minecraft.Client projects
├── Compile.md                # Build instructions
├── README.md                 # Project overview and controls
└── .gitignore                # VS-focused ignores
```

---

## Build System

### Two build targets
1. **MinecraftWorld** — Static library. All game logic.
2. **MinecraftClient** — Executable. Links MinecraftWorld + d3d11 + XInput + Iggy + Miles + 4JLibs.

### Key defines
`_LARGE_WORLDS`, `_DEBUG_MENUS_ENABLED`, `_WINDOWS64`, `_CRT_SECURE_NO_WARNINGS`

### CMake files (`cmake/`)
| File | Purpose |
|------|---------|
| `WorldSources.cmake` | Lists ~700 source files for Minecraft.World |
| `ClientSources.cmake` | Lists ~450 source files for Minecraft.Client |
| `CopyAssets.cmake` | Post-build: copies media, DLLs (iggy_w64.dll, mss64.dll), redistributables |

### Third-party libraries (linked as .lib/.dll)
| Library | Path | Purpose |
|---------|------|---------|
| **4JLibs** | `Minecraft.Client/Windows64/4JLibs/` | Input, Storage, Profile, Render (4J Studios proprietary) |
| **Iggy** | `Minecraft.Client/Windows64/Iggy/` | Scaleform-like UI middleware (iggy_w64.lib/dll) |
| **Miles** | `Minecraft.Client/Windows64/Miles/` | RAD Game Tools audio (mss64.lib/dll) |
| **zlib** | `Minecraft.Client/Common/zlib/` | Compression (compiled from source) |

### Runtime redistributables (`Minecraft.Client/redist64/`)
binkawin64.asi, mss64dolby.flt, mss64ds3d.flt, mss64dsp.flt, mss64eax.flt, mss64srs.flt

---

## Minecraft.Client — Rendering & UI Layer

### Top-level source files (~515 files)
All files sit directly in `Minecraft.Client/` (flat structure, no subdirectories for game code).

#### Core Engine
| File(s) | Purpose |
|---------|---------|
| `Minecraft.cpp/.h` | **Main application class** — orchestrates world, rendering, input, UI, game loop |
| `GameRenderer.cpp/.h` | Main rendering pipeline, camera, FOV, item-in-hand rendering |
| `LevelRenderer.cpp/.h` | World/chunk rendering, frustum culling, block destruction animation |
| `TileRenderer.cpp/.h` | Individual block/tile rendering |
| `Tesselator.cpp/.h` | Low-level vertex buffer / geometry submission |
| `Camera.cpp/.h` | Camera positioning and view matrix |
| `Frustum.cpp/.h`, `FrustumCuller.cpp/.h`, `FrustumData.cpp/.h` | View frustum culling |
| `Lighting.cpp/.h` | Lighting calculations |
| `Timer.cpp/.h` | Frame timing |
| `Chunk.cpp/.h` | Client-side chunk render data |

#### Textures & Resources
| File(s) | Purpose |
|---------|---------|
| `Textures.cpp/.h` | Master texture registry (80+ texture IDs for terrain, mobs, GUI, items) |
| `TextureManager.cpp/.h` | Texture loading and lifecycle |
| `TextureMap.cpp/.h`, `PreStitchedTextureMap.cpp/.h` | Atlas stitching |
| `Stitcher.cpp/.h`, `StitchSlot.cpp/.h`, `StitchedTexture.cpp/.h` | Texture atlas packing |
| `BufferedImage.cpp/.h` | Image pixel manipulation |
| `TexturePack.cpp/.h`, `TexturePackRepository.cpp/.h` | Texture pack system |
| `DefaultTexturePack`, `FileTexturePack`, `FolderTexturePack`, `DLCTexturePack` | Pack implementations |
| `ClockTexture`, `CompassTexture` | Animated special textures |
| `Font.cpp/.h` | Bitmap font rendering |

#### Entity Renderers (50+ renderer/model pairs)
Pattern: `<Entity>Renderer.cpp/.h` + `<Entity>Model.cpp/.h`

| Renderer + Model | Entity |
|------------------|--------|
| `PlayerRenderer`, `HumanoidModel` | Players |
| `CreeperRenderer`, `CreeperModel` | Creeper |
| `SkeletonRenderer`, `SkeletonModel` | Skeleton |
| `ZombieRenderer`, `ZombieModel` | Zombie |
| `SpiderRenderer`, `SpiderModel` | Spider |
| `CowRenderer`, `CowModel` | Cow |
| `PigRenderer`, `PigModel` | Pig |
| `ChickenRenderer`, `ChickenModel` | Chicken |
| `SheepRenderer`, `SheepModel`, `SheepFurModel` | Sheep |
| `WolfRenderer`, `WolfModel` | Wolf |
| `EndermanRenderer`, `EndermanModel` | Enderman |
| `BlazeRenderer`, `BlazeModel` | Blaze |
| `GhastRenderer`, `GhastModel` | Ghast |
| `SlimeRenderer`, `SlimeModel` | Slime |
| `LavaSlimeRenderer`, `LavaSlimeModel` | Magma Cube |
| `SilverfishRenderer`, `SilverfishModel` | Silverfish |
| `SquidRenderer`, `SquidModel` | Squid |
| `VillagerRenderer`, `VillagerModel` | Villager |
| `VillagerGolemRenderer`, `VillagerGolemModel` | Iron Golem |
| `SnowManRenderer`, `SnowManModel` | Snow Golem |
| `OzelotRenderer`, `OzelotModel` | Ocelot |
| `EnderDragonRenderer`, `DragonModel` | Ender Dragon |
| `EnderCrystalRenderer`, `EnderCrystalModel` | End Crystal |
| `BoatRenderer`, `BoatModel` | Boat |
| `MinecartRenderer`, `MinecartModel` | Minecart |
| `ChestRenderer`, `ChestModel`, `LargeChestModel` | Chest |
| `SignRenderer`, `SignModel` | Sign |
| `BookModel` | Enchanting table book |
| `ArrowRenderer` | Arrow |
| `FireballRenderer` | Fireball |
| `FishingHookRenderer` | Fishing hook |
| `ExperienceOrbRenderer` | XP orbs |
| `FallingTileRenderer` | Falling blocks |
| `TntRenderer` | TNT |
| `PaintingRenderer` | Paintings |
| `ItemFrameRenderer` | Item frames |
| `LightningBoltRenderer` | Lightning |
| `PistonPieceRenderer` | Moving piston |

Base classes: `EntityRenderer`, `MobRenderer`, `HumanoidMobRenderer`, `DefaultRenderer`, `GiantMobRenderer`

Tile entity rendering: `TileEntityRenderDispatcher`, `TileEntityRenderer`, `EnchantTableRenderer`, `EnderChestRenderer`, `MobSpawnerRenderer`, `SkullTileRenderer`, `TheEndPortalRenderer`

Items: `ItemRenderer`, `ItemInHandRenderer`, `ItemSpriteRenderer`

Model primitives: `Model`, `ModelPart`, `Cube`, `Polygon`, `Vertex`, `TexOffs`

#### Particle System (20+ types)
All in `Minecraft.Client/`:
`Particle` (base), `ParticleEngine` (manager),
`BubbleParticle`, `BreakingItemParticle`, `CritParticle`, `CritParticle2`,
`DragonBreathParticle`, `DripParticle`, `EchantmentTableParticle`,
`EnderParticle`, `ExplodeParticle`, `FlameParticle`, `FootstepParticle`,
`GuiParticle`, `HeartParticle`, `HugeExplosionParticle`, `HugeExplosionSeedParticle`,
`LavaParticle`, `NetherPortalParticle`, `NoteParticle`, `PlayerCloudParticle`,
`RedDustParticle`, `SmokeParticle`, `SnowShovelParticle`, `SpellParticle`,
`SplashParticle`, `SuspendedParticle`, `SuspendedTownParticle`,
`TakeAnimationParticle`, `TerrainParticle`, `WaterDropParticle`

#### UI Screens (40+)
| Screen | Purpose |
|--------|---------|
| `TitleScreen` | Main menu |
| `PauseScreen` | In-game pause |
| `OptionsScreen`, `VideoSettingsScreen`, `ControlsScreen` | Settings |
| `SelectWorldScreen`, `CreateWorldScreen`, `RenameWorldScreen` | World management |
| `InventoryScreen` | Player inventory |
| `CraftingScreen` | Crafting table |
| `ContainerScreen`, `AbstractContainerScreen` | Generic container UI |
| `FurnaceScreen` | Furnace |
| `ChatScreen`, `InBedChatScreen` | Chat |
| `DeathScreen` | Death/respawn |
| `ConnectScreen`, `JoinMultiplayerScreen` | Multiplayer |
| `DisconnectedScreen`, `ErrorScreen` | Error handling |
| `ReceivingLevelScreen` | Loading screen |
| `StatsScreen` | Statistics |
| `AchievementScreen` | Achievements |
| `ConfirmScreen` | Confirmation dialogs |
| `TextEditScreen`, `NameEntryScreen` | Text input |
| `TrapScreen` | (Unknown/legacy) |

Base: `Screen.cpp/.h`, `GuiComponent.cpp/.h`
HUD: `Gui.cpp/.h` (health, hotbar, messages, overlay)
Widgets: `Button`, `SmallButton`, `SlideButton`, `EditBox`, `ScrolledSelectionList`

#### Game Modes (client-side)
| File | Purpose |
|------|---------|
| `GameMode.cpp/.h` | Base game mode |
| `SurvivalMode.cpp/.h` | Survival mode client behavior |
| `CreativeMode.cpp/.h` | Creative mode client behavior |
| `MultiPlayerGameMode.cpp/.h` | Multiplayer game mode |
| `DemoMode.cpp/.h` | Demo restrictions |

#### Multiplayer (client-side)
| File | Purpose |
|------|---------|
| `ClientConnection.cpp/.h` | Socket connection to server |
| `PendingConnection.cpp/.h` | Connection handshake |
| `PlayerConnection.cpp/.h` | Protocol handling |
| `MultiPlayerLevel.cpp/.h` | Network-synced level |
| `MultiPlayerLocalPlayer.cpp/.h` | Net-aware local player |
| `MultiPlayerChunkCache.cpp/.h` | Client chunk storage |
| `RemotePlayer.cpp/.h` | Other players |
| `PlayerList.cpp/.h`, `PlayerInfo.h` | Player roster |
| `EntityTracker.cpp/.h`, `TrackedEntity.cpp/.h` | Entity sync tracking |

#### Server (integrated/listen server)
| File | Purpose |
|------|---------|
| `MinecraftServer.cpp/.h` | Integrated server |
| `ServerLevel.cpp/.h`, `DerivedServerLevel.cpp/.h` | Server world |
| `ServerPlayer.cpp/.h` | Server-side player |
| `ServerPlayerGameMode.cpp/.h` | Server game mode logic |
| `ServerConnection.cpp/.h` | Server networking |
| `ServerCommandDispatcher.cpp/.h` | Command handling |
| `ServerChunkCache.cpp/.h` | Server chunk management |
| `PlayerChunkMap.cpp/.h` | Chunk visibility per player |

#### Other notable files
| File | Purpose |
|------|---------|
| `LocalPlayer.cpp/.h` | Client-side player state + input |
| `Input.cpp/.h` | Input device abstraction |
| `ConsoleInput.cpp/.h`, `ConsoleInputSource.h` | Console controller input |
| `KeyMapping.cpp/.h` | Key binding definitions |
| `Options.cpp/.h` | Game options/settings |
| `Settings.cpp/.h` | Persistent settings |
| `StringTable.cpp/.h` | Localization |
| `Minimap.cpp/.h` | Minimap rendering |
| `MemoryTracker.cpp/.h` | Memory diagnostics |
| `ArchiveFile.cpp/.h` | Archive/asset loading |
| `AchievementPopup.cpp/.h` | Achievement toast UI |
| `ProgressRenderer.cpp/.h` | Loading progress bar |
| `StatsCounter.cpp/.h`, `StatsSyncher.cpp/.h` | Statistics tracking |
| `DemoLevel.cpp/.h`, `DemoUser.cpp/.h` | Demo mode |
| `TeleportCommand.cpp/.h` | Teleport command |
| `ScreenSizeCalculator.cpp/.h` | Resolution scaling |
| `Rect2i.cpp/.h` | 2D rectangle utility |
| `glWrapper.cpp` | OpenGL compatibility wrapper |
| `crt_compat.cpp` | CRT compatibility shims |
| `stubs.cpp/.h`, `Extrax64Stubs.cpp` | Platform stubs |
| `stdafx.cpp/.h` | Precompiled header |

---

### Platform Directories (under `Minecraft.Client/`)

#### Platform Abstraction Pattern
```
CMinecraftApp (Common/Consoles_App.h)    ← base class
  └─ CConsoleMinecraftApp (*_App.h)       ← per-platform override
```
Each platform provides: `*_App.cpp/.h`, `*_Minecraft.cpp` (main loop, ~45-70KB), `*_UIController.cpp/.h`

| Directory | Platform | Key entry point | Notes |
|-----------|----------|-----------------|-------|
| `Windows64/` | **Windows PC** | `Windows64_Minecraft.cpp` (52KB) | D3D11, keyboard/mouse input, primary dev target |
| `Xbox/` | Xbox 360 | `Xbox_Minecraft.cpp` (42KB) | Xbox Live, Kinect, largest app file (94KB) |
| `Durango/` | Xbox One | `Durango_Minecraft.cpp` (46KB) | UWP app, Xbox Live, content updates |
| `PS3/` | PlayStation 3 | `PS3_Minecraft.cpp` (61KB) | Cell/SPU tasks, PSN |
| `PSVita/` | PS Vita | `PSVita_Minecraft.cpp` (46KB) | Touch input, portable optimizations |
| `Orbis/` | PlayStation 4 | `Orbis_Minecraft.cpp` (69KB) | PSN, Sony Commerce |

#### Windows64-specific files
| File | Purpose |
|------|---------|
| `KeyboardMouseInput.cpp/.h` | **Keyboard + mouse input** — raw input, accumulator pattern, mouse capture |
| `Windows64_App.cpp/.h` | Windows app lifecycle |
| `Windows64_Minecraft.cpp` | Main game loop, window creation, D3D11 init |
| `Windows64_UIController.cpp/.h` | UI input routing |

Each platform dir also contains: `4JLibs/`, `Iggy/`, `Miles/`, `Leaderboards/`, `Network/`, `Sentient/` (crash telemetry), `Social/`, `XML/`, `GameConfig/`

#### Common/ — Shared console code (`Minecraft.Client/Common/`)

**Root files:**
| File | Purpose |
|------|---------|
| `Consoles_App.cpp/.h` | **Base application class** (CMinecraftApp) |
| `ConsoleGameMode.cpp/.h` | Console-specific game mode logic |
| `Console_Utils.cpp` | Shared utility functions |
| `App_Defines.h`, `App_enums.h`, `App_structs.h` | Shared type definitions |
| `C4JMemoryPool.h`, `C4JMemoryPoolAllocator.h` | Custom memory allocator |
| `Minecraft_Macros.h`, `Potion_Macros.h` | Preprocessor macros |
| `Console_Awards_enum.h`, `Console_Debug_enum.h` | Award/debug enumerations |

**Subdirectories:**

| Directory | Files | Purpose |
|-----------|-------|---------|
| `Audio/` | SoundEngine, SoundNames | Audio engine integration |
| `Colours/` | ColourTable | Color management |
| `DLC/` | DLCManager, DLCFile, DLCSkinFile, DLCTextureFile, DLCGameRules, etc. (13 classes) | DLC pack management |
| `GameRules/` | GameRuleManager, GameRule, LevelRuleset, BiomeOverride, ConsoleSchematicFile, etc. (28 classes) | Game rule + level generation customization |
| `Leaderboards/` | LeaderboardManager | Cross-platform leaderboard interface |
| `Network/` | GameNetworkManager, PlatformNetworkManagerStub | Network abstraction |
| `Telemetry/` | TelemetryManager | Analytics |
| `Trial/` | TrialMode | Trial/demo mode |
| `Tutorial/` | FullTutorial, TutorialMode, TutorialTask, TutorialHint, etc. (36 classes) | In-game tutorial system |
| `UI/` | **~110 classes** — UIScene_*, UIControl_*, UIComponent_* | Full menu/UI framework (see below) |
| `XUI/` | Xbox UI helpers | XUI framework integration |
| `Media/` | Graphics/, Sound/, fonts, localization | Asset directories |
| `zlib/` | adler32, compress, deflate, inflate, etc. | Compression library |

**Common/UI/ — Console UI Framework (~110 classes):**
- **Scenes** (full screens): UIScene_MainMenu, UIScene_PauseMenu, UIScene_HUD, UIScene_InventoryMenu, UIScene_CraftingMenu, UIScene_CreativeMenu, UIScene_FurnaceMenu, UIScene_AnvilMenu, UIScene_BrewingStandMenu, UIScene_EnchantingMenu, UIScene_TradingMenu, UIScene_ContainerMenu, UIScene_DispenserMenu, UIScene_DeathMenu, UIScene_ControlsMenu, UIScene_CreateWorldMenu, UIScene_LoadMenu, UIScene_JoinMenu, UIScene_Credits, UIScene_EndPoem, UIScene_HowToPlay, UIScene_DLCMainMenu, UIScene_DLCOffersMenu, UIScene_LeaderboardsMenu, UIScene_SkinSelectMenu, UIScene_SettingsMenu (+Audio/Control/Graphics/Options/UI sub-menus), UIScene_DebugOptions, UIScene_DebugOverlay, UIScene_Keyboard, UIScene_MessageBox, UIScene_TeleportMenu
- **Controls** (widgets): UIControl_Button, UIControl_ButtonList, UIControl_Slider, UIControl_CheckBox, UIControl_Label, UIControl_DynamicLabel, UIControl_HTMLLabel, UIControl_TextInput, UIControl_SlotList, UIControl_SaveList, UIControl_DLCList, UIControl_PlayerList, UIControl_LeaderboardList, UIControl_TexturePackList, UIControl_Progress, UIControl_Cursor, UIControl_BitmapIcon, UIControl_MinecraftPlayer, UIControl_PlayerSkinPreview, UIControl_EnchantmentBook, UIControl_EnchantmentButton, UIControl_SpaceIndicatorBar, UIControl_Touch
- **Components** (overlays): UIComponent_Chat, UIComponent_Logo, UIComponent_MenuBackground, UIComponent_Panorama, UIComponent_PressStartToPlay, UIComponent_Tooltips, UIComponent_TutorialPopup, UIComponent_DebugUIConsole
- **Base/infra**: UI, UIScene, UILayer, UIGroup, UIControl, UIControl_Base, UIController, IUIController, UIEnums, UIStructs, UIFontData, UIBitmapFont, UITTFFont

---

## Minecraft.World — Game Logic Layer

All ~1560 files sit in `Minecraft.World/` (flat structure). One subdirectory: `x64headers/` (platform stubs: extraX64.h, qnet.h, xmcore.h, xrnm.h, xsocialpost.h, xuiapp.h, xuiresource.h).

### Entity Hierarchy
```
Entity                          ← base (position, velocity, collision, damage)
├── Mob                         ← living entity (health, AI, effects, equipment)
│   ├── PathfinderMob           ← pathfinding-capable mob
│   │   ├── Creature            ← passive/neutral creatures
│   │   │   ├── Animal          ← breedable animals
│   │   │   │   ├── AgableMob   ← aging (baby → adult)
│   │   │   │   ├── Cow, Pig, Chicken, Sheep, MushroomCow, Squid
│   │   │   │   ├── WaterAnimal
│   │   │   │   ├── TamableAnimal ← tamable
│   │   │   │   │   ├── Wolf, Ozelot
│   │   │   ├── Golem           ← utility mobs
│   │   │   │   ├── VillagerGolem (Iron Golem), SnowMan
│   │   │   ├── Npc             ← NPCs
│   │   │       └── Villager
│   │   ├── Monster             ← hostile mobs
│   │   │   ├── Zombie, Skeleton, Spider, CaveSpider, Creeper
│   │   │   ├── EnderMan, Silverfish, PigZombie
│   │   │   ├── Blaze, Ghast, LavaSlime (Magma Cube), Slime
│   │   ├── FlyingMob           ← flying hostile
│   ├── BossMob                 ← boss entities
│   │   └── EnderDragon (+ BossMobPart)
│   ├── Player                  ← players
├── Projectile / Throwable
│   ├── Arrow, Fireball, SmallFireball, DragonFireball
│   ├── ThrownEgg, ThrownEnderpearl, ThrownExpBottle, ThrownPotion
│   ├── Snowball, FishingHook, EyeOfEnderSignal
├── HangingEntity               ← wall-mounted
│   └── Painting
├── ExperienceOrb, Boat, PrimedTnt, FallingTile
├── LightningBolt (GlobalEntity)
├── EnderCrystal
└── PistonPieceEntity (piston moving piece)
```

### Tiles (Blocks) — 80+ types
Base classes: `Tile` → `TransparentTile`, `LiquidTile`, `HalfTransparentTile`, `HeavyTile`, `DirectionalTile`, `EntityTile`, `Bush`

**Natural:** AirTile, DirtTile, GrassTile, GravelTile, ClayTile, SandTile, StoneTile, OreTile, IceTile, SnowTile, ObsidianTile, NetherrackTile, SoulSandTile, GlowstoneTile, EndStoneTile, MyceliumTile, CoralTile
**Wood/Plants:** LogTile, LeafTile, SaplingTile, WoodTile, Bush, TallGrass, DeadBushTile, CactusTile, CropTile, CarrotTile, PotatoTile, CocoaTile, StemTile, VineTile, WaterlilyTile, FlowerPotTile, MushroomTile, HugeMushroomTile
**Building:** StoneBrickTile, SandStoneTile, ClothTile (wool), GlassTile, StainedGlassTile, StainedGlassPaneTile, BookshelfTile, FenceTile, FenceGateTile, WallTile, IronBarsTile, HalfSlabTile, StairTile, DoorTile, TrapDoorTile, LadderTile
**Redstone:** RedstoneTorchTile, RedStoneWireTile, RepeaterTile (DiodeTile), ButtonTile, LeverTile, PressurePlateTile, DetectorRailTile, PoweredRailTile, RailTile, PistonBaseTile, PistonHeadTile, DispenserTile, DropperTile, TNTTile, RedstoneLampTile, DaylightDetectorTile, TripwireTile, TripwireHookTile, NoteBlockTile
**Interactive:** ChestTile, EnderChestTile, FurnaceTile, BrewingStandTile, EnchantmentTableTile, AnvilTile, CauldronTile, CakeTile, BedTile, SignTile, SpawnerTile, JukeboxTile, BeaconTile, CommandTile (command block)
**Liquids:** LiquidTile (water/lava), FallingLiquidTile
**Portal:** PortalTile, EndPortalTile, EndPortalFrameTile, DragonEggTile (EggTile)

### Tile Entities (block entities)
`TileEntity` (base) → `FurnaceTileEntity`, `ChestTileEntity`, `BrewingStandTileEntity`, `DispenserTileEntity`, `EnderChestTileEntity`, `MobSpawnerTileEntity`, `SignTileEntity`, `MusicTileEntity` (noteblock/jukebox), `SkullTileEntity`, `TheEndPortalTileEntity`

### Items — 60+ types
Base: `Item` → `TileItem` (block-as-item), `DiggerItem` (tools), `WeaponItem` (swords), `ArmorItem`, `FoodItem`

**Tools:** PickaxeItem, HatchetItem (axe), ShovelItem, HoeItem, ShearsItem, FlintAndSteelItem, FishingRodItem, CarrotOnAStickItem
**Weapons:** WeaponItem (swords), BowItem
**Food:** FoodItem, BowlFoodItem (stew), SeedFoodItem, GoldenAppleItem
**Functional:** BucketItem, MilkBucketItem, BedItem, BoatItem, MinecartItem, SaddleItem, DoorItem, SignItem, BookItem, EnchantedBookItem, MapItem, CompassItem, ClockItem, DyePowderItem, BottleItem, PotionItem
**Throwable:** EggItem, SnowballItem, EnderpearlItem, EnderEyeItem, ExperienceItem (bottle o' enchanting), FireChargeItem
**Placement:** MonsterPlacerItem (spawn eggs), SkullItem, HangingEntityItem (paintings/item frames), RecordingItem (music discs)
**Block items:** TileItem, AuxDataTileItem, MultiTextureTileItem, ColoredTileItem, various specialized TileItems

### Crafting & Recipes
| File | Purpose |
|------|---------|
| `Recipes.cpp/.h` | Recipe registry |
| `Recipy.h` | Recipe interface |
| `ShapedRecipy.cpp/.h` | Shaped crafting recipes |
| `ShapelessRecipy.cpp/.h` | Shapeless recipes |
| `FurnaceRecipes.cpp/.h` | Smelting recipes |
| `ArmorRecipes`, `WeaponRecipies`, `ToolRecipies`, `OreRecipies` | Category recipe registration |
| `FoodRecipies`, `ClothDyeRecipes`, `StructureRecipies` | More recipe categories |
| `ArmorDyeRecipe` | Special armor dyeing recipe |
| `MerchantRecipe`, `MerchantRecipeList` | Villager trading |
| `PotionBrewing` | Brewing recipes |

### Enchantments — 16 types
Base: `Enchantment`

`DamageEnchantment` (Sharpness/Smite/Bane), `KnockbackEnchantment`, `FireAspectEnchantment`,
`ProtectionEnchantment` (Protection/Fire Prot/Blast Prot/Projectile Prot/Feather Falling),
`DiggingEnchantment` (Efficiency), `DigDurabilityEnchantment` (Unbreaking), `UntouchingEnchantment` (Silk Touch),
`LootBonusEnchantment` (Looting/Fortune), `OxygenEnchantment` (Respiration), `WaterWorkerEnchantment` (Aqua Affinity),
`ThornsEnchantment`, `ArrowDamageEnchantment` (Power), `ArrowKnockbackEnchantment` (Punch),
`ArrowFireEnchantment` (Flame), `ArrowInfiniteEnchantment` (Infinity)

### AI Goals — 45 types
Base: `Goal`, `TargetGoal`

**Movement:** FloatGoal, RandomStrollGoal, PanicGoal, FleeSunGoal, MoveIndoorsGoal, MoveThroughVillageGoal, MoveTowardsRestrictionGoal, MoveTowardsTargetGoal, FollowOwnerGoal, FollowParentGoal, ControlledByPlayerGoal
**Combat:** MeleeAttackGoal, ArrowAttackGoal, LeapAtTargetGoal, SwellGoal (creeper), OzelotAttackGoal
**Targeting:** NearestAttackableTargetGoal, HurtByTargetGoal, OwnerHurtTargetGoal, OwnerHurtByTargetGoal, DefendVillageTargetGoal, NonTameRandomTargetGoal, AvoidPlayerGoal
**Social:** BreedGoal, MakeLoveGoal (villager), TemptGoal, BegGoal (wolf), PlayGoal, TradeWithPlayerGoal, LookAtPlayerGoal, LookAtTradingPlayerGoal, OfferFlowerGoal, TakeFlowerGoal (iron golem)
**Interaction:** BreakDoorGoal, OpenDoorGoal, DoorInteractGoal, EatTileGoal, OcelotSitOnTileGoal, SitGoal, RestrictSunGoal, RestrictOpenDoorGoal
**Look:** RandomLookAroundGoal, InteractGoal

Control classes: `BodyControl`, `JumpControl`, `LookControl`, `MoveControl`, `PathNavigation`, `Sensing`

### World Generation
**Level sources:** `RandomLevelSource` (overworld), `HellRandomLevelSource` (nether), `TheEndLevelRandomLevelSource` (end), `FlatLevelSource` (superflat), `CustomLevelSource`

**Biomes:** Biome (base), PlainsBiome, ForestBiome, DesertBiome, ExtremeHillsBiome, TaigaBiome, SwampBiome, JungleBiome, IceBiome, OceanBiome, RiverBiome, BeachBiome, MushroomIslandBiome, HellBiome, TheEndBiome, RainforestBiome
**Biome infrastructure:** BiomeSource, BiomeCache, BiomeDecorator, FoliageColor, GrassColor, WaterColor

**Layer system (biome generation):** Layer (base), BiomeInitLayer, BiomeOverrideLayer, IslandLayer, AddIslandLayer, AddMushroomIslandLayer, AddSnowLayer, DownfallLayer, DownfallMixerLayer, FuzzyZoomLayer, ZoomLayer, SmoothLayer, SmoothZoomLayer, RegionHillsLayer, RiverInitLayer, RiverLayer, RiverMixerLayer, ShoreLayer, SwampRiversLayer, TemperatureLayer, TemperatureMixerLayer, VoronoiZoom, FlatLayer, GrowMushroomIslandLayer, DataLayer, LightLayer

**Features (structures/decorations):** Feature (base), LargeFeature, StructureFeature
- Trees: TreeFeature, BirchFeature, PineFeature, SpruceFeature, MegaTreeFeature, SwampTreeFeature, GroundBushFeature, BasicTree, Sapling
- Structures: StrongholdFeature (+StrongholdPieces), VillageFeature (+VillagePieces), NetherBridgeFeature (+NetherBridgePieces), MineShaftFeature, RandomScatteredLargeFeature, TownFeature, HouseFeature, DesertWellFeature, EndPodiumFeature
- Terrain: CaveFeature, LargeCaveFeature, CanyonFeature, DungeonFeature (MonsterRoomFeature)
- Decoration: OreFeature, LakeFeature, SpringFeature, FlowerFeature, TallGrassFeature, DeadBushFeature, CactusFeature, ReedsFeature, PumpkinFeature, HugeMushroomFeature, VinesFeature, WaterlilyFeature, SandFeature, ClayFeature, SpikeFeature, BonusChestFeature
- Nether: HellFireFeature, HellPortalFeature, HellSpringFeature, LightGemFeature, NetherSphere

### Networking — 85 packet types
Base: `Packet`

**Entity packets:** AddPlayerPacket, AddMobPacket, AddEntityPacket, AddPaintingPacket, AddGlobalEntityPacket, AddExperienceOrbPacket, RemoveEntitiesPacket, MoveEntityPacket, MovePlayerPacket, RotateHeadPacket, TeleportEntityPacket, SetEntityDataPacket, SetEntityMotionPacket, SetRidingPacket, AnimatePacket, EntityEventPacket, EntityActionAtPositionPacket, InteractPacket
**Player packets:** PlayerActionPacket, PlayerCommandPacket, PlayerInputPacket, PlayerAbilitiesPacket, PlayerInfoPacket, SetCarriedItemPacket, SetEquippedItemPacket, SetHealthPacket, SetExperiencePacket, RespawnPacket, UseItemPacket
**Block packets:** TileUpdatePacket, TileDestructionPacket, TileEventPacket, TileEntityDataPacket, ChunkTilesUpdatePacket, BlockRegionUpdatePacket, SignUpdatePacket
**Chunk packets:** ChunkVisibilityPacket, ChunkVisibilityAreaPacket, XZPacket
**Container packets:** ContainerOpenPacket, ContainerClosePacket, ContainerClickPacket, ContainerAckPacket, ContainerSetSlotPacket, ContainerSetContentPacket, ContainerSetDataPacket, ContainerButtonClickPacket, SetCreativeModeSlotPacket, CraftItemPacket, TradeItemPacket
**World packets:** LevelEventPacket, LevelSoundPacket, GameEventPacket, SetTimePacket, SetSpawnPositionPacket, ExplodePacket
**Status packets:** UpdateMobEffectPacket, RemoveMobEffectPacket, UpdateProgressPacket, UpdateGameRuleProgressPacket, AwardStatPacket
**Connection packets:** LoginPacket, PreLoginPacket, DisconnectPacket, KickPlayerPacket, KeepAlivePacket, SharedKeyPacket, ServerAuthDataPacket, ClientProtocolPacket, ClientCommandPacket, ClientInformationPacket, GetInfoPacket
**Other:** ChatPacket, ChatAutoCompletePacket, CustomPayloadPacket, TexturePacket, TextureChangePacket, TextureAndGeometryPacket, TextureAndGeometryChangePacket, ComplexItemDataPacket, DebugOptionsPacket, GameCommandPacket, ServerSettingsChangedPacket, DefaultGameModeCommand, ExperienceCommand, GameModeCommand, KillCommand, TimeCommand, ToggleDownfallCommand

### Dimensions
`Dimension` (base) → `NormalDimension` (overworld), `HellDimension` (nether), `TheEndDimension`

### Level / Chunk Storage
**Level:** Level, LevelData, LevelSettings, LevelType, LevelEvent, LevelListener, DerivedLevelData, LevelSummary
**Chunks:** LevelChunk, EmptyLevelChunk, WaterLevelChunk, ChunkPos, ChunkSource, ChunkStorage
**Storage formats:** McRegionChunkStorage, McRegionLevelStorage, McRegionLevelStorageSource, ZonedChunkStorage, ZoneFile, ZoneIo, RegionFile, RegionFileCache, MemoryChunkStorage, MemoryLevelStorage, OldChunkStorage
**Console save:** ConsoleSaveFile, ConsoleSaveFileIO, ConsoleSaveFileConverter, ConsoleSaveFileOriginal, ConsoleSaveFileSplit, ConsoleSaveFileInputStream, ConsoleSaveFileOutputStream, ConsoleSavePath
**NBT:** NbtIo, CompoundTag, ListTag, ByteTag, ShortTag, IntTag, LongTag, FloatTag, DoubleTag, StringTag, ByteArrayTag, IntArrayTag, EndTag, Tag

### Combat & Damage
`DamageSource` (base), `EntityDamageSource`, `IndirectEntityDamageSource`
`MobEffect`, `MobEffectInstance`, `InstantenousMobEffect`, `PotionBrewing`
`Explosion`, `BlockDestructionProgress`

### Menus (Inventory UIs — server logic)
`AbstractContainerMenu` (base), `ContainerMenu`, `InventoryMenu`, `CraftingMenu`, `FurnaceMenu`, `BrewingStandMenu`, `RepairMenu` (anvil), `MerchantMenu`, `TrapMenu`
Containers: `Container`, `SimpleContainer`, `CompoundContainer`, `CraftingContainer`, `ResultContainer`, `PlayerEnderChestContainer`, `RepairContainer`, `MerchantContainer`
Slots: `Slot`, `ArmorSlot`, `ResultSlot`, `FurnaceResultSlot`, `RepairResultSlot`, `MerchantResultSlot`

### Achievements & Stats
`Achievement`, `Achievements` (40+ achievements), `Stat`, `Stats`, `CommonStats`, `GenericStats`, `GeneralStat`

### Utility / Infrastructure
**Math:** Mth, Vec3, AABB, BoundingBox, Pos, Coord, Facing, Direction, FastNoise, PerlinNoise, SimplexNoise, PerlinSimplexNoise, ImprovedNoise, SmoothFloat, Random, Mth
**IO streams:** InputStream, OutputStream, DataInputStream, DataOutputStream, FileInputStream, FileOutputStream, ByteArrayInputStream, ByteArrayOutputStream, BufferedOutputStream, BufferedReader, Reader, InputStreamReader, InputOutputStream, GZIPInputStream, GZIPOutputStream, Buffer, ByteBuffer, IntBuffer, FloatBuffer
**Collections:** IntCache, JavaIntHash, BinaryHeap, SparseDataStorage, SparseLightStorage, ArrayWithLength, BasicTypeContainers
**Threading:** C4JThread, ThreadName, PerformanceTimer
**Other:** I18n (localization), Language, StringHelpers, SharedConstants, Definitions, Exceptions, HashExtension, Hasher, Color, MaterialColor, WeighedRandom, WeighedTreasure, Rarity, SynchedEntityData, EntityIO, PlayerIO, NbtSlotFile, Path, PathFinder, Node, File, FileFilter, FileHeader, FilenameFilter, Village, Villages, VillageSiege, DoorInfo, PortalForcer

### Namespace header files
The `net.minecraft.*` headers in Minecraft.World/ are forward-declaration / namespace organization headers mirroring Java Minecraft's package structure. They are not actual Java code — they define C++ forward declarations.

---

## CI / GitHub

`.github/workflows/build.yml` — GitHub Actions: builds on windows-2022, MSBuild, Windows64 platform, uploads artifacts
`.github/workflows/nightly.yml` — Nightly build pipeline
`.github/pull_request_template.md` — PR template (previous behavior, root cause, new behavior, fix details)

---

## Quick Reference — "Where do I find...?"

| Want to change... | Look in |
|-------------------|---------|
| Main game loop / init | `Minecraft.Client/Minecraft.cpp` + `Windows64/Windows64_Minecraft.cpp` |
| Block rendering | `Minecraft.Client/TileRenderer.cpp` |
| Entity rendering | `Minecraft.Client/<Entity>Renderer.cpp` |
| Entity 3D models | `Minecraft.Client/<Entity>Model.cpp` |
| Particle effects | `Minecraft.Client/<Type>Particle.cpp`, `ParticleEngine.cpp` |
| HUD / overlay | `Minecraft.Client/Gui.cpp` |
| Menu screens | `Minecraft.Client/<Name>Screen.cpp` or `Common/UI/UIScene_<Name>.cpp` |
| Block behavior | `Minecraft.World/<Name>Tile.cpp` |
| Block entity logic | `Minecraft.World/<Name>TileEntity.cpp` |
| Mob behavior / AI | `Minecraft.World/<Mob>.cpp` + `*Goal.cpp` |
| Item behavior | `Minecraft.World/<Name>Item.cpp` |
| Crafting recipes | `Minecraft.World/Recipes.cpp` + `*Recipies.cpp` / `*Recipe.cpp` |
| Enchantments | `Minecraft.World/<Name>Enchantment.cpp` |
| World generation | `Minecraft.World/RandomLevelSource.cpp` + `*Feature.cpp` + `*Layer.cpp` |
| Biome definitions | `Minecraft.World/<Name>Biome.cpp` + `BiomeDecorator.cpp` |
| Network protocol | `Minecraft.World/<Name>Packet.cpp` |
| Chunk storage / saves | `Minecraft.World/ConsoleSaveFile*.cpp`, `McRegion*.cpp`, `ZonedChunkStorage.cpp` |
| Player logic | `Minecraft.World/Player.cpp`, `Minecraft.Client/LocalPlayer.cpp` |
| Keyboard/mouse input | `Minecraft.Client/Windows64/KeyboardMouseInput.cpp` |
| Controller input | `Minecraft.Client/<Platform>/<Platform>_UIController.cpp` |
| Game settings | `Minecraft.Client/Options.cpp`, `Settings.cpp` |
| DLC system | `Minecraft.Client/Common/DLC/DLCManager.cpp` |
| Tutorial system | `Minecraft.Client/Common/Tutorial/FullTutorial.cpp` |
| Console game rules | `Minecraft.Client/Common/GameRules/GameRuleManager.cpp` |
| Audio | `Minecraft.Client/Common/Audio/SoundEngine.cpp` |
| Build configuration | `CMakeLists.txt`, `cmake/*.cmake` |
| Platform abstraction | `Minecraft.Client/Common/Consoles_App.cpp` + `<Platform>/<Platform>_App.cpp` |
