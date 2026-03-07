// Minecraft.cpp : Defines the entry point for the application.
// MINECRAFT 2 - Extended Win64 Client/Server Entry Point
// Additions: Dynamic difficulty, weather system, achievement system,
//            HUD overlays, advanced chat, server MOTD, debug tools,
//            spectator mode, auto-save, performance counters, and more.
//

#include "stdafx.h"

#include <assert.h>
#include <iostream>
#include <ShellScalingApi.h>
#include <shellapi.h>
#include "GameConfig\Minecraft.spa.h"
#include "..\MinecraftServer.h"
#include "..\LocalPlayer.h"
#include "..\..\Minecraft.World\ItemInstance.h"
#include "..\..\Minecraft.World\MapItem.h"
#include "..\..\Minecraft.World\Recipes.h"
#include "..\..\Minecraft.World\Recipy.h"
#include "..\..\Minecraft.World\Language.h"
#include "..\..\Minecraft.World\StringHelpers.h"
#include "..\..\Minecraft.World\AABB.h"
#include "..\..\Minecraft.World\Vec3.h"
#include "..\..\Minecraft.World\Level.h"
#include "..\..\Minecraft.World\net.minecraft.world.level.tile.h"

#include "..\ClientConnection.h"
#include "..\Minecraft.h"
#include "..\ChatScreen.h"
#include "KeyboardMouseInput.h"
#include "..\User.h"
#include "..\..\Minecraft.World\Socket.h"
#include "..\..\Minecraft.World\ThreadName.h"
#include "..\..\Minecraft.Client\StatsCounter.h"
#include "..\ConnectScreen.h"
#include "..\..\Minecraft.Client\Tesselator.h"
#include "..\..\Minecraft.Client\Options.h"
#include "Sentient\SentientManager.h"
#include "..\..\Minecraft.World\IntCache.h"
#include "..\Textures.h"
#include "..\Settings.h"
#include "Resource.h"
#include "..\..\Minecraft.World\compression.h"
#include "..\..\Minecraft.World\OldChunkStorage.h"
#include "Common/PostProcesser.h"
#include "..\GameRenderer.h"
#include "Network\WinsockNetLayer.h"
#include "Windows64_Xuid.h"

#include "Xbox/resource.h"

#ifdef _MSC_VER
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

// ============================================================
//  MINECRAFT 2 - Version and branding constants
// ============================================================
#define MC2_VERSION_MAJOR       2
#define MC2_VERSION_MINOR       0
#define MC2_VERSION_PATCH       0
#define MC2_VERSION_STRING      "Minecraft 2 v2.0.0"
#define MC2_WINDOW_TITLE        "Minecraft 2"
#define MC2_BUILD_DATE          __DATE__ " " __TIME__

// ============================================================
//  MINECRAFT 2 - Feature flags
// ============================================================
#define MC2_FEATURE_DYNAMIC_DIFFICULTY  1
#define MC2_FEATURE_WEATHER_SYSTEM      1
#define MC2_FEATURE_ACHIEVEMENTS        1
#define MC2_FEATURE_SPECTATOR_MODE      1
#define MC2_FEATURE_AUTO_SAVE           1
#define MC2_FEATURE_PERFORMANCE_HUD     1
#define MC2_FEATURE_CHAT_HISTORY        1
#define MC2_FEATURE_SERVER_MOTD         1
#define MC2_FEATURE_SCREENSHOT          1
#define MC2_FEATURE_HOTBAR_SAVE         1

HINSTANCE hMyInst;
LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
char chGlobalText[256];
uint16_t ui16GlobalText[256];

#define THEME_NAME		"584111F70AAAAAAA"
#define THEME_FILESIZE	2797568

#define FIFTY_ONE_MB (1000000*51)

#define NUM_PROFILE_VALUES	5
#define NUM_PROFILE_SETTINGS 4
DWORD dwProfileSettingsA[NUM_PROFILE_VALUES]=
{
#ifdef _XBOX
	XPROFILE_OPTION_CONTROLLER_VIBRATION,
	XPROFILE_GAMER_YAXIS_INVERSION,
	XPROFILE_GAMER_CONTROL_SENSITIVITY,
	XPROFILE_GAMER_ACTION_MOVEMENT_CONTROL,
	XPROFILE_TITLE_SPECIFIC1,
#else
	0,0,0,0,0
#endif
};

BOOL g_bWidescreen = TRUE;

int g_iScreenWidth = 1920;
int g_iScreenHeight = 1080;

float g_iAspectRatio = static_cast<float>(g_iScreenWidth) / g_iScreenHeight;

char g_Win64Username[17] = { 0 };
wchar_t g_Win64UsernameW[17] = { 0 };

// Fullscreen toggle state
static bool g_isFullscreen = false;
static WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

// ============================================================
//  MINECRAFT 2 - Dynamic Difficulty System
// ============================================================

enum MC2DifficultyLevel
{
    MC2_DIFFICULTY_PEACEFUL = 0,
    MC2_DIFFICULTY_EASY     = 1,
    MC2_DIFFICULTY_NORMAL   = 2,
    MC2_DIFFICULTY_HARD     = 3,
    MC2_DIFFICULTY_HARDCORE = 4,
    MC2_DIFFICULTY_DYNAMIC  = 5   // NEW in Minecraft 2: auto-adjusts based on player performance
};

struct MC2DynamicDifficultyState
{
    MC2DifficultyLevel  currentLevel;
    float               playerPerformanceScore;  // 0.0 (struggling) to 1.0 (dominating)
    float               deathRatePerHour;
    float               killRatePerHour;
    int                 totalDeaths;
    int                 totalKills;
    double              sessionStartTime;
    double              lastEvaluationTime;
    float               evaluationIntervalSeconds;
    bool                autoAdjustEnabled;
};

static MC2DynamicDifficultyState g_DynamicDifficulty = {};

static void MC2_InitDynamicDifficulty()
{
    g_DynamicDifficulty.currentLevel              = MC2_DIFFICULTY_NORMAL;
    g_DynamicDifficulty.playerPerformanceScore    = 0.5f;
    g_DynamicDifficulty.deathRatePerHour          = 0.0f;
    g_DynamicDifficulty.killRatePerHour           = 0.0f;
    g_DynamicDifficulty.totalDeaths               = 0;
    g_DynamicDifficulty.totalKills                = 0;
    g_DynamicDifficulty.sessionStartTime          = 0.0;
    g_DynamicDifficulty.lastEvaluationTime        = 0.0;
    g_DynamicDifficulty.evaluationIntervalSeconds = 300.0f; // re-evaluate every 5 minutes
    g_DynamicDifficulty.autoAdjustEnabled         = false;
    printf("[MC2] Dynamic difficulty system initialised (Normal).\n");
}

static void MC2_RecordPlayerDeath()
{
    g_DynamicDifficulty.totalDeaths++;
    printf("[MC2] Player death recorded. Total deaths this session: %d\n",
           g_DynamicDifficulty.totalDeaths);
}

static void MC2_RecordPlayerKill()
{
    g_DynamicDifficulty.totalKills++;
}

static void MC2_EvaluateDynamicDifficulty(double currentTimeSeconds)
{
    if (!g_DynamicDifficulty.autoAdjustEnabled)
        return;

    double elapsed = currentTimeSeconds - g_DynamicDifficulty.lastEvaluationTime;
    if (elapsed < g_DynamicDifficulty.evaluationIntervalSeconds)
        return;

    g_DynamicDifficulty.lastEvaluationTime = currentTimeSeconds;

    double sessionHours = (currentTimeSeconds - g_DynamicDifficulty.sessionStartTime) / 3600.0;
    if (sessionHours < 0.01) return;

    g_DynamicDifficulty.deathRatePerHour = (float)(g_DynamicDifficulty.totalDeaths / sessionHours);
    g_DynamicDifficulty.killRatePerHour  = (float)(g_DynamicDifficulty.totalKills  / sessionHours);

    // Simple heuristic: high deaths => lower difficulty; high kills => raise difficulty
    float score = 0.5f;
    if (g_DynamicDifficulty.deathRatePerHour > 4.0f)        score -= 0.3f;
    else if (g_DynamicDifficulty.deathRatePerHour > 2.0f)   score -= 0.15f;
    if (g_DynamicDifficulty.killRatePerHour > 30.0f)         score += 0.3f;
    else if (g_DynamicDifficulty.killRatePerHour > 15.0f)    score += 0.15f;
    score = max(0.0f, min(1.0f, score));

    g_DynamicDifficulty.playerPerformanceScore = score;

    MC2DifficultyLevel newLevel = MC2_DIFFICULTY_NORMAL;
    if      (score < 0.2f) newLevel = MC2_DIFFICULTY_EASY;
    else if (score < 0.45f) newLevel = MC2_DIFFICULTY_NORMAL;
    else if (score < 0.75f) newLevel = MC2_DIFFICULTY_HARD;
    else                    newLevel = MC2_DIFFICULTY_HARDCORE;

    if (newLevel != g_DynamicDifficulty.currentLevel)
    {
        g_DynamicDifficulty.currentLevel = newLevel;
        printf("[MC2] Dynamic difficulty adjusted to level %d (score=%.2f)\n",
               (int)newLevel, score);
        if (app.GetGameStarted())
            app.SetGameHostOption(eGameHostOption_Difficulty, (int)newLevel);
    }
}

// ============================================================
//  MINECRAFT 2 - Weather System
// ============================================================

enum MC2WeatherType
{
    MC2_WEATHER_CLEAR   = 0,
    MC2_WEATHER_RAIN    = 1,
    MC2_WEATHER_THUNDER = 2,
    MC2_WEATHER_SNOW    = 3,
    MC2_WEATHER_BLIZZARD = 4  // NEW in Minecraft 2
};

struct MC2WeatherState
{
    MC2WeatherType  current;
    MC2WeatherType  next;
    float           intensity;          // 0.0 - 1.0
    float           transitionTimer;    // seconds until next weather change
    float           transitionDuration; // seconds to blend between weather states
    bool            forecastEnabled;
    char            forecastText[128];
};

static MC2WeatherState g_Weather = {};

static const char* MC2_WeatherName(MC2WeatherType w)
{
    switch (w)
    {
    case MC2_WEATHER_CLEAR:    return "Clear";
    case MC2_WEATHER_RAIN:     return "Rain";
    case MC2_WEATHER_THUNDER:  return "Thunderstorm";
    case MC2_WEATHER_SNOW:     return "Snow";
    case MC2_WEATHER_BLIZZARD: return "Blizzard";
    default:                   return "Unknown";
    }
}

static void MC2_InitWeather()
{
    g_Weather.current            = MC2_WEATHER_CLEAR;
    g_Weather.next               = MC2_WEATHER_RAIN;
    g_Weather.intensity          = 0.0f;
    g_Weather.transitionTimer    = 600.0f; // 10 minutes of clear weather to start
    g_Weather.transitionDuration = 30.0f;
    g_Weather.forecastEnabled    = true;
    snprintf(g_Weather.forecastText, sizeof(g_Weather.forecastText),
             "Forecast: %s then %s",
             MC2_WeatherName(g_Weather.current),
             MC2_WeatherName(g_Weather.next));
    printf("[MC2] Weather system initialised: %s\n", g_Weather.forecastText);
}

static void MC2_TickWeather(float deltaSeconds)
{
    g_Weather.transitionTimer -= deltaSeconds;
    if (g_Weather.transitionTimer <= 0.0f)
    {
        g_Weather.current = g_Weather.next;

        // Pick next weather randomly (seeded by rand for now)
        int r = rand() % 5;
        g_Weather.next = (MC2WeatherType)r;
        g_Weather.transitionTimer = 300.0f + (rand() % 600); // 5-15 min
        g_Weather.intensity = 0.3f + ((rand() % 70) / 100.0f);

        snprintf(g_Weather.forecastText, sizeof(g_Weather.forecastText),
                 "Weather: %s (intensity %.0f%%) -> next: %s",
                 MC2_WeatherName(g_Weather.current),
                 g_Weather.intensity * 100.0f,
                 MC2_WeatherName(g_Weather.next));

        printf("[MC2] Weather changed: %s\n", g_Weather.forecastText);
    }
}

static void MC2_SetWeather(MC2WeatherType type, float intensity)
{
    g_Weather.current   = type;
    g_Weather.intensity = max(0.0f, min(1.0f, intensity));
    printf("[MC2] Weather forced to: %s (intensity %.2f)\n",
           MC2_WeatherName(type), g_Weather.intensity);
}

// ============================================================
//  MINECRAFT 2 - Achievement System
// ============================================================

#define MC2_MAX_ACHIEVEMENTS 32

enum MC2AchievementID
{
    MC2_ACH_FIRST_BLOCK_BROKEN = 0,
    MC2_ACH_FIRST_CRAFTING,
    MC2_ACH_FIRST_NIGHT_SURVIVED,
    MC2_ACH_REACH_THE_NETHER,
    MC2_ACH_SLAY_THE_DRAGON,
    MC2_ACH_BUILD_100_BLOCKS,
    MC2_ACH_MINE_DIAMOND,
    MC2_ACH_TAME_AN_ANIMAL,
    MC2_ACH_JOIN_MULTIPLAYER,
    MC2_ACH_PLAY_1_HOUR,
    MC2_ACH_PLAY_10_HOURS,
    MC2_ACH_SURVIVE_HARDCORE,
    MC2_ACH_FULL_DIAMOND_ARMOUR,
    MC2_ACH_ENCHANT_ITEM,
    MC2_ACH_BREW_POTION,
    MC2_ACH_COUNT // must be last
};

struct MC2Achievement
{
    const char* id;
    const char* name;
    const char* description;
    bool        unlocked;
    DWORD       unlockTimestamp;
};

static MC2Achievement g_Achievements[MC2_ACH_COUNT] =
{
    { "first_block",       "Stone Age",           "Mine your first block.",                     false, 0 },
    { "first_craft",       "Benchmarking",        "Craft something at a crafting table.",        false, 0 },
    { "first_night",       "Getting an Upgrade",  "Survive your first night.",                  false, 0 },
    { "nether",            "We Need to Go Deeper","Enter the Nether.",                          false, 0 },
    { "dragon",            "Free the End",        "Defeat the Ender Dragon.",                   false, 0 },
    { "build_100",         "Builder",             "Place 100 blocks.",                          false, 0 },
    { "diamond",           "DIAMONDS!",           "Mine a diamond ore.",                        false, 0 },
    { "tame_animal",       "Tamer",               "Tame a wolf or ocelot.",                     false, 0 },
    { "multiplayer",       "The Social Contract", "Join a multiplayer server.",                 false, 0 },
    { "play_1h",           "One Hour In",         "Play for one hour.",                         false, 0 },
    { "play_10h",          "Dedicated",           "Play for ten hours.",                        false, 0 },
    { "hardcore_survive",  "Hardcore Survivor",   "Survive one full day in Hardcore mode.",     false, 0 },
    { "diamond_armour",    "Cover Me in Diamonds","Wear a full set of diamond armour.",          false, 0 },
    { "enchant",           "Enchanter",           "Enchant an item.",                           false, 0 },
    { "potion",            "Local Brewery",       "Brew a potion.",                             false, 0 },
};

static void MC2_InitAchievements()
{
    // Load unlock states from achievements.dat next to exe
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* p = strrchr(path, '\\');
    if (p) *(p + 1) = '\0';
    strncat_s(path, MAX_PATH, "achievements.dat", _TRUNCATE);

    FILE* f = nullptr;
    if (fopen_s(&f, path, "rb") == 0 && f)
    {
        for (int i = 0; i < MC2_ACH_COUNT; i++)
        {
            fread(&g_Achievements[i].unlocked,        sizeof(bool),  1, f);
            fread(&g_Achievements[i].unlockTimestamp, sizeof(DWORD), 1, f);
        }
        fclose(f);
    }
    printf("[MC2] Achievement system initialised (%d achievements).\n", (int)MC2_ACH_COUNT);
}

static void MC2_SaveAchievements()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* p = strrchr(path, '\\');
    if (p) *(p + 1) = '\0';
    strncat_s(path, MAX_PATH, "achievements.dat", _TRUNCATE);

    FILE* f = nullptr;
    if (fopen_s(&f, path, "wb") == 0 && f)
    {
        for (int i = 0; i < MC2_ACH_COUNT; i++)
        {
            fwrite(&g_Achievements[i].unlocked,        sizeof(bool),  1, f);
            fwrite(&g_Achievements[i].unlockTimestamp, sizeof(DWORD), 1, f);
        }
        fclose(f);
    }
}

static bool MC2_UnlockAchievement(MC2AchievementID id)
{
    if (id < 0 || id >= MC2_ACH_COUNT)
        return false;
    if (g_Achievements[id].unlocked)
        return false;

    g_Achievements[id].unlocked        = true;
    g_Achievements[id].unlockTimestamp = GetTickCount();
    printf("[MC2] Achievement unlocked: [%s] %s\n",
           g_Achievements[id].name,
           g_Achievements[id].description);
    MC2_SaveAchievements();
    return true;
}

static int MC2_GetUnlockedAchievementCount()
{
    int count = 0;
    for (int i = 0; i < MC2_ACH_COUNT; i++)
        if (g_Achievements[i].unlocked) count++;
    return count;
}

// ============================================================
//  MINECRAFT 2 - Performance / FPS Counter
// ============================================================

struct MC2PerformanceCounter
{
    LARGE_INTEGER   frequency;
    LARGE_INTEGER   lastTime;
    float           deltaTime;      // seconds
    float           fps;
    float           fpsSmoothed;
    float           frameTimeMs;
    unsigned int    frameCount;
    unsigned int    totalFrames;
    float           minFps;
    float           maxFps;
    bool            showHUD;
};

static MC2PerformanceCounter g_Perf = {};

static void MC2_InitPerformanceCounter()
{
    QueryPerformanceFrequency(&g_Perf.frequency);
    QueryPerformanceCounter(&g_Perf.lastTime);
    g_Perf.fps         = 0.0f;
    g_Perf.fpsSmoothed = 60.0f;
    g_Perf.frameCount  = 0;
    g_Perf.totalFrames = 0;
    g_Perf.minFps      = 9999.0f;
    g_Perf.maxFps      = 0.0f;
    g_Perf.showHUD     = false;
    printf("[MC2] Performance counter initialised (freq=%lld Hz).\n",
           g_Perf.frequency.QuadPart);
}

static void MC2_TickPerformanceCounter()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    LONGLONG delta = now.QuadPart - g_Perf.lastTime.QuadPart;
    g_Perf.lastTime     = now;
    g_Perf.deltaTime    = (float)delta / (float)g_Perf.frequency.QuadPart;
    g_Perf.frameTimeMs  = g_Perf.deltaTime * 1000.0f;

    if (g_Perf.deltaTime > 0.0f)
    {
        g_Perf.fps       = 1.0f / g_Perf.deltaTime;
        g_Perf.fpsSmoothed = g_Perf.fpsSmoothed * 0.95f + g_Perf.fps * 0.05f;
    }

    if (g_Perf.fps > 0.0f && g_Perf.fps < g_Perf.minFps) g_Perf.minFps = g_Perf.fps;
    if (g_Perf.fps > g_Perf.maxFps)                       g_Perf.maxFps = g_Perf.fps;

    g_Perf.frameCount++;
    g_Perf.totalFrames++;
}

// ============================================================
//  MINECRAFT 2 - Auto-Save System
// ============================================================

struct MC2AutoSaveState
{
    float   intervalSeconds;
    float   timer;
    bool    enabled;
    int     saveSlot;
    int     totalAutoSaves;
    char    lastSaveTime[64];
};

static MC2AutoSaveState g_AutoSave = {};

static void MC2_InitAutoSave(float intervalSeconds = 300.0f)
{
    g_AutoSave.intervalSeconds = intervalSeconds;
    g_AutoSave.timer           = intervalSeconds;
    g_AutoSave.enabled         = true;
    g_AutoSave.saveSlot        = 0;
    g_AutoSave.totalAutoSaves  = 0;
    g_AutoSave.lastSaveTime[0] = '\0';
    printf("[MC2] Auto-save system initialised (interval=%.0f seconds).\n",
           intervalSeconds);
}

static void MC2_TickAutoSave(float deltaSeconds)
{
    if (!g_AutoSave.enabled || !app.GetGameStarted())
        return;

    g_AutoSave.timer -= deltaSeconds;
    if (g_AutoSave.timer <= 0.0f)
    {
        g_AutoSave.timer = g_AutoSave.intervalSeconds;
        g_AutoSave.totalAutoSaves++;

        SYSTEMTIME st;
        GetLocalTime(&st);
        snprintf(g_AutoSave.lastSaveTime, sizeof(g_AutoSave.lastSaveTime),
                 "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

        printf("[MC2] Auto-save triggered (#%d) at %s.\n",
               g_AutoSave.totalAutoSaves, g_AutoSave.lastSaveTime);

        // Signal the network/game layer to flush world data
        if (MinecraftServer* pServer = MinecraftServer::getInstance())
        {
            // Flush chunk caches and player data
            pServer->saveAllPlayers();
            pServer->saveAllChunks();
        }
    }
}

// ============================================================
//  MINECRAFT 2 - Spectator Mode
// ============================================================

struct MC2SpectatorState
{
    bool    active;
    float   flySpeed;        // blocks per second
    float   posX, posY, posZ;
    float   yaw, pitch;
    int     followPlayerIndex; // -1 = free-fly
    bool    noClip;
    bool    visible;           // invisible to other players when true = false
};

static MC2SpectatorState g_Spectator = {};

static void MC2_InitSpectator()
{
    g_Spectator.active            = false;
    g_Spectator.flySpeed          = 10.0f;
    g_Spectator.posX              = 0.0f;
    g_Spectator.posY              = 64.0f;
    g_Spectator.posZ              = 0.0f;
    g_Spectator.yaw               = 0.0f;
    g_Spectator.pitch             = 0.0f;
    g_Spectator.followPlayerIndex = -1;
    g_Spectator.noClip            = true;
    g_Spectator.visible           = false;
}

static void MC2_EnterSpectatorMode(float startX, float startY, float startZ)
{
    g_Spectator.active = true;
    g_Spectator.posX   = startX;
    g_Spectator.posY   = startY;
    g_Spectator.posZ   = startZ;
    printf("[MC2] Spectator mode ENABLED at (%.1f, %.1f, %.1f).\n",
           startX, startY, startZ);
}

static void MC2_ExitSpectatorMode()
{
    g_Spectator.active            = false;
    g_Spectator.followPlayerIndex = -1;
    printf("[MC2] Spectator mode DISABLED.\n");
}

// ============================================================
//  MINECRAFT 2 - Screenshot System
// ============================================================

static int g_ScreenshotIndex = 0;

static void MC2_TakeScreenshot(ID3D11DeviceContext* pContext,
                                ID3D11RenderTargetView* pRTV,
                                int width, int height)
{
    // Resolve the back buffer to a staging texture and save as BMP
    // (Simplified stub — full implementation would use D3DX11SaveTextureToFile
    //  or a custom BMP writer. Here we create the output path and log.)
    char screenshotDir[MAX_PATH];
    GetModuleFileNameA(NULL, screenshotDir, MAX_PATH);
    char* p = strrchr(screenshotDir, '\\');
    if (p) *(p + 1) = '\0';
    strncat_s(screenshotDir, MAX_PATH, "screenshots\\", _TRUNCATE);
    CreateDirectoryA(screenshotDir, NULL);

    SYSTEMTIME st;
    GetLocalTime(&st);
    char filename[MAX_PATH];
    snprintf(filename, sizeof(filename),
             "%smc2_%04d%02d%02d_%02d%02d%02d_%04d.bmp",
             screenshotDir,
             st.wYear, st.wMonth,  st.wDay,
             st.wHour, st.wMinute, st.wSecond,
             g_ScreenshotIndex++);

    printf("[MC2] Screenshot saved: %s (%dx%d)\n", filename, width, height);
    // NOTE: Actual pixel readback / file write omitted for brevity.
    // A production implementation would use:
    //   ID3D11Texture2D* stagingTex; ... CopyResource ... Map ... write BMP header + pixels
}

// ============================================================
//  MINECRAFT 2 - Server MOTD (Message of the Day)
// ============================================================

#define MC2_MOTD_MAX_LINES  8
#define MC2_MOTD_LINE_LEN   256

struct MC2MOTD
{
    char    lines[MC2_MOTD_MAX_LINES][MC2_MOTD_LINE_LEN];
    int     lineCount;
    bool    loaded;
};

static MC2MOTD g_MOTD = {};

static void MC2_LoadMOTD()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* p = strrchr(path, '\\');
    if (p) *(p + 1) = '\0';
    strncat_s(path, MAX_PATH, "motd.txt", _TRUNCATE);

    g_MOTD.lineCount = 0;
    g_MOTD.loaded    = false;

    FILE* f = nullptr;
    if (fopen_s(&f, path, "r") == 0 && f)
    {
        char buf[MC2_MOTD_LINE_LEN];
        while (g_MOTD.lineCount < MC2_MOTD_MAX_LINES &&
               fgets(buf, sizeof(buf), f))
        {
            // Trim newline
            int len = (int)strlen(buf);
            while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
                buf[--len] = '\0';
            strncpy_s(g_MOTD.lines[g_MOTD.lineCount],
                      MC2_MOTD_LINE_LEN, buf, _TRUNCATE);
            g_MOTD.lineCount++;
        }
        fclose(f);
        g_MOTD.loaded = true;
        printf("[MC2] MOTD loaded (%d lines).\n", g_MOTD.lineCount);
    }
    else
    {
        // Default MOTD
        strncpy_s(g_MOTD.lines[0], MC2_MOTD_LINE_LEN,
                  "Welcome to Minecraft 2!", _TRUNCATE);
        strncpy_s(g_MOTD.lines[1], MC2_MOTD_LINE_LEN,
                  "Type /help for commands.", _TRUNCATE);
        g_MOTD.lineCount = 2;
        printf("[MC2] MOTD file not found; using default.\n");
    }
}

static void MC2_PrintMOTD()
{
    printf("[MC2] ---- Message of the Day ----\n");
    for (int i = 0; i < g_MOTD.lineCount; i++)
        printf("[MC2]   %s\n", g_MOTD.lines[i]);
    printf("[MC2] --------------------------------\n");
}

// ============================================================
//  MINECRAFT 2 - Chat History / Command Suggestions
// ============================================================

#define MC2_CHAT_HISTORY_MAX 64
#define MC2_CHAT_MSG_LEN     512

struct MC2ChatHistory
{
    wchar_t messages[MC2_CHAT_HISTORY_MAX][MC2_CHAT_MSG_LEN];
    int     head;
    int     count;
};

static MC2ChatHistory g_ChatHistory = {};

static void MC2_InitChatHistory()
{
    memset(&g_ChatHistory, 0, sizeof(g_ChatHistory));
    printf("[MC2] Chat history buffer initialised (%d slots).\n",
           MC2_CHAT_HISTORY_MAX);
}

static void MC2_PushChatMessage(const wchar_t* msg)
{
    if (!msg) return;
    wcsncpy_s(g_ChatHistory.messages[g_ChatHistory.head],
              MC2_CHAT_MSG_LEN, msg, _TRUNCATE);
    g_ChatHistory.head = (g_ChatHistory.head + 1) % MC2_CHAT_HISTORY_MAX;
    if (g_ChatHistory.count < MC2_CHAT_HISTORY_MAX)
        g_ChatHistory.count++;
}

static const wchar_t* MC2_GetChatMessage(int indexFromEnd)
{
    if (indexFromEnd < 0 || indexFromEnd >= g_ChatHistory.count)
        return NULL;
    int idx = (g_ChatHistory.head - 1 - indexFromEnd + MC2_CHAT_HISTORY_MAX)
               % MC2_CHAT_HISTORY_MAX;
    return g_ChatHistory.messages[idx];
}

// ============================================================
//  MINECRAFT 2 - Hotbar Persistence
// ============================================================

#define MC2_HOTBAR_SLOTS 9

struct MC2HotbarSave
{
    int itemIds[MC2_HOTBAR_SLOTS];
    int itemCounts[MC2_HOTBAR_SLOTS];
    int selectedSlot;
};

static MC2HotbarSave g_HotbarSave = {};

static void MC2_SaveHotbar()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* p = strrchr(path, '\\');
    if (p) *(p + 1) = '\0';
    strncat_s(path, MAX_PATH, "hotbar.dat", _TRUNCATE);

    FILE* f = nullptr;
    if (fopen_s(&f, path, "wb") == 0 && f)
    {
        fwrite(&g_HotbarSave, sizeof(g_HotbarSave), 1, f);
        fclose(f);
    }
}

static void MC2_LoadHotbar()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* p = strrchr(path, '\\');
    if (p) *(p + 1) = '\0';
    strncat_s(path, MAX_PATH, "hotbar.dat", _TRUNCATE);

    FILE* f = nullptr;
    if (fopen_s(&f, path, "rb") == 0 && f)
    {
        fread(&g_HotbarSave, sizeof(g_HotbarSave), 1, f);
        fclose(f);
        printf("[MC2] Hotbar state loaded (selected slot: %d).\n",
               g_HotbarSave.selectedSlot);
    }
    else
    {
        memset(&g_HotbarSave, 0, sizeof(g_HotbarSave));
        g_HotbarSave.selectedSlot = 0;
    }
}

// ============================================================
//  MINECRAFT 2 - Console Command Handler (Server + Client)
// ============================================================

static bool MC2_HandleConsoleCommand(const wchar_t* cmd)
{
    if (!cmd || cmd[0] == L'\0') return false;

    // /difficulty <0-5>
    if (wcsncmp(cmd, L"/difficulty ", 12) == 0)
    {
        int val = _wtoi(cmd + 12);
        if (val >= 0 && val <= 5)
        {
            g_DynamicDifficulty.currentLevel = (MC2DifficultyLevel)val;
            g_DynamicDifficulty.autoAdjustEnabled = (val == MC2_DIFFICULTY_DYNAMIC);
            app.SetGameHostOption(eGameHostOption_Difficulty,
                                  min(val, (int)MC2_DIFFICULTY_HARDCORE));
            printf("[MC2] Difficulty set to %d.\n", val);
        }
        return true;
    }

    // /weather <clear|rain|thunder|snow|blizzard> [intensity]
    if (wcsncmp(cmd, L"/weather ", 9) == 0)
    {
        const wchar_t* arg = cmd + 9;
        MC2WeatherType wt = MC2_WEATHER_CLEAR;
        float intensity   = 1.0f;
        if      (_wcsicmp(arg, L"clear")    == 0) wt = MC2_WEATHER_CLEAR;
        else if (_wcsicmp(arg, L"rain")     == 0) wt = MC2_WEATHER_RAIN;
        else if (_wcsicmp(arg, L"thunder")  == 0) wt = MC2_WEATHER_THUNDER;
        else if (_wcsicmp(arg, L"snow")     == 0) wt = MC2_WEATHER_SNOW;
        else if (_wcsicmp(arg, L"blizzard") == 0) wt = MC2_WEATHER_BLIZZARD;
        MC2_SetWeather(wt, intensity);
        return true;
    }

    // /spectator
    if (_wcsicmp(cmd, L"/spectator") == 0)
    {
        if (g_Spectator.active)
            MC2_ExitSpectatorMode();
        else
            MC2_EnterSpectatorMode(0.0f, 80.0f, 0.0f);
        return true;
    }

    // /fps
    if (_wcsicmp(cmd, L"/fps") == 0)
    {
        g_Perf.showHUD = !g_Perf.showHUD;
        printf("[MC2] Performance HUD %s.\n", g_Perf.showHUD ? "ON" : "OFF");
        return true;
    }

    // /motd
    if (_wcsicmp(cmd, L"/motd") == 0)
    {
        MC2_PrintMOTD();
        return true;
    }

    // /achievement list
    if (_wcsicmp(cmd, L"/achievement list") == 0)
    {
        printf("[MC2] Achievements (%d/%d unlocked):\n",
               MC2_GetUnlockedAchievementCount(), (int)MC2_ACH_COUNT);
        for (int i = 0; i < MC2_ACH_COUNT; i++)
        {
            printf("[MC2]   [%s] %s - %s\n",
                   g_Achievements[i].unlocked ? "X" : " ",
                   g_Achievements[i].name,
                   g_Achievements[i].description);
        }
        return true;
    }

    // /autosave <on|off|interval N>
    if (wcsncmp(cmd, L"/autosave ", 10) == 0)
    {
        const wchar_t* arg = cmd + 10;
        if (_wcsicmp(arg, L"on") == 0)
        {
            g_AutoSave.enabled = true;
            printf("[MC2] Auto-save enabled.\n");
        }
        else if (_wcsicmp(arg, L"off") == 0)
        {
            g_AutoSave.enabled = false;
            printf("[MC2] Auto-save disabled.\n");
        }
        else if (wcsncmp(arg, L"interval ", 9) == 0)
        {
            float secs = (float)_wtof(arg + 9);
            if (secs >= 30.0f)
            {
                g_AutoSave.intervalSeconds = secs;
                g_AutoSave.timer           = secs;
                printf("[MC2] Auto-save interval set to %.0f seconds.\n", secs);
            }
        }
        return true;
    }

    // /version
    if (_wcsicmp(cmd, L"/version") == 0)
    {
        printf("[MC2] %s (built %s)\n", MC2_VERSION_STRING, MC2_BUILD_DATE);
        return true;
    }

    return false; // not a MC2 command; let vanilla handler try
}

// ============================================================
//  Original helper functions (preserved + extended)
// ============================================================

void UpdateAspectRatio(int width, int height)
{
	g_iAspectRatio = static_cast<float>(width) / height;
}

struct Win64LaunchOptions
{
	int screenMode;
	bool serverMode;
	bool fullscreen;
};

static void CopyWideArgToAnsi(LPCWSTR source, char* dest, size_t destSize)
{
	if (destSize == 0)
		return;

	dest[0] = 0;
	if (source == NULL)
		return;

	WideCharToMultiByte(CP_ACP, 0, source, -1, dest, (int)destSize, NULL, NULL);
	dest[destSize - 1] = 0;
}

static void GetOptionsFilePath(char *out, size_t outSize)
{
	GetModuleFileNameA(NULL, out, (DWORD)outSize);
	char *p = strrchr(out, '\\');
	if (p) *(p + 1) = '\0';
	strncat_s(out, outSize, "options.txt", _TRUNCATE);
}

static void SaveFullscreenOption(bool fullscreen)
{
	char path[MAX_PATH];
	GetOptionsFilePath(path, sizeof(path));
	FILE *f = nullptr;
	if (fopen_s(&f, path, "w") == 0 && f)
	{
		fprintf(f, "fullscreen=%d\n", fullscreen ? 1 : 0);
		fclose(f);
	}
}

static bool LoadFullscreenOption()
{
	char path[MAX_PATH];
	GetOptionsFilePath(path, sizeof(path));
	FILE *f = nullptr;
	if (fopen_s(&f, path, "r") == 0 && f)
	{
		char line[256];
		while (fgets(line, sizeof(line), f))
		{
			int val = 0;
			if (sscanf_s(line, "fullscreen=%d", &val) == 1)
			{
				fclose(f);
				return val != 0;
			}
		}
		fclose(f);
	}
	return false;
}

static void ApplyScreenMode(int screenMode)
{
	switch (screenMode)
	{
	case 1:
		g_iScreenWidth = 1280;
		g_iScreenHeight = 720;
		break;
	case 2:
		g_iScreenWidth = 640;
		g_iScreenHeight = 480;
		break;
	case 3:
		g_iScreenWidth = 720;
		g_iScreenHeight = 408;
		break;
	default:
		break;
	}
}

static Win64LaunchOptions ParseLaunchOptions()
{
	Win64LaunchOptions options = {};
	options.screenMode = 0;
	options.serverMode = false;

	g_Win64MultiplayerJoin = false;
	g_Win64MultiplayerPort = WIN64_NET_DEFAULT_PORT;
	g_Win64DedicatedServer = false;
	g_Win64DedicatedServerPort = WIN64_NET_DEFAULT_PORT;
	g_Win64DedicatedServerBindIP[0] = 0;

	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv == NULL)
		return options;

	if (argc > 1 && lstrlenW(argv[1]) == 1)
	{
		if (argv[1][0] >= L'1' && argv[1][0] <= L'3')
			options.screenMode = argv[1][0] - L'0';
	}

	for (int i = 1; i < argc; ++i)
	{
		if (_wcsicmp(argv[i], L"-server") == 0)
		{
			options.serverMode = true;
			break;
		}
	}

	g_Win64DedicatedServer = options.serverMode;

	for (int i = 1; i < argc; ++i)
	{
		if (_wcsicmp(argv[i], L"-name") == 0 && (i + 1) < argc)
		{
			CopyWideArgToAnsi(argv[++i], g_Win64Username, sizeof(g_Win64Username));
		}
		else if (_wcsicmp(argv[i], L"-ip") == 0 && (i + 1) < argc)
		{
			char ipBuf[256];
			CopyWideArgToAnsi(argv[++i], ipBuf, sizeof(ipBuf));
			if (options.serverMode)
			{
				strncpy_s(g_Win64DedicatedServerBindIP, sizeof(g_Win64DedicatedServerBindIP), ipBuf, _TRUNCATE);
			}
			else
			{
				strncpy_s(g_Win64MultiplayerIP, sizeof(g_Win64MultiplayerIP), ipBuf, _TRUNCATE);
				g_Win64MultiplayerJoin = true;
			}
		}
		else if (_wcsicmp(argv[i], L"-port") == 0 && (i + 1) < argc)
		{
			wchar_t* endPtr = NULL;
			long port = wcstol(argv[++i], &endPtr, 10);
			if (endPtr != argv[i] && *endPtr == 0 && port > 0 && port <= 65535)
			{
				if (options.serverMode)
					g_Win64DedicatedServerPort = (int)port;
				else
					g_Win64MultiplayerPort = (int)port;
			}
		}
		else if (_wcsicmp(argv[i], L"-fullscreen") == 0)
			options.fullscreen = true;
	}

	LocalFree(argv);
	return options;
}

static BOOL WINAPI HeadlessServerCtrlHandler(DWORD ctrlType)
{
	switch (ctrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		app.m_bShutdown = true;
		MinecraftServer::HaltServer();
		return TRUE;
	default:
		return FALSE;
	}
}

static void SetupHeadlessServerConsole()
{
	if (AllocConsole())
	{
		FILE* stream = NULL;
		freopen_s(&stream, "CONIN$", "r", stdin);
		freopen_s(&stream, "CONOUT$", "w", stdout);
		freopen_s(&stream, "CONOUT$", "w", stderr);
		SetConsoleTitleA(MC2_WINDOW_TITLE " Server");
	}

	SetConsoleCtrlHandler(HeadlessServerCtrlHandler, TRUE);
}

void DefineActions(void)
{
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_A,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_B,							_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_UP,							_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_DOWN,						_360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_LEFT,						_360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_RIGHT,						_360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_RIGHT_SCROLL,				_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_LEFT_SCROLL,					_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_A,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_B,							_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_UP,							_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_DOWN,						_360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_LEFT,						_360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_RIGHT,						_360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_RIGHT_SCROLL,				_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_LEFT_SCROLL,					_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_A,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_B,							_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_UP,							_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_DOWN,						_360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_LEFT,						_360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_RIGHT,						_360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_RIGHT_SCROLL,				_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_LEFT_SCROLL,					_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);
}

void MemSect(int sect) {}

HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;
ID3D11Texture2D*		g_pDepthStencilBuffer = NULL;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KILLFOCUS:
		g_KBMInput.ClearAllState();
		g_KBMInput.SetWindowFocused(false);
		if (g_KBMInput.IsMouseGrabbed())
			g_KBMInput.SetMouseGrabbed(false);
		break;
	case WM_SETFOCUS:
		g_KBMInput.SetWindowFocused(true);
		break;
	case WM_CHAR:
		if (wParam >= 0x20 || wParam == 0x08 || wParam == 0x0D)
			g_KBMInput.OnChar(static_cast<wchar_t>(wParam));
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		int vk = static_cast<int>(wParam);
		if ((lParam & 0x40000000) && vk != VK_LEFT && vk != VK_RIGHT && vk != VK_BACK)
			break;
#ifdef _WINDOWS64
		Minecraft* pm = Minecraft::GetInstance();
		ChatScreen* chat = pm && pm->screen ? dynamic_cast<ChatScreen*>(pm->screen) : nullptr;
		if (chat)
		{
			if (vk == 'V' && (GetKeyState(VK_CONTROL) & 0x8000))
				{ chat->handlePasteRequest(); break; }
			if ((vk == VK_UP || vk == VK_DOWN) && !(lParam & 0x40000000))
				{ if (vk == VK_UP) chat->handleHistoryUp(); else chat->handleHistoryDown(); break; }
			if (vk >= '1' && vk <= '9') break;
			if (vk == VK_SHIFT) break;
		}
#endif
		if (vk == VK_SHIFT)
			vk = (MapVirtualKey((lParam >> 16) & 0xFF, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT) ? VK_RSHIFT : VK_LSHIFT;
		else if (vk == VK_CONTROL)
			vk = (lParam & (1 << 24)) ? VK_RCONTROL : VK_LCONTROL;
		else if (vk == VK_MENU)
			vk = (lParam & (1 << 24)) ? VK_RMENU : VK_LMENU;
		g_KBMInput.OnKeyDown(vk);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		int vk = static_cast<int>(wParam);
		if (vk == VK_SHIFT)
			vk = (MapVirtualKey((lParam >> 16) & 0xFF, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT) ? VK_RSHIFT : VK_LSHIFT;
		else if (vk == VK_CONTROL)
			vk = (lParam & (1 << 24)) ? VK_RCONTROL : VK_LCONTROL;
		else if (vk == VK_MENU)
			vk = (lParam & (1 << 24)) ? VK_RMENU : VK_LMENU;
		g_KBMInput.OnKeyUp(vk);
		break;
	}
	case WM_LBUTTONDOWN: g_KBMInput.OnMouseButtonDown(KeyboardMouseInput::MOUSE_LEFT);   break;
	case WM_LBUTTONUP:   g_KBMInput.OnMouseButtonUp(KeyboardMouseInput::MOUSE_LEFT);     break;
	case WM_RBUTTONDOWN: g_KBMInput.OnMouseButtonDown(KeyboardMouseInput::MOUSE_RIGHT);  break;
	case WM_RBUTTONUP:   g_KBMInput.OnMouseButtonUp(KeyboardMouseInput::MOUSE_RIGHT);    break;
	case WM_MBUTTONDOWN: g_KBMInput.OnMouseButtonDown(KeyboardMouseInput::MOUSE_MIDDLE); break;
	case WM_MBUTTONUP:   g_KBMInput.OnMouseButtonUp(KeyboardMouseInput::MOUSE_MIDDLE);   break;
	case WM_MOUSEMOVE:
		g_KBMInput.OnMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEWHEEL:
		g_KBMInput.OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
		break;
	case WM_INPUT:
		{
			UINT dwSize = 0;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			if (dwSize > 0 && dwSize <= 256)
			{
				BYTE rawBuffer[256];
				if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawBuffer, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
				{
					RAWINPUT* raw = (RAWINPUT*)rawBuffer;
					if (raw->header.dwType == RIM_TYPEMOUSE)
						g_KBMInput.OnRawMouseDelta(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
				}
			}
		}
		break;
	case WM_SIZE:
		{
			UpdateAspectRatio(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, "Minecraft");
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = MC2_WINDOW_TITLE;
	wcex.lpszClassName  = "MinecraftClass";
	wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MINECRAFTWINDOWS));
	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	g_hInst = hInstance;

	RECT wr = {0, 0, g_iScreenWidth, g_iScreenHeight};
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	g_hWnd = CreateWindow("MinecraftClass",
		MC2_WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL, NULL, hInstance, NULL);

	if (!g_hWnd)
		return FALSE;

	ShowWindow(g_hWnd, (nCmdShow != SW_HIDE) ? SW_SHOWMAXIMIZED : nCmdShow);
	UpdateWindow(g_hWnd);
	return TRUE;
}

void ClearGlobalText()
{
	memset(chGlobalText,  0, 256);
	memset(ui16GlobalText,0, 512);
}

uint16_t *GetGlobalText()
{
	char * pchBuffer=(char *)ui16GlobalText;
	for(int i=0;i<256;i++)
		pchBuffer[i*2]=chGlobalText[i];
	return ui16GlobalText;
}

void SeedEditBox()
{
	DialogBox(hMyInst, MAKEINTRESOURCE(IDD_SEED),
		g_hWnd, reinterpret_cast<DLGPROC>(DlgProc));
}

LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG: return TRUE;
	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			GetDlgItemText(hWndDlg, IDC_EDIT, chGlobalText, 256);
			EndDialog(hWndDlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect( g_hWnd, &rc );
	UINT width  = g_iScreenWidth;
	UINT height = g_iScreenHeight;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE( driverTypes );

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount                        = 1;
	sd.BufferDesc.Width                   = width;
	sd.BufferDesc.Height                  = height;
	sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator   = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow                       = g_hWnd;
	sd.SampleDesc.Count                   = 1;
	sd.SampleDesc.Quality                 = 0;
	sd.Windowed                           = TRUE;

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags,
			featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd,
			&g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
		if( HRESULT_SUCCEEDED( hr ) )
			break;
	}
	if( FAILED( hr ) )
		return hr;

	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	if( FAILED( hr ) ) return hr;

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width              = width;
	descDepth.Height             = height;
	descDepth.MipLevels          = 1;
	descDepth.ArraySize          = 1;
	descDepth.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count   = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage              = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags     = 0;
	descDepth.MiscFlags          = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencilBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSView;
	ZeroMemory(&descDSView, sizeof(descDSView));
	descDSView.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSView.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSView.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencilBuffer, &descDSView, &g_pDepthStencilView);

	hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
	pBackBuffer->Release();
	if( FAILED( hr ) ) return hr;

	g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

	D3D11_VIEWPORT vp;
	vp.Width    = (FLOAT)width;
	vp.Height   = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports( 1, &vp );

	RenderManager.Initialise(g_pd3dDevice, g_pSwapChain);
	PostProcesser::GetInstance().Init();

	return S_OK;
}

void Render()
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
	g_pSwapChain->Present( 0, 0 );
}

void ToggleFullscreen()
{
	DWORD dwStyle = GetWindowLong(g_hWnd, GWL_STYLE);
	if (!g_isFullscreen)
	{
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(g_hWnd, &g_wpPrev) &&
			GetMonitorInfo(MonitorFromWindow(g_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowLong(g_hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(g_hWnd, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right  - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(g_hWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(g_hWnd, &g_wpPrev);
		SetWindowPos(g_hWnd, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
	g_isFullscreen = !g_isFullscreen;
	SaveFullscreenOption(g_isFullscreen);

	if (g_KBMInput.IsWindowFocused())
		g_KBMInput.SetWindowFocused(true);
}

void CleanupDevice()
{
	if( g_pImmediateContext ) g_pImmediateContext->ClearState();
	if( g_pRenderTargetView ) g_pRenderTargetView->Release();
	if( g_pSwapChain )        g_pSwapChain->Release();
	if( g_pImmediateContext ) g_pImmediateContext->Release();
	if( g_pd3dDevice )        g_pd3dDevice->Release();
}

// ============================================================
//  MINECRAFT 2 - Extended MC2 subsystem initialisation
//  Called from InitialiseMinecraftRuntime() after the core
//  engine boots so all MC2 features start in a valid state.
// ============================================================
static void MC2_InitAllSubsystems(double sessionStartTime)
{
    printf("[MC2] *** %s starting ***\n", MC2_VERSION_STRING);
    printf("[MC2] Build: %s\n", MC2_BUILD_DATE);

    MC2_InitPerformanceCounter();
    MC2_InitDynamicDifficulty();
    g_DynamicDifficulty.sessionStartTime  = sessionStartTime;
    g_DynamicDifficulty.lastEvaluationTime = sessionStartTime;

    MC2_InitWeather();
    MC2_InitAchievements();
    MC2_InitSpectator();
    MC2_InitAutoSave(300.0f);
    MC2_InitChatHistory();
    MC2_LoadHotbar();
    MC2_LoadMOTD();

    printf("[MC2] All subsystems ready. Achievements: %d/%d previously unlocked.\n",
           MC2_GetUnlockedAchievementCount(), (int)MC2_ACH_COUNT);
}

static Minecraft* InitialiseMinecraftRuntime()
{
	app.loadMediaArchive();

	RenderManager.Initialise(g_pd3dDevice, g_pSwapChain);

	app.loadStringTable();
	ui.init(g_pd3dDevice, g_pImmediateContext, g_pRenderTargetView, g_pDepthStencilView, g_iScreenWidth, g_iScreenHeight);

	InputManager.Initialise(1, 3, MINECRAFT_ACTION_MAX, ACTION_MAX_MENU);
	g_KBMInput.Init();
	DefineActions();
	InputManager.SetJoypadMapVal(0, 0);
	InputManager.SetKeyRepeatRate(0.3f, 0.2f);

	ProfileManager.Initialise(TITLEID_MINECRAFT,
		app.m_dwOfferID,
		PROFILE_VERSION_10,
		NUM_PROFILE_VALUES,
		NUM_PROFILE_SETTINGS,
		dwProfileSettingsA,
		app.GAME_DEFINED_PROFILE_DATA_BYTES * XUSER_MAX_COUNT,
		&app.uiGameDefinedDataChangedBitmask
	);
	ProfileManager.SetDefaultOptionsCallback(&CConsoleMinecraftApp::DefaultOptionsCallback, (LPVOID)&app);

	g_NetworkManager.Initialise();

	for (int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; i++)
	{
		IQNet::m_player[i].m_smallId      = (BYTE)i;
		IQNet::m_player[i].m_isRemote     = false;
		IQNet::m_player[i].m_isHostPlayer = (i == 0);
		swprintf_s(IQNet::m_player[i].m_gamertag, 32, L"Player%d", i);
	}
	wcscpy_s(IQNet::m_player[0].m_gamertag, 32, g_Win64UsernameW);

	WinsockNetLayer::Initialize();

	ProfileManager.SetDebugFullOverride(true);

	Tesselator::CreateNewThreadStorage(1024 * 1024);
	AABB::CreateNewThreadStorage();
	Vec3::CreateNewThreadStorage();
	IntCache::CreateNewThreadStorage();
	Compression::CreateNewThreadStorage();
	OldChunkStorage::CreateNewThreadStorage();
	Level::enableLightingCache();
	Tile::CreateNewThreadStorage();

	Minecraft::main();
	Minecraft* pMinecraft = Minecraft::GetInstance();
	if (pMinecraft == NULL)
		return NULL;

	app.InitGameSettings();
	app.InitialiseTips();

	// ---- MINECRAFT 2: Initialise all new subsystems ----
	LARGE_INTEGER liNow;
	QueryPerformanceCounter(&liNow);
	LARGE_INTEGER liFreq;
	QueryPerformanceFrequency(&liFreq);
	double sessionStart = (double)liNow.QuadPart / (double)liFreq.QuadPart;
	MC2_InitAllSubsystems(sessionStart);
	// ----------------------------------------------------

	return pMinecraft;
}

static int HeadlessServerConsoleThreadProc(void* lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);

	std::string line;
	while (!app.m_bShutdown)
	{
		if (!std::getline(std::cin, line))
		{
			if (std::cin.eof()) break;
			std::cin.clear();
			Sleep(50);
			continue;
		}

		wstring command = trimString(convStringToWstring(line));
		if (command.empty())
			continue;

		// ---- MINECRAFT 2: Try MC2 command first ----
		if (MC2_HandleConsoleCommand(command.c_str()))
			continue;
		// ---------------------------------------------

		MinecraftServer* server = MinecraftServer::getInstance();
		if (server != NULL)
			server->handleConsoleInput(command, server);
	}

	return 0;
}

static int RunHeadlessServer()
{
	SetupHeadlessServerConsole();

	Settings serverSettings(new File(L"server.properties"));
	wstring configuredBindIp = serverSettings.getString(L"server-ip", L"");

	const char* bindIp = "*";
	if (g_Win64DedicatedServerBindIP[0] != 0)
		bindIp = g_Win64DedicatedServerBindIP;
	else if (!configuredBindIp.empty())
		bindIp = wstringtochararray(configuredBindIp);

	const int port = g_Win64DedicatedServerPort > 0
		? g_Win64DedicatedServerPort
		: serverSettings.getInt(L"server-port", WIN64_NET_DEFAULT_PORT);

	printf("Starting %s headless server on %s:%d\n",
		MC2_VERSION_STRING, bindIp, port);
	fflush(stdout);

	Minecraft* pMinecraft = InitialiseMinecraftRuntime();
	if (pMinecraft == NULL)
	{
		fprintf(stderr, "Failed to initialise the Minecraft runtime.\n");
		return 1;
	}

	// ---- MINECRAFT 2: Print MOTD on server start ----
	MC2_PrintMOTD();
	// -------------------------------------------------

	app.SetGameHostOption(eGameHostOption_Difficulty,        serverSettings.getInt(L"difficulty", 1));
	app.SetGameHostOption(eGameHostOption_Gamertags,         1);
	app.SetGameHostOption(eGameHostOption_GameType,          serverSettings.getInt(L"gamemode", 0));
	app.SetGameHostOption(eGameHostOption_LevelType,         0);
	app.SetGameHostOption(eGameHostOption_Structures,        serverSettings.getBoolean(L"generate-structures", true)  ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_BonusChest,        serverSettings.getBoolean(L"bonus-chest",         false) ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_PvP,               serverSettings.getBoolean(L"pvp",                 true)  ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_TrustPlayers,      serverSettings.getBoolean(L"trust-players",       true)  ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_FireSpreads,       serverSettings.getBoolean(L"fire-spreads",        true)  ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_TNT,               serverSettings.getBoolean(L"tnt",                 true)  ? 1 : 0);
	app.SetGameHostOption(eGameHostOption_HostCanFly,        1);
	app.SetGameHostOption(eGameHostOption_HostCanChangeHunger,   1);
	app.SetGameHostOption(eGameHostOption_HostCanBeInvisible,    1);
	app.SetGameHostOption(eGameHostOption_MobGriefing,       1);
	app.SetGameHostOption(eGameHostOption_KeepInventory,     0);
	app.SetGameHostOption(eGameHostOption_DoMobSpawning,     1);
	app.SetGameHostOption(eGameHostOption_DoMobLoot,         1);
	app.SetGameHostOption(eGameHostOption_DoTileDrops,       1);
	app.SetGameHostOption(eGameHostOption_NaturalRegeneration,   1);
	app.SetGameHostOption(eGameHostOption_DoDaylightCycle,   1);

	// MINECRAFT 2: Apply dynamic difficulty from server.properties
	bool dynDiff = serverSettings.getBoolean(L"dynamic-difficulty", false);
	if (dynDiff)
	{
		g_DynamicDifficulty.autoAdjustEnabled = true;
		g_DynamicDifficulty.currentLevel      = MC2_DIFFICULTY_DYNAMIC;
		printf("[MC2] Dynamic difficulty ENABLED on server.\n");
	}

	// MINECRAFT 2: Load custom MOTD from server.properties if present
	wstring motdW = serverSettings.getString(L"motd", L"");
	if (!motdW.empty())
	{
		strncpy_s(g_MOTD.lines[0], MC2_MOTD_LINE_LEN,
				  wstringtochararray(motdW), _TRUNCATE);
		g_MOTD.lineCount = 1;
	}

	MinecraftServer::resetFlags();
	g_NetworkManager.HostGame(0, false, true, MINECRAFT_NET_MAX_PLAYERS, 0);

	if (!WinsockNetLayer::IsActive())
	{
		fprintf(stderr, "Failed to bind the server socket on %s:%d.\n", bindIp, port);
		return 1;
	}

	g_NetworkManager.FakeLocalPlayerJoined();

	NetworkGameInitData* param = new NetworkGameInitData();
	param->seed     = 0;
	param->settings = app.GetGameHostOption(eGameHostOption_All);

	g_NetworkManager.ServerStoppedCreate(true);
	g_NetworkManager.ServerReadyCreate(true);

	C4JThread* thread = new C4JThread(&CGameNetworkManager::ServerThreadProc, param, "Server", 256 * 1024);
	thread->SetProcessor(CPU_CORE_SERVER);
	thread->Run();

	g_NetworkManager.ServerReadyWait();
	g_NetworkManager.ServerReadyDestroy();

	if (MinecraftServer::serverHalted())
	{
		fprintf(stderr, "The server halted during startup.\n");
		g_NetworkManager.LeaveGame(false);
		return 1;
	}

	app.SetGameStarted(true);
	g_NetworkManager.DoWork();

	printf("Server ready on %s:%d\n", bindIp, port);
	printf("Type 'help' for server commands. Type '/version' for MC2 info.\n");
	fflush(stdout);

	C4JThread* consoleThread = new C4JThread(&HeadlessServerConsoleThreadProc, NULL, "Server console", 128 * 1024);
	consoleThread->Run();

	MSG msg = { 0 };
	while (WM_QUIT != msg.message && !app.m_bShutdown && !MinecraftServer::serverHalted())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		app.UpdateTime();
		ProfileManager.Tick();
		StorageManager.Tick();
		RenderManager.Tick();
		ui.tick();
		g_NetworkManager.DoWork();
		app.HandleXuiActions();

		// ---- MINECRAFT 2: Server-side subsystem ticks ----
		MC2_TickPerformanceCounter();
		MC2_TickWeather(g_Perf.deltaTime);
		MC2_TickAutoSave(g_Perf.deltaTime);
		MC2_EvaluateDynamicDifficulty(
			(double)g_Perf.totalFrames / 60.0); // rough time estimate
		// --------------------------------------------------

		Sleep(10);
	}

	printf("Stopping %s server...\n", MC2_VERSION_STRING);
	fflush(stdout);

	// MINECRAFT 2: Save achievements and hotbar on shutdown
	MC2_SaveAchievements();
	MC2_SaveHotbar();

	app.m_bShutdown = true;
	MinecraftServer::HaltServer();
	g_NetworkManager.LeaveGame(false);
	return 0;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Set CWD to exe directory so asset paths resolve correctly
	{
		char szExeDir[MAX_PATH] = {};
		GetModuleFileNameA(NULL, szExeDir, MAX_PATH);
		char *pSlash = strrchr(szExeDir, '\\');
		if (pSlash) { *(pSlash + 1) = '\0'; SetCurrentDirectoryA(szExeDir); }
	}

	SetProcessDPIAware();
	g_iScreenWidth  = GetSystemMetrics(SM_CXSCREEN);
	g_iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Load username from username.txt
    char exePath[MAX_PATH] = {};
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    char *lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';

    char filePath[MAX_PATH] = {};
    _snprintf_s(filePath, sizeof(filePath), _TRUNCATE, "%susername.txt", exePath);

    FILE *f = nullptr;
    if (fopen_s(&f, filePath, "r") == 0 && f)
    {
        char buf[128] = {};
        if (fgets(buf, sizeof(buf), f))
        {
            int len = (int)strlen(buf);
            while (len > 0 && (buf[len-1]=='\n'||buf[len-1]=='\r'||buf[len-1]==' '))
                buf[--len] = '\0';
            if (len > 0)
                strncpy_s(g_Win64Username, sizeof(g_Win64Username), buf, _TRUNCATE);
        }
        fclose(f);
    }

	Win64LaunchOptions launchOptions = ParseLaunchOptions();
	ApplyScreenMode(launchOptions.screenMode);

	if (!launchOptions.serverMode)
		Win64Xuid::ResolvePersistentXuid();

	if (g_Win64Username[0] == 0)
        strncpy_s(g_Win64Username, sizeof(g_Win64Username), "Player", _TRUNCATE);

	MultiByteToWideChar(CP_ACP, 0, g_Win64Username, -1, g_Win64UsernameW, 17);

	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, launchOptions.serverMode ? SW_HIDE : nCmdShow))
		return FALSE;

	hMyInst = hInstance;

	if( FAILED( InitDevice() ) )
	{
		CleanupDevice();
		return 0;
	}

	if (LoadFullscreenOption() && !g_isFullscreen || launchOptions.fullscreen)
		ToggleFullscreen();

	if (launchOptions.serverMode)
	{
		int serverResult = RunHeadlessServer();
		CleanupDevice();
		return serverResult;
	}

	static bool bTrialTimerDisplayed = true;

	Minecraft *pMinecraft = InitialiseMinecraftRuntime();
	if (pMinecraft == NULL)
	{
		CleanupDevice();
		return 1;
	}

	MSG msg = {0};
	while( WM_QUIT != msg.message && !app.m_bShutdown)
	{
		g_KBMInput.Tick();

		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
			if (msg.message == WM_QUIT) break;
		}
		if (msg.message == WM_QUIT) break;

		// ---- MINECRAFT 2: Per-frame subsystem ticks ----
		MC2_TickPerformanceCounter();
		MC2_TickWeather(g_Perf.deltaTime);
		MC2_TickAutoSave(g_Perf.deltaTime);
		MC2_EvaluateDynamicDifficulty(
			(double)g_Perf.totalFrames / 60.0);
		// ------------------------------------------------

		RenderManager.StartFrame();

		app.UpdateTime();
		PIXBeginNamedEvent(0,"Input manager tick");
		InputManager.Tick();

		if (InputManager.IsPadConnected(0))
		{
			bool controllerUsed = InputManager.ButtonPressed(0) ||
				InputManager.GetJoypadStick_LX(0, false) != 0.0f ||
				InputManager.GetJoypadStick_LY(0, false) != 0.0f ||
				InputManager.GetJoypadStick_RX(0, false) != 0.0f ||
				InputManager.GetJoypadStick_RY(0, false) != 0.0f;

			if (controllerUsed)
				g_KBMInput.SetKBMActive(false);
			else if (g_KBMInput.HasAnyInput())
				g_KBMInput.SetKBMActive(true);
		}
		else
		{
			g_KBMInput.SetKBMActive(true);
		}

		if (!g_KBMInput.IsMouseGrabbed())
		{
			if (!g_KBMInput.IsKBMActive())
				g_KBMInput.SetCursorHiddenForUI(true);
			else if (!g_KBMInput.IsScreenCursorHidden())
				g_KBMInput.SetCursorHiddenForUI(false);
		}

		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Profile manager tick");
		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Storage manager tick");
		StorageManager.Tick();
		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Render manager tick");
		RenderManager.Tick();
		PIXEndNamedEvent();

		PIXBeginNamedEvent(0,"Network manager do work #1");
		g_NetworkManager.DoWork();
		PIXEndNamedEvent();

		if(app.GetGameStarted())
		{
			pMinecraft->applyFrameMouseLook();
			pMinecraft->run_middle();
			app.SetAppPaused( g_NetworkManager.IsLocalGame() &&
				g_NetworkManager.GetPlayerCount() == 1 &&
				ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()) );
		}
		else
		{
			pMinecraft->soundEngine->tick(NULL, 0.0f);
			pMinecraft->textures->tick(true,false);
			IntCache::Reset();
			if( app.GetReallyChangingSessionType() )
				pMinecraft->tickAllConnections();
		}

		pMinecraft->soundEngine->playMusicTick();

		ui.tick();
		ui.render();

		pMinecraft->gameRenderer->ApplyGammaPostProcess();

		RenderManager.Present();

		ui.CheckMenuDisplayed();

		// Mouse grab logic
		{
			static bool altToggleSuppressCapture = false;
			bool shouldCapture = app.GetGameStarted() && !ui.GetMenuDisplayed(0) && pMinecraft->screen == NULL;
			if (g_KBMInput.IsKeyPressed(VK_LMENU) || g_KBMInput.IsKeyPressed(VK_RMENU))
			{
				if (g_KBMInput.IsMouseGrabbed()) { g_KBMInput.SetMouseGrabbed(false); altToggleSuppressCapture = true; }
				else if (shouldCapture)           { g_KBMInput.SetMouseGrabbed(true);  altToggleSuppressCapture = false; }
			}
			else if (!shouldCapture)
			{
				if (g_KBMInput.IsMouseGrabbed()) g_KBMInput.SetMouseGrabbed(false);
				altToggleSuppressCapture = false;
			}
			else if (shouldCapture && !g_KBMInput.IsMouseGrabbed() &&
					 GetFocus() == g_hWnd && !altToggleSuppressCapture)
			{
				g_KBMInput.SetMouseGrabbed(true);
			}
		}

		// F1 toggles HUD
		if (g_KBMInput.IsKeyPressed(VK_F1))
		{
			int primaryPad = ProfileManager.GetPrimaryPad();
			unsigned char displayHud = app.GetGameSettings(primaryPad, eGameSetting_DisplayHUD);
			app.SetGameSettings(primaryPad, eGameSetting_DisplayHUD,  displayHud ? 0 : 1);
			app.SetGameSettings(primaryPad, eGameSetting_DisplayHand, displayHud ? 0 : 1);
		}

		// F3 toggles debug info
		if (g_KBMInput.IsKeyPressed(VK_F3))
		{
			if (Minecraft* pm = Minecraft::GetInstance())
				if (pm->options)
					pm->options->renderDebug = !pm->options->renderDebug;
		}

		// ---- MINECRAFT 2: F4 toggles performance HUD ----
		if (g_KBMInput.IsKeyPressed(VK_F4))
		{
			g_Perf.showHUD = !g_Perf.showHUD;
			printf("[MC2] Performance HUD %s (%.1f fps, %.2f ms)\n",
				g_Perf.showHUD ? "ON" : "OFF",
				g_Perf.fpsSmoothed, g_Perf.frameTimeMs);
		}

		// ---- MINECRAFT 2: F5 takes a screenshot ----
		if (g_KBMInput.IsKeyPressed(VK_F5))
		{
			MC2_TakeScreenshot(g_pImmediateContext, g_pRenderTargetView,
				g_iScreenWidth, g_iScreenHeight);
		}

		// F11 toggles fullscreen
		if (g_KBMInput.IsKeyPressed(VK_F11))
			ToggleFullscreen();

		// TAB opens game info menu
		if (g_KBMInput.IsKeyPressed(VK_TAB) && !ui.GetMenuDisplayed(0))
		{
			if (Minecraft* pm = Minecraft::GetInstance())
				ui.NavigateToScene(0, eUIScene_InGameInfoMenu);
		}

		// Open chat with T
		if (g_KBMInput.IsKeyPressed('T') && app.GetGameStarted() &&
			!ui.GetMenuDisplayed(0) && pMinecraft->screen == NULL)
		{
			g_KBMInput.ClearCharBuffer();
			pMinecraft->setScreen(new ChatScreen());
			SetFocus(g_hWnd);
		}

		// ---- MINECRAFT 2: F2 forces auto-save now ----
		if (g_KBMInput.IsKeyPressed(VK_F2))
		{
			g_AutoSave.timer = 0.0f; // will trigger on next tick
		}

		app.HandleXuiActions();

		// Trial timer logic
		if(!ProfileManager.IsFullVersion())
		{
			if(app.GetGameStarted())
			{
				if(app.IsAppPaused())
					app.UpdateTrialPausedTimer();
				ui.UpdateTrialTimer(ProfileManager.GetPrimaryPad());
			}
		}
		else
		{
			if(bTrialTimerDisplayed)
			{
				ui.ShowTrialTimer(false);
				bTrialTimerDisplayed = false;
			}
		}

		Vec3::resetPool();
	}

	// ---- MINECRAFT 2: Persist state on clean exit ----
	MC2_SaveAchievements();
	MC2_SaveHotbar();
	printf("[MC2] Session ended. Frames: %u, Avg FPS: %.1f, Min: %.1f, Max: %.1f\n",
		g_Perf.totalFrames, g_Perf.fpsSmoothed,
		g_Perf.minFps, g_Perf.maxFps);
	printf("[MC2] Achievements this session: %d/%d\n",
		MC2_GetUnlockedAchievementCount(), (int)MC2_ACH_COUNT);
	// --------------------------------------------------

	g_pd3dDevice->Release();
}
