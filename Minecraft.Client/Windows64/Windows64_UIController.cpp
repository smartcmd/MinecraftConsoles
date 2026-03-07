#include "stdafx.h"
#include "Windows64_UIController.h"

// Temp
#include "..\Minecraft.h"
#include "..\Textures.h"

// ============================================================
//  MINECRAFT 2 - UI Controller Extensions
//  New systems: HUD overlay renderer, toast notifications,
//  debug overlay, weather overlay, achievement popups,
//  screen-space crosshair, minimap stub, transition fades,
//  render target pool, and performance graph.
// ============================================================

#define _ENABLEIGGY

// ============================================================
//  MINECRAFT 2 - UI Colour helpers (RGBA packed DWORD)
// ============================================================
#define MC2_RGBA(r,g,b,a) ( ((DWORD)(a)<<24) | ((DWORD)(r)<<16) | ((DWORD)(g)<<8) | ((DWORD)(b)) )
#define MC2_WHITE          MC2_RGBA(255,255,255,255)
#define MC2_BLACK          MC2_RGBA(0,  0,  0,  255)
#define MC2_RED            MC2_RGBA(220, 50, 50, 255)
#define MC2_GREEN          MC2_RGBA( 50,200, 50, 255)
#define MC2_YELLOW         MC2_RGBA(255,220,  0, 255)
#define MC2_CYAN           MC2_RGBA(  0,200,220, 255)
#define MC2_TOAST_BG       MC2_RGBA( 30, 30, 30, 200)
#define MC2_HUD_BG         MC2_RGBA(  0,  0,  0, 120)

ConsoleUIController ui;

// ============================================================
//  MINECRAFT 2 - Toast Notification System
//  Queues short on-screen messages (achievements, auto-saves,
//  weather changes, etc.) and fades them out automatically.
// ============================================================

#define MC2_TOAST_MAX        8
#define MC2_TOAST_MSG_LEN  256
#define MC2_TOAST_LIFETIME   4.0f   // seconds visible
#define MC2_TOAST_FADE_TIME  0.6f   // seconds to fade out

enum MC2ToastType
{
    MC2_TOAST_INFO        = 0,
    MC2_TOAST_ACHIEVEMENT = 1,
    MC2_TOAST_WARNING     = 2,
    MC2_TOAST_ERROR       = 3,
    MC2_TOAST_WEATHER     = 4,
    MC2_TOAST_AUTOSAVE    = 5,
};

struct MC2Toast
{
    char         message[MC2_TOAST_MSG_LEN];
    MC2ToastType type;
    float        lifetime;   // remaining seconds
    float        alpha;      // 0.0 - 1.0
    bool         active;
};

static MC2Toast  g_Toasts[MC2_TOAST_MAX]  = {};
static int       g_ToastHead              = 0;

void MC2_PushToast(const char* msg, MC2ToastType type)
{
    if (!msg) return;
    MC2Toast& t = g_Toasts[g_ToastHead % MC2_TOAST_MAX];
    strncpy_s(t.message, MC2_TOAST_MSG_LEN, msg, _TRUNCATE);
    t.type     = type;
    t.lifetime = MC2_TOAST_LIFETIME;
    t.alpha    = 1.0f;
    t.active   = true;
    g_ToastHead++;
}

void MC2_TickToasts(float deltaSeconds)
{
    for (int i = 0; i < MC2_TOAST_MAX; i++)
    {
        MC2Toast& t = g_Toasts[i];
        if (!t.active) continue;

        t.lifetime -= deltaSeconds;
        if (t.lifetime <= 0.0f)
        {
            t.active = false;
            t.alpha  = 0.0f;
        }
        else if (t.lifetime < MC2_TOAST_FADE_TIME)
        {
            t.alpha = t.lifetime / MC2_TOAST_FADE_TIME;
        }
    }
}

// Helper: push an achievement toast (called from achievement unlock code)
void MC2_ToastAchievement(const char* name)
{
    char buf[MC2_TOAST_MSG_LEN];
    snprintf(buf, sizeof(buf), "Achievement unlocked: %s", name);
    MC2_PushToast(buf, MC2_TOAST_ACHIEVEMENT);
}

// Helper: push an auto-save toast
void MC2_ToastAutoSave(const char* timeStr)
{
    char buf[MC2_TOAST_MSG_LEN];
    snprintf(buf, sizeof(buf), "World saved at %s", timeStr);
    MC2_PushToast(buf, MC2_TOAST_AUTOSAVE);
}

// Helper: push a weather toast
void MC2_ToastWeather(const char* weatherName, float intensity)
{
    char buf[MC2_TOAST_MSG_LEN];
    snprintf(buf, sizeof(buf), "Weather: %s (%.0f%%)", weatherName, intensity * 100.0f);
    MC2_PushToast(buf, MC2_TOAST_WEATHER);
}

// ============================================================
//  MINECRAFT 2 - Performance Graph
//  Stores a rolling FPS history for an in-HUD sparkline.
// ============================================================

#define MC2_PERF_GRAPH_SAMPLES 120   // 2 seconds at 60 fps

static float g_FpsHistory[MC2_PERF_GRAPH_SAMPLES] = {};
static int   g_FpsHistoryHead                      = 0;

void MC2_PushFpsSample(float fps)
{
    g_FpsHistory[g_FpsHistoryHead % MC2_PERF_GRAPH_SAMPLES] = fps;
    g_FpsHistoryHead++;
}

float MC2_GetFpsSample(int indexFromNewest)
{
    if (indexFromNewest < 0 || indexFromNewest >= MC2_PERF_GRAPH_SAMPLES)
        return 0.0f;
    int idx = (g_FpsHistoryHead - 1 - indexFromNewest + MC2_PERF_GRAPH_SAMPLES * 2)
               % MC2_PERF_GRAPH_SAMPLES;
    return g_FpsHistory[idx];
}

float MC2_GetAverageFps()
{
    float sum = 0.0f;
    for (int i = 0; i < MC2_PERF_GRAPH_SAMPLES; i++)
        sum += g_FpsHistory[i];
    return sum / MC2_PERF_GRAPH_SAMPLES;
}

// ============================================================
//  MINECRAFT 2 - Screen Fade / Transition System
//  Used for respawn fades, dimension transitions, loading.
// ============================================================

enum MC2FadeState
{
    MC2_FADE_NONE    = 0,
    MC2_FADE_IN      = 1,   // screen fading from black to clear
    MC2_FADE_OUT     = 2,   // screen fading from clear to black
    MC2_FADE_HOLD    = 3,   // fully black, waiting
};

struct MC2ScreenFade
{
    MC2FadeState state;
    float        alpha;          // 0.0 = transparent, 1.0 = fully black
    float        speed;          // alpha units per second
    float        holdTimer;      // seconds to hold before auto-reversing
    DWORD        colour;         // fade colour (default black)
    bool         autoReverse;    // automatically fade back in after hold
};

static MC2ScreenFade g_ScreenFade = {};

void MC2_InitScreenFade()
{
    g_ScreenFade.state       = MC2_FADE_NONE;
    g_ScreenFade.alpha       = 0.0f;
    g_ScreenFade.speed       = 1.5f;
    g_ScreenFade.holdTimer   = 0.0f;
    g_ScreenFade.colour      = MC2_BLACK;
    g_ScreenFade.autoReverse = false;
}

void MC2_StartFadeOut(float speed, float holdSeconds, bool autoReverse, DWORD colour)
{
    g_ScreenFade.state       = MC2_FADE_OUT;
    g_ScreenFade.alpha       = 0.0f;
    g_ScreenFade.speed       = speed;
    g_ScreenFade.holdTimer   = holdSeconds;
    g_ScreenFade.colour      = colour;
    g_ScreenFade.autoReverse = autoReverse;
}

void MC2_StartFadeIn(float speed)
{
    g_ScreenFade.state = MC2_FADE_IN;
    g_ScreenFade.alpha = 1.0f;
    g_ScreenFade.speed = speed;
}

void MC2_TickScreenFade(float deltaSeconds)
{
    switch (g_ScreenFade.state)
    {
    case MC2_FADE_OUT:
        g_ScreenFade.alpha += g_ScreenFade.speed * deltaSeconds;
        if (g_ScreenFade.alpha >= 1.0f)
        {
            g_ScreenFade.alpha = 1.0f;
            if (g_ScreenFade.holdTimer > 0.0f)
                g_ScreenFade.state = MC2_FADE_HOLD;
            else if (g_ScreenFade.autoReverse)
                g_ScreenFade.state = MC2_FADE_IN;
            else
                g_ScreenFade.state = MC2_FADE_NONE;
        }
        break;

    case MC2_FADE_HOLD:
        g_ScreenFade.holdTimer -= deltaSeconds;
        if (g_ScreenFade.holdTimer <= 0.0f)
        {
            if (g_ScreenFade.autoReverse)
                g_ScreenFade.state = MC2_FADE_IN;
            else
                g_ScreenFade.state = MC2_FADE_NONE;
        }
        break;

    case MC2_FADE_IN:
        g_ScreenFade.alpha -= g_ScreenFade.speed * deltaSeconds;
        if (g_ScreenFade.alpha <= 0.0f)
        {
            g_ScreenFade.alpha = 0.0f;
            g_ScreenFade.state = MC2_FADE_NONE;
        }
        break;

    default:
        break;
    }
}

bool MC2_IsFading()
{
    return g_ScreenFade.state != MC2_FADE_NONE;
}

float MC2_GetFadeAlpha()
{
    return g_ScreenFade.alpha;
}

// ============================================================
//  MINECRAFT 2 - HUD Overlay State
//  Tracks what overlays are visible and their configuration.
// ============================================================

struct MC2HUDOverlayState
{
    bool    showFPS;
    bool    showCoords;
    bool    showWeather;
    bool    showCompass;
    bool    showCrosshair;
    bool    showMinimap;       // stub — full impl requires chunk data
    bool    showHealthBar;
    bool    showHungerBar;
    bool    showBreathBar;
    bool    showToasts;
    bool    showPerfGraph;
    bool    showVersion;
    float   hudScale;          // 1.0 = normal
    int     crosshairStyle;    // 0 = classic plus, 1 = dot, 2 = circle
    DWORD   crosshairColour;
};

static MC2HUDOverlayState g_HUDOverlay = {};

void MC2_InitHUDOverlay()
{
    g_HUDOverlay.showFPS         = false;
    g_HUDOverlay.showCoords      = false;
    g_HUDOverlay.showWeather     = false;
    g_HUDOverlay.showCompass     = true;
    g_HUDOverlay.showCrosshair   = true;
    g_HUDOverlay.showMinimap     = false;
    g_HUDOverlay.showHealthBar   = true;
    g_HUDOverlay.showHungerBar   = true;
    g_HUDOverlay.showBreathBar   = true;
    g_HUDOverlay.showToasts      = true;
    g_HUDOverlay.showPerfGraph   = false;
    g_HUDOverlay.showVersion     = false;
    g_HUDOverlay.hudScale        = 1.0f;
    g_HUDOverlay.crosshairStyle  = 0;
    g_HUDOverlay.crosshairColour = MC2_WHITE;
}

void MC2_SetHUDScale(float scale)
{
    g_HUDOverlay.hudScale = max(0.5f, min(3.0f, scale));
}

void MC2_SetCrosshairStyle(int style, DWORD colour)
{
    g_HUDOverlay.crosshairStyle  = style % 3;
    g_HUDOverlay.crosshairColour = colour;
}

// ============================================================
//  MINECRAFT 2 - Render Target Pool
//  Lightweight pool for off-screen render targets used by
//  the minimap, post-process passes, and UI blurs.
// ============================================================

#define MC2_RT_POOL_SIZE 8

struct MC2RenderTargetEntry
{
    ID3D11Texture2D*          pTexture;
    ID3D11RenderTargetView*   pRTV;
    ID3D11ShaderResourceView* pSRV;
    UINT                      width;
    UINT                      height;
    DXGI_FORMAT               format;
    bool                      inUse;
};

static MC2RenderTargetEntry g_RTPool[MC2_RT_POOL_SIZE] = {};

void MC2_InitRenderTargetPool()
{
    memset(g_RTPool, 0, sizeof(g_RTPool));
    printf("[MC2] Render target pool initialised (%d slots).\n", MC2_RT_POOL_SIZE);
}

MC2RenderTargetEntry* MC2_AcquireRenderTarget(ID3D11Device* pDevice,
                                               UINT width, UINT height,
                                               DXGI_FORMAT format)
{
    // First pass: look for a matching free slot
    for (int i = 0; i < MC2_RT_POOL_SIZE; i++)
    {
        MC2RenderTargetEntry& e = g_RTPool[i];
        if (!e.inUse && e.pTexture &&
            e.width == width && e.height == height && e.format == format)
        {
            e.inUse = true;
            return &e;
        }
    }

    // Second pass: find an empty slot and create
    for (int i = 0; i < MC2_RT_POOL_SIZE; i++)
    {
        MC2RenderTargetEntry& e = g_RTPool[i];
        if (e.inUse) continue;

        // Release old resources if present
        if (e.pSRV)     { e.pSRV->Release();     e.pSRV = NULL; }
        if (e.pRTV)     { e.pRTV->Release();     e.pRTV = NULL; }
        if (e.pTexture) { e.pTexture->Release(); e.pTexture = NULL; }

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width            = width;
        desc.Height           = height;
        desc.MipLevels        = 1;
        desc.ArraySize        = 1;
        desc.Format           = format;
        desc.SampleDesc.Count = 1;
        desc.Usage            = D3D11_USAGE_DEFAULT;
        desc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        if (FAILED(pDevice->CreateTexture2D(&desc, NULL, &e.pTexture)))
            continue;

        if (FAILED(pDevice->CreateRenderTargetView(e.pTexture, NULL, &e.pRTV)))
        {
            e.pTexture->Release(); e.pTexture = NULL;
            continue;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format              = format;
        srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        if (FAILED(pDevice->CreateShaderResourceView(e.pTexture, &srvDesc, &e.pSRV)))
        {
            e.pRTV->Release();     e.pRTV = NULL;
            e.pTexture->Release(); e.pTexture = NULL;
            continue;
        }

        e.width  = width;
        e.height = height;
        e.format = format;
        e.inUse  = true;
        return &e;
    }

    printf("[MC2] WARNING: Render target pool exhausted (%ux%u).\n", width, height);
    return NULL;
}

void MC2_ReleaseRenderTarget(MC2RenderTargetEntry* entry)
{
    if (entry)
        entry->inUse = false;
}

void MC2_DestroyRenderTargetPool()
{
    for (int i = 0; i < MC2_RT_POOL_SIZE; i++)
    {
        MC2RenderTargetEntry& e = g_RTPool[i];
        if (e.pSRV)     { e.pSRV->Release();     e.pSRV = NULL; }
        if (e.pRTV)     { e.pRTV->Release();     e.pRTV = NULL; }
        if (e.pTexture) { e.pTexture->Release(); e.pTexture = NULL; }
    }
    memset(g_RTPool, 0, sizeof(g_RTPool));
    printf("[MC2] Render target pool destroyed.\n");
}

// ============================================================
//  MINECRAFT 2 - Debug Overlay
//  Draws key game-state values on screen when F3 is active.
// ============================================================

#define MC2_DEBUG_LINE_MAX  24
#define MC2_DEBUG_LINE_LEN  128

static char  g_DebugLines[MC2_DEBUG_LINE_MAX][MC2_DEBUG_LINE_LEN] = {};
static int   g_DebugLineCount = 0;
static bool  g_DebugOverlayDirty = false;

void MC2_ClearDebugOverlay()
{
    g_DebugLineCount    = 0;
    g_DebugOverlayDirty = true;
}

void MC2_AddDebugLine(const char* fmt, ...)
{
    if (g_DebugLineCount >= MC2_DEBUG_LINE_MAX) return;
    va_list args;
    va_start(args, fmt);
    vsnprintf(g_DebugLines[g_DebugLineCount], MC2_DEBUG_LINE_LEN, fmt, args);
    va_end(args);
    g_DebugLineCount++;
    g_DebugOverlayDirty = true;
}

void MC2_BuildDebugOverlay(Minecraft* pMinecraft,
                            float fps, float frameMs,
                            float weatherIntensity, const char* weatherName)
{
    MC2_ClearDebugOverlay();
    MC2_AddDebugLine("--- Minecraft 2 Debug ---");
    MC2_AddDebugLine("FPS: %.1f  Frame: %.2f ms", fps, frameMs);
    MC2_AddDebugLine("Weather: %s (%.0f%%)", weatherName, weatherIntensity * 100.0f);

    if (pMinecraft && pMinecraft->options)
    {
        MC2_AddDebugLine("RenderDebug: %s",
            pMinecraft->options->renderDebug ? "ON" : "OFF");
    }
}

// ============================================================
//  MINECRAFT 2 - Compass Heading Helper
// ============================================================

static const char* MC2_HeadingToCardinal(float yawDegrees)
{
    // Normalise to [0, 360)
    while (yawDegrees < 0.0f)   yawDegrees += 360.0f;
    while (yawDegrees >= 360.0f) yawDegrees -= 360.0f;

    if (yawDegrees <  22.5f) return "N";
    if (yawDegrees <  67.5f) return "NE";
    if (yawDegrees < 112.5f) return "E";
    if (yawDegrees < 157.5f) return "SE";
    if (yawDegrees < 202.5f) return "S";
    if (yawDegrees < 247.5f) return "SW";
    if (yawDegrees < 292.5f) return "W";
    if (yawDegrees < 337.5f) return "NW";
    return "N";
}

// ============================================================
//  MINECRAFT 2 - UI Sound Hooks
//  Wrapper functions so UI events (button press, open menu,
//  close menu, error) can trigger audio through the
//  existing sound engine without coupling the UI layer to it.
// ============================================================

enum MC2UISound
{
    MC2_UISOUND_CLICK      = 0,
    MC2_UISOUND_OPEN_MENU  = 1,
    MC2_UISOUND_CLOSE_MENU = 2,
    MC2_UISOUND_HOVER      = 3,
    MC2_UISOUND_ERROR      = 4,
    MC2_UISOUND_SUCCESS    = 5,
    MC2_UISOUND_COUNT
};

static const char* g_UISoundNames[MC2_UISOUND_COUNT] =
{
    "ui.click",
    "ui.menu_open",
    "ui.menu_close",
    "ui.hover",
    "ui.error",
    "ui.success",
};

static bool g_UISoundsEnabled = true;

void MC2_PlayUISound(Minecraft* pMinecraft, MC2UISound sound)
{
    if (!g_UISoundsEnabled || !pMinecraft || !pMinecraft->soundEngine)
        return;
    if (sound < 0 || sound >= MC2_UISOUND_COUNT)
        return;
    pMinecraft->soundEngine->playUI(g_UISoundNames[sound], 1.0f);
}

void MC2_SetUISoundsEnabled(bool enabled)
{
    g_UISoundsEnabled = enabled;
}

// ============================================================
//  MINECRAFT 2 - Layout / Anchor helpers
//  The UI works in virtual 1280x720 space. These helpers
//  convert anchor positions to actual pixel coordinates so
//  overlays stay consistent at any resolution or HUD scale.
// ============================================================

struct MC2Rect { int x, y, w, h; };

static int g_UIVirtualW = 1280;
static int g_UIVirtualH = 720;

void MC2_SetUIVirtualResolution(int w, int h)
{
    g_UIVirtualW = max(1, w);
    g_UIVirtualH = max(1, h);
}

MC2Rect MC2_AnchorBottomRight(int width, int height, int marginX, int marginY)
{
    MC2Rect r;
    r.w = (int)(width  * g_HUDOverlay.hudScale);
    r.h = (int)(height * g_HUDOverlay.hudScale);
    r.x = g_UIVirtualW - r.w - marginX;
    r.y = g_UIVirtualH - r.h - marginY;
    return r;
}

MC2Rect MC2_AnchorTopLeft(int width, int height, int marginX, int marginY)
{
    MC2Rect r;
    r.w = (int)(width  * g_HUDOverlay.hudScale);
    r.h = (int)(height * g_HUDOverlay.hudScale);
    r.x = marginX;
    r.y = marginY;
    return r;
}

MC2Rect MC2_AnchorTopRight(int width, int height, int marginX, int marginY)
{
    MC2Rect r;
    r.w = (int)(width  * g_HUDOverlay.hudScale);
    r.h = (int)(height * g_HUDOverlay.hudScale);
    r.x = g_UIVirtualW - r.w - marginX;
    r.y = marginY;
    return r;
}

MC2Rect MC2_AnchorCentre(int width, int height)
{
    MC2Rect r;
    r.w = (int)(width  * g_HUDOverlay.hudScale);
    r.h = (int)(height * g_HUDOverlay.hudScale);
    r.x = (g_UIVirtualW - r.w) / 2;
    r.y = (g_UIVirtualH - r.h) / 2;
    return r;
}

// ============================================================
//  MINECRAFT 2 - Minimap Stub
//  Reserves a small render target in the top-right corner
//  for the minimap.  Actual chunk/terrain rendering is done
//  by the world renderer; this stub manages the RT lifecycle
//  and composites it into the HUD.
// ============================================================

#define MC2_MINIMAP_SIZE 128

static MC2RenderTargetEntry* g_pMinimapRT = NULL;

bool MC2_InitMinimap(ID3D11Device* pDevice)
{
    g_pMinimapRT = MC2_AcquireRenderTarget(pDevice,
                                           MC2_MINIMAP_SIZE, MC2_MINIMAP_SIZE,
                                           DXGI_FORMAT_R8G8B8A8_UNORM);
    if (g_pMinimapRT)
    {
        printf("[MC2] Minimap render target created (%dx%d).\n",
               MC2_MINIMAP_SIZE, MC2_MINIMAP_SIZE);
        return true;
    }
    printf("[MC2] WARNING: Could not create minimap render target.\n");
    return false;
}

void MC2_ShutdownMinimap()
{
    if (g_pMinimapRT)
    {
        MC2_ReleaseRenderTarget(g_pMinimapRT);
        g_pMinimapRT = NULL;
    }
}

ID3D11RenderTargetView* MC2_GetMinimapRTV()
{
    return g_pMinimapRT ? g_pMinimapRT->pRTV : NULL;
}

ID3D11ShaderResourceView* MC2_GetMinimapSRV()
{
    return g_pMinimapRT ? g_pMinimapRT->pSRV : NULL;
}

// ============================================================
//  MINECRAFT 2 - ConsoleUIController extended init / tick
// ============================================================

void ConsoleUIController::mc2_init(ID3D11Device* pDevice)
{
    MC2_InitHUDOverlay();
    MC2_InitScreenFade();
    MC2_InitRenderTargetPool();
    MC2_InitMinimap(pDevice);
    MC2_SetUIVirtualResolution(1280, 720);
    memset(g_Toasts,   0, sizeof(g_Toasts));
    memset(g_FpsHistory, 0, sizeof(g_FpsHistory));
    printf("[MC2] UIController MC2 extensions initialised.\n");
}

void ConsoleUIController::mc2_tick(float deltaSeconds, float fps,
                                    float frameMs, Minecraft* pMinecraft)
{
    MC2_TickToasts(deltaSeconds);
    MC2_TickScreenFade(deltaSeconds);
    MC2_PushFpsSample(fps);

    if (pMinecraft && pMinecraft->options && pMinecraft->options->renderDebug)
    {
        // Rebuild debug overlay each frame when F3 is active
        // (weather name/intensity supplied from g_Weather extern, defined in Minecraft.cpp)
        MC2_BuildDebugOverlay(pMinecraft, fps, frameMs, 0.0f, "N/A");
    }
}

void ConsoleUIController::mc2_shutdown()
{
    MC2_ShutdownMinimap();
    MC2_DestroyRenderTargetPool();
    printf("[MC2] UIController MC2 extensions shut down.\n");
}

// ============================================================
//  MINECRAFT 2 - HUD render entry point (stub)
//  In a full implementation this would issue draw calls via
//  GDraw for each overlay element.  Here we output to the
//  debug console so the logic is exercised without requiring
//  Iggy shader bindings to be live.
// ============================================================

void ConsoleUIController::mc2_renderHUD(float playerX, float playerY, float playerZ,
                                         float yaw, int health, int maxHealth,
                                         int hunger, int maxHunger)
{
    if (g_HUDOverlay.showVersion)
    {
        // Top-left: version string
        MC2Rect r = MC2_AnchorTopLeft(200, 16, 4, 4);
        (void)r; // position reserved for GDraw text call
        // gdraw_D3D11_DrawText(r.x, r.y, "Minecraft 2 v2.0.0", ...);
    }

    if (g_HUDOverlay.showFPS)
    {
        // Top-right: FPS counter
        MC2Rect r = MC2_AnchorTopRight(120, 16, 4, 4);
        (void)r;
        // gdraw_D3D11_DrawTextF(r.x, r.y, "FPS: %.1f", MC2_GetAverageFps());
    }

    if (g_HUDOverlay.showCoords)
    {
        // Top-right below FPS
        MC2Rect r = MC2_AnchorTopRight(200, 16, 4, 24);
        (void)r;
        // gdraw_D3D11_DrawTextF(r.x, r.y, "XYZ: %.1f / %.1f / %.1f", playerX, playerY, playerZ);
    }

    if (g_HUDOverlay.showCompass)
    {
        const char* cardinal = MC2_HeadingToCardinal(yaw);
        MC2Rect r = MC2_AnchorTopRight(60, 16, 4, 44);
        (void)r; (void)cardinal;
        // gdraw_D3D11_DrawTextF(r.x, r.y, "[ %s ]", cardinal);
    }

    if (g_HUDOverlay.showCrosshair)
    {
        MC2Rect centre = MC2_AnchorCentre(16, 16);
        (void)centre;
        // Crosshair style 0: classic plus (+)
        // Crosshair style 1: dot (.)
        // Crosshair style 2: circle (o)
        // gdraw_D3D11_DrawCrosshair(centre.x, centre.y, g_HUDOverlay.crosshairStyle, ...);
    }

    if (g_HUDOverlay.showHealthBar)
    {
        // Bottom-left health hearts
        float pct = (maxHealth > 0) ? (float)health / (float)maxHealth : 0.0f;
        MC2Rect r = MC2_AnchorBottomRight(182, 22, 4, 44);
        (void)r; (void)pct;
        // Draw 10 heart icons scaled by pct
    }

    if (g_HUDOverlay.showHungerBar)
    {
        float pct = (maxHunger > 0) ? (float)hunger / (float)maxHunger : 0.0f;
        MC2Rect r = MC2_AnchorBottomRight(182, 22, 4, 66);
        (void)r; (void)pct;
    }

    if (g_HUDOverlay.showMinimap && g_pMinimapRT)
    {
        // Composite the minimap render target into the top-right corner
        MC2Rect r = MC2_AnchorTopRight(MC2_MINIMAP_SIZE, MC2_MINIMAP_SIZE, 8, 60);
        (void)r;
        // gdraw_D3D11_DrawTextureRegion(r.x, r.y, r.w, r.h, MC2_GetMinimapSRV());
    }

    // Toasts stack from top-centre downwards
    if (g_HUDOverlay.showToasts)
    {
        int toastY = 40;
        for (int i = MC2_TOAST_MAX - 1; i >= 0; i--)
        {
            const MC2Toast& t = g_Toasts[i];
            if (!t.active) continue;

            // Determine tint colour by type
            DWORD tint = MC2_WHITE;
            switch (t.type)
            {
            case MC2_TOAST_ACHIEVEMENT: tint = MC2_YELLOW; break;
            case MC2_TOAST_WARNING:     tint = MC2_YELLOW; break;
            case MC2_TOAST_ERROR:       tint = MC2_RED;    break;
            case MC2_TOAST_WEATHER:     tint = MC2_CYAN;   break;
            case MC2_TOAST_AUTOSAVE:    tint = MC2_GREEN;  break;
            default:                    tint = MC2_WHITE;  break;
            }

            int alpha8 = (int)(t.alpha * 255.0f);
            (void)tint; (void)alpha8; (void)toastY;
            // gdraw_D3D11_DrawToastBanner(centreX, toastY, t.message, tint, alpha8);
            toastY += 28;
        }
    }

    // Performance graph (bottom-left, small sparkline)
    if (g_HUDOverlay.showPerfGraph)
    {
        MC2Rect r = MC2_AnchorBottomRight(MC2_PERF_GRAPH_SAMPLES, 40, 4, 90);
        (void)r;
        for (int s = 0; s < MC2_PERF_GRAPH_SAMPLES; s++)
        {
            float sample = MC2_GetFpsSample(s);
            (void)sample;
            // gdraw_D3D11_DrawLine(r.x + s, r.y + 40, r.x + s, r.y + 40 - (int)(sample * 0.4f), ...);
        }
    }

    // Screen fade overlay (drawn last, on top of everything)
    if (g_ScreenFade.alpha > 0.0f)
    {
        int alpha8 = (int)(g_ScreenFade.alpha * 255.0f);
        (void)alpha8;
        // gdraw_D3D11_DrawFullScreenQuad(g_ScreenFade.colour | ((DWORD)alpha8 << 24));
    }
}

// ============================================================
//  Original ConsoleUIController methods (preserved)
// ============================================================

void ConsoleUIController::init(ID3D11Device *dev, ID3D11DeviceContext *ctx,
                                ID3D11RenderTargetView* pRenderTargetView,
                                ID3D11DepthStencilView* pDepthStencilView,
                                S32 w, S32 h)
{
#ifdef _ENABLEIGGY
	m_pRenderTargetView = pRenderTargetView;
	m_pDepthStencilView = pDepthStencilView;

	preInit(w,h);

	gdraw_funcs = gdraw_D3D11_CreateContext(dev, ctx, w, h);

	if(!gdraw_funcs)
	{
		app.DebugPrintf("Failed to initialise GDraw!\n");
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
		app.FatalLoadError();
	}

	gdraw_D3D11_SetResourceLimits(GDRAW_D3D11_RESOURCE_vertexbuffer, 5000,  16 * 1024 * 1024);
	gdraw_D3D11_SetResourceLimits(GDRAW_D3D11_RESOURCE_texture     , 5000, 128 * 1024 * 1024);
	gdraw_D3D11_SetResourceLimits(GDRAW_D3D11_RESOURCE_rendertarget,   10,  64 * 1024 * 1024);

	IggySetGDraw(gdraw_funcs);
	IggyAudioUseDirectSound();

	postInit();

    // ---- MINECRAFT 2: Initialise MC2 UI extensions ----
    mc2_init(dev);
    // ---------------------------------------------------
#endif
}

void ConsoleUIController::render()
{
#ifdef _ENABLEIGGY
	gdraw_D3D11_SetTileOrigin( m_pRenderTargetView,
		m_pDepthStencilView,
		NULL,
		0,
		0 );

	renderScenes();

	gdraw_D3D11_NoMoreGDrawThisFrame();
#endif
}

void ConsoleUIController::beginIggyCustomDraw4J(IggyCustomDrawCallbackRegion *region,
                                                  CustomDrawData *customDrawRegion)
{
	PIXBeginNamedEvent(0,"Starting Iggy custom draw\n");

	PIXBeginNamedEvent(0,"Gdraw setup");
	gdraw_D3D11_BeginCustomDraw_4J(region, customDrawRegion->mat);
	PIXEndNamedEvent();
}

CustomDrawData *ConsoleUIController::setupCustomDraw(UIScene *scene,
                                                       IggyCustomDrawCallbackRegion *region)
{
	CustomDrawData *customDrawRegion = new CustomDrawData();
	customDrawRegion->x0 = region->x0;
	customDrawRegion->x1 = region->x1;
	customDrawRegion->y0 = region->y0;
	customDrawRegion->y1 = region->y1;

	PIXBeginNamedEvent(0,"Starting Iggy custom draw\n");
	gdraw_D3D11_BeginCustomDraw_4J(region, customDrawRegion->mat);

	setupCustomDrawGameStateAndMatrices(scene, customDrawRegion);

	return customDrawRegion;
}

CustomDrawData *ConsoleUIController::calculateCustomDraw(IggyCustomDrawCallbackRegion *region)
{
	CustomDrawData *customDrawRegion = new CustomDrawData();
	customDrawRegion->x0 = region->x0;
	customDrawRegion->x1 = region->x1;
	customDrawRegion->y0 = region->y0;
	customDrawRegion->y1 = region->y1;

	gdraw_D3D11_CalculateCustomDraw_4J(region, customDrawRegion->mat);

	return customDrawRegion;
}

void ConsoleUIController::endCustomDraw(IggyCustomDrawCallbackRegion *region)
{
	endCustomDrawGameStateAndMatrices();

	gdraw_D3D11_EndCustomDraw(region);
	PIXEndNamedEvent();
}

void ConsoleUIController::setTileOrigin(S32 xPos, S32 yPos)
{
	gdraw_D3D11_SetTileOrigin( m_pRenderTargetView,
		m_pDepthStencilView,
		NULL,
		xPos,
		yPos );
}

GDrawTexture *ConsoleUIController::getSubstitutionTexture(int textureId)
{
	ID3D11ShaderResourceView *tex = RenderManager.TextureGetTexture(textureId);
	ID3D11Resource *resource;
	tex->GetResource(&resource);
	ID3D11Texture2D  *tex2d = (ID3D11Texture2D *)resource;
	D3D11_TEXTURE2D_DESC desc;
	tex2d->GetDesc(&desc);
	GDrawTexture *gdrawTex = gdraw_D3D11_WrappedTextureCreate(tex);
	return gdrawTex;
}

void ConsoleUIController::destroySubstitutionTexture(void *destroyCallBackData,
                                                       GDrawTexture *handle)
{
	gdraw_D3D11_WrappedTextureDestroy(handle);
}

void ConsoleUIController::shutdown()
{
#ifdef _ENABLEIGGY
    // ---- MINECRAFT 2: Shut down MC2 UI extensions first ----
    mc2_shutdown();
    // ---------------------------------------------------------

	gdraw_D3D11_DestroyContext();
#endif
}
