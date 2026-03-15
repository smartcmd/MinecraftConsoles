#include "stdafx.h"
#include "UIController.h"
#include "UI.h"
#include "UIScene.h"
#include "UIControl_Slider.h"
#include "UIControl_TexturePackList.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\LocalPlayer.h"
#include "..\..\DLCTexturePack.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\Minecraft.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.boss.enderdragon.h"
#include "..\..\EnderDragonRenderer.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "UIFontData.h"
#include "UISplitScreenHelpers.h"
#ifdef _WINDOWS64
#include "..\..\Windows64\KeyboardMouseInput.h"
#endif
#ifdef __PSVITA__
#include <message_dialog.h>
#endif

// 4J Stu - Enable this to override the Iggy Allocator
//#define ENABLE_IGGY_ALLOCATOR
//#define EXCLUDE_IGGY_ALLOCATIONS_FROM_HEAP_INSPECTOR

//#define ENABLE_IGGY_EXPLORER
#ifdef ENABLE_IGGY_EXPLORER
#include "Windows64\Iggy\include\iggyexpruntime.h"
#endif

//#define ENABLE_IGGY_PERFMON
#ifdef ENABLE_IGGY_PERFMON

#define PM_ORIGIN_X 24
#define PM_ORIGIN_Y 34

#ifdef __ORBIS__
#include "Orbis\Iggy\include\iggyperfmon.h"
#include "Orbis\Iggy\include\iggyperfmon_orbis.h"
#elif defined _DURANGO
#include "Durango\Iggy\include\iggyperfmon.h"
#elif defined __PS3__
#include "PS3\Iggy\include\iggyperfmon.h"
#include "PS3\Iggy\include\iggyperfmon_ps3.h"
#elif defined __PSVITA__
#include "PSVita\Iggy\include\iggyperfmon.h"
#include "PSVita\Iggy\include\iggyperfmon_psp2.h"
#elif defined __WINDOWS64
#include "Windows64\Iggy\include\iggyperfmon.h"
#endif

#endif

CRITICAL_SECTION UIController::ms_reloadSkinCS;
bool UIController::ms_bReloadSkinCSInitialised = false;

DWORD UIController::m_dwTrialTimerLimitSecs=DYNAMIC_CONFIG_DEFAULT_TRIAL_TIME;

// GetViewportRect and Fit16x9 are now in UISplitScreenHelpers.h

#ifdef _WINDOWS64
static UIControl_Slider *FindSliderById(UIScene *pScene, int sliderId)
{
	vector<UIControl *> *controls = pScene->GetControls();
	if (!controls) return nullptr;
	for (size_t i = 0; i < controls->size(); ++i)
	{
		UIControl *ctrl = (*controls)[i];
		if (ctrl && ctrl->getControlType() == UIControl::eSlider && ctrl->getId() == sliderId)
			return static_cast<UIControl_Slider *>(ctrl);
	}
	return nullptr;
}
#endif

static void RADLINK WarningCallback(void *user_callback_data, Iggy *player, IggyResult code, const char *message)
{
	//enum IggyResult{    IGGY_RESULT_SUCCESS = 0,    IGGY_RESULT_Warning_None = 0,
	//   IGGY_RESULT_Warning_Misc = 100,    IGGY_RESULT_Warning_GDraw = 101,
	//   IGGY_RESULT_Warning_ProgramFlow = 102,
	//   IGGY_RESULT_Warning_Actionscript = 103,
	//   IGGY_RESULT_Warning_Graphics = 104,    IGGY_RESULT_Warning_Font = 105,
	//   IGGY_RESULT_Warning_Timeline = 106,    IGGY_RESULT_Warning_Library = 107,
	//   IGGY_RESULT_Warning_CannotSustainFrameRate = 201,
	//   IGGY_RESULT_Warning_ThrewException = 202,
	//   IGGY_RESULT_Error_Threshhold = 400,    IGGY_RESULT_Error_Misc = 400,
	//   IGGY_RESULT_Error_GDraw = 401,    IGGY_RESULT_Error_ProgramFlow = 402,
	//   IGGY_RESULT_Error_Actionscript = 403,    IGGY_RESULT_Error_Graphics = 404,
	//   IGGY_RESULT_Error_Font = 405,    IGGY_RESULT_Error_Create = 406,
	//   IGGY_RESULT_Error_Library = 407,    IGGY_RESULT_Error_ValuePath = 408,
	//   IGGY_RESULT_Error_Audio = 409,    IGGY_RESULT_Error_Internal = 499,
	//   IGGY_RESULT_Error_InvalidIggy = 501,
	//   IGGY_RESULT_Error_InvalidArgument = 502,
	//   IGGY_RESULT_Error_InvalidEntity = 503,
	//   IGGY_RESULT_Error_UndefinedEntity = 504,
	//   IGGY_RESULT_Error_OutOfMemory = 1001,};

	switch(code)
	{
	case IGGY_RESULT_Warning_CannotSustainFrameRate:
		// Ignore warning
		break;
	default:
		/* Normally, we'd want to issue this warning to some kind of
		logging system or error reporting system, but since this is a
		tutorial app, we just use Win32's default error stream.  Since
		ActionScript 3 exceptions are routed through this warning
		callback, it's definitely a good idea to make sure these
		warnings get printed somewhere that's easy for you to read and
		use for debugging, otherwise debugging errors in the
		ActionScript 3 code in your Flash content will be very
		difficult! */
		app.DebugPrintf(app.USER_SR, message);
		app.DebugPrintf(app.USER_SR, "\n");
		break;
	};
}


/* Flash provides a way for ActionScript 3 code to print debug output
using a function called "trace".  It's very useful for debugging
Flash programs, so ideally, when using Iggy, we'd like to see any
trace output alongside our own debugging output.  To facilitate
this, Iggy allows us to install a callback that will be called
any time ActionScript code calls trace. */
static void RADLINK TraceCallback(void *user_callback_data, Iggy *player, char const *utf8_string, S32 length_in_bytes)
{
	app.DebugPrintf(app.USER_UI, (char *)utf8_string);
}

#ifdef ENABLE_IGGY_PERFMON
static void *RADLINK perf_malloc(void *handle, U32 size)
{
   return malloc(size);
}

static void RADLINK perf_free(void *handle, void *ptr)
{
   return free(ptr);
}
#endif

#ifdef EXCLUDE_IGGY_ALLOCATIONS_FROM_HEAP_INSPECTOR
extern "C" void *__real_malloc(size_t t);
extern "C" void __real_free(void *t);
#endif

int64_t UIController::iggyAllocCount = 0;
static unordered_map<void *,size_t> allocations;
static void * RADLINK AllocateFunction ( void * alloc_callback_user_data , size_t size_requested , size_t * size_returned )
{
	UIController *controller = static_cast<UIController *>(alloc_callback_user_data);
	EnterCriticalSection(&controller->m_Allocatorlock);
#ifdef EXCLUDE_IGGY_ALLOCATIONS_FROM_HEAP_INSPECTOR
	void *alloc = __real_malloc(size_requested);
#else
	void *alloc = malloc(size_requested);
#endif
	*size_returned = size_requested;
	UIController::iggyAllocCount += size_requested;
	allocations[alloc] = size_requested;
	app.DebugPrintf(app.USER_SR, "Allocating %d, new total: %d\n", size_requested, UIController::iggyAllocCount);
	LeaveCriticalSection(&controller->m_Allocatorlock);
	return alloc;
}

static void RADLINK DeallocateFunction ( void * alloc_callback_user_data , void * ptr )
{
	UIController *controller = static_cast<UIController *>(alloc_callback_user_data);
	EnterCriticalSection(&controller->m_Allocatorlock);
	size_t size = allocations[ptr];
	UIController::iggyAllocCount -= size;
	allocations.erase(ptr);
	app.DebugPrintf(app.USER_SR, "Freeing %d, new total %d\n", size, UIController::iggyAllocCount);
#ifdef EXCLUDE_IGGY_ALLOCATIONS_FROM_HEAP_INSPECTOR
	__real_free(ptr);
#else
	free(ptr);
#endif
	LeaveCriticalSection(&controller->m_Allocatorlock);
}

UIController::UIController()
{
	m_uiDebugConsole = nullptr;
	m_reloadSkinThread = nullptr;

	m_navigateToHomeOnReload = false;

	m_bCleanupOnReload = false;
	m_mcTTFFont = nullptr;
	m_moj7 = nullptr;
	m_moj11 = nullptr;

	// 4J-JEV: It's important that these remain the same, unless updateCurrentLanguage is going to be called.
	m_eCurrentFont = m_eTargetFont = eFont_NotLoaded;

#ifdef ENABLE_IGGY_ALLOCATOR
	InitializeCriticalSection(&m_Allocatorlock);
#endif

	// 4J Stu - This is a bit of a hack until we change the Minecraft initialisation to store the proper screen size for other platforms
#if defined _WINDOWS64 || defined _DURANGO || defined __ORBIS__
	m_fScreenWidth = 1920.0f;
	m_fScreenHeight = 1080.0f;
	m_bScreenWidthSetup = false;
#else
	m_fScreenWidth = 1280.0f;
	m_fScreenHeight = 720.0f;
	m_bScreenWidthSetup = false;
#endif

	for(unsigned int i = 0; i < eLibrary_Count; ++i)
	{
		m_iggyLibraries[i] = IGGY_INVALID_LIBRARY;
	}

	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		m_bMenuDisplayed[i] = false;
		m_iCountDown[i]=0;
		m_bMenuToBeClosed[i]=false;

		for(unsigned int key = 0; key <= ACTION_MAX_MENU; ++key)
		{
			m_actionRepeatTimer[i][key] = 0;
		}
	}

	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		m_bCloseAllScenes[i] = false;
	}

	m_iPressStartQuadrantsMask = 0;

	m_currentRenderViewport = C4JRender::VIEWPORT_TYPE_FULLSCREEN;
	m_bCustomRenderPosition = false;
	m_winUserIndex = 0;
	m_mouseDraggingSliderScene = eUIScene_COUNT;
	m_mouseDraggingSliderId = -1;
	m_mouseClickConsumedByScene = false;
	m_bMouseHoverHorizontalList = false;
	m_lastHoverMouseX = -1;
	m_lastHoverMouseY = -1;
	m_accumulatedTicks = 0;
	m_lastUiSfx = 0;

	InitializeCriticalSection(&m_navigationLock);
	InitializeCriticalSection(&m_registeredCallbackScenesCS);
	//m_bSysUIShowing=false;
	m_bSystemUIShowing=false;
#ifdef __PSVITA__
	m_bTouchscreenPressed=false;
#endif

	if(!ms_bReloadSkinCSInitialised)
	{
		// MGH - added to prevent crash loading Iggy movies while the skins were being reloaded
		InitializeCriticalSection(&ms_reloadSkinCS);
		ms_bReloadSkinCSInitialised = true;
	}
}

void UIController::SetSysUIShowing(bool bVal)
{
	if(bVal) app.DebugPrintf("System UI showing\n");
	else app.DebugPrintf("System UI stopped showing\n");
	m_bSystemUIShowing=bVal;
}

void UIController::SetSystemUIShowing(LPVOID lpParam,bool bVal)
{
	UIController *pClass=static_cast<UIController *>(lpParam);
	pClass->SetSysUIShowing(bVal);
}

// SETUP
void UIController::preInit(S32 width, S32 height)
{
	m_fScreenWidth = width;
	m_fScreenHeight = height;
	m_bScreenWidthSetup = true;

#ifdef ENABLE_IGGY_ALLOCATOR
	IggyAllocator allocator;
	allocator.user_callback_data = this;
	allocator.mem_alloc = &AllocateFunction;
	allocator.mem_free = &DeallocateFunction;
	IggyInit(&allocator);
#else
	IggyInit(0);
#endif

	IggySetWarningCallback(WarningCallback, 0);
	IggySetTraceCallbackUTF8(TraceCallback, 0);

	setFontCachingCalculationBuffer(-1);
}

void UIController::postInit()
{
	// set up a custom rendering callback
	IggySetCustomDrawCallback(&UIController::CustomDrawCallback, this);
	IggySetAS3ExternalFunctionCallbackUTF16 ( &UIController::ExternalFunctionCallback, this );
	IggySetTextureSubstitutionCallbacks ( &UIController::TextureSubstitutionCreateCallback , &UIController::TextureSubstitutionDestroyCallback, this );

	SetupFont();
	//
	loadSkins();

	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		m_groups[i] = new UIGroup(static_cast<EUIGroup>(i),i-1);
	}


#ifdef ENABLE_IGGY_EXPLORER
	iggy_explorer = IggyExpCreate("127.0.0.1", 9190, malloc(IGGYEXP_MIN_STORAGE), IGGYEXP_MIN_STORAGE);
	if ( iggy_explorer == nullptr )
	{
		// not normally an error, just an error for this demo!
		app.DebugPrintf( "Couldn't connect to Iggy Explorer, did you run it first?" );
	}
	else
	{
		IggyUseExplorer( m_groups[1]->getHUD()->getMovie(), iggy_explorer);
	}
#endif

#ifdef ENABLE_IGGY_PERFMON
	m_iggyPerfmonEnabled = false;
	iggy_perfmon = IggyPerfmonCreate(perf_malloc, perf_free, nullptr);
	IggyInstallPerfmon(iggy_perfmon);
#endif

	NavigateToScene(0, eUIScene_Intro);
}


UIController::EFont UIController::getFontForLanguage(int language)
	{
	switch(language)
	{
	case XC_LANGUAGE_JAPANESE:	return eFont_Japanese;
#ifdef _DURANGO
	case XC_LANGUAGE_SCHINESE:	return eFont_SimpChinese;
#endif
	case XC_LANGUAGE_TCHINESE:	return eFont_TradChinese;
	case XC_LANGUAGE_KOREAN:	return eFont_Korean;
	default:					return eFont_Bitmap;
	}
}

UITTFFont *UIController::createFont(EFont fontLanguage)
	{
	switch(fontLanguage)
	{
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	case eFont_Japanese:		return new UITTFFont("Mojangles_TTF_jaJP", "Common/Media/font/JPN/DF-DotDotGothic16.ttf", 0x203B); // JPN
	// case eFont_SimpChinese:	Simplified Chinese is unsupported.
	case eFont_TradChinese:		return new UITTFFont("Mojangles_TTF_cnTD", "Common/Media/font/CHT/DFTT_R5.TTC", 0x203B); // CHT
	case eFont_Korean:			return new UITTFFont("Mojangles_TTF_koKR", "Common/Media/font/KOR/candadite2.ttf", 0x203B); // KOR
#else
	case eFont_Japanese:		return new UITTFFont("Mojangles_TTF_jaJP", "Common/Media/font/JPN/DFGMaruGothic-Md.ttf", 0x2022); // JPN
#ifdef _DURANGO
	case eFont_SimpChinese:		return new UITTFFont("Mojangled_TTF_cnCN", "Common/Media/font/CHS/MSYH.ttf", 0x2022); // CHS
#endif
	case eFont_TradChinese:		return new UITTFFont("Mojangles_TTF_cnTD", "Common/Media/font/CHT/DFHeiMedium-B5.ttf", 0x2022); // CHT
	case eFont_Korean:			return new UITTFFont("Mojangles_TTF_koKR", "Common/Media/font/KOR/BOKMSD.ttf", 0x2022); // KOR
#endif
	// 4J-JEV, Cyrillic characters have been added to this font now, (4/July/14)
	// XC_LANGUAGE_RUSSIAN and XC_LANGUAGE_GREEK:
	default:					return nullptr;
	}
}

void UIController::SetupFont()
{
	// 4J-JEV: Language hasn't changed or is already changing.
	if ( (m_eCurrentFont != m_eTargetFont) || !UIString::setCurrentLanguage() ) return;

	DWORD nextLanguage = UIString::getCurrentLanguage();
	m_eTargetFont = getFontForLanguage(nextLanguage);

	// flag a language change to reload the string tables in the DLC
	app.m_dlcManager.LanguageChanged();

	app.loadStringTable(); // Switch to use new string table,

	if (m_eTargetFont == m_eCurrentFont)
	{
		// 4J-JEV: If we're ingame, reload the font to update all the text.
		if (app.GetGameStarted()) app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ReloadFont);
		return;
	}

	if (m_eCurrentFont != eFont_NotLoaded)	app.DebugPrintf("[UIController] Font switch required for language transition to %i.\n", nextLanguage);
	else									app.DebugPrintf("[UIController] Initialising font for language %i.\n", nextLanguage);

	if (m_mcTTFFont != nullptr)
	{
		delete m_mcTTFFont;
		m_mcTTFFont = nullptr;
	}

	if(m_eTargetFont == eFont_Bitmap)
	{
		// these may have been set up by a previous language being chosen
		if (m_moj7 == nullptr)		m_moj7  = new UIBitmapFont(SFontData::Mojangles_7);
		if (m_moj11 == nullptr)	m_moj11 = new UIBitmapFont(SFontData::Mojangles_11);

		// 4J-JEV: Ensure we redirect to them correctly, even if the objects were previously initialised.
		m_moj7->registerFont();
		m_moj11->registerFont();
	}
	else if (m_eTargetFont != eFont_NotLoaded)
	{
		m_mcTTFFont = createFont(m_eTargetFont);

		app.DebugPrintf("[Iggy] Set font indirect to '%hs'.\n", m_mcTTFFont->getFontName().c_str());
		IggyFontSetIndirectUTF8( "Mojangles7",	-1, IGGY_FONTFLAG_all, m_mcTTFFont->getFontName().c_str(), -1, IGGY_FONTFLAG_none );
		IggyFontSetIndirectUTF8( "Mojangles11",	-1, IGGY_FONTFLAG_all, m_mcTTFFont->getFontName().c_str(), -1, IGGY_FONTFLAG_none );
	}
	else
	{
		assert(false);
	}

	// Reload ui to set new font.
	if (m_eCurrentFont != eFont_NotLoaded)
	{
		app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ReloadFont);
	}
	else
	{
		updateCurrentFont();
	}
}

bool UIController::PendingFontChange()
{
	return getFontForLanguage( XGetLanguage() ) != m_eCurrentFont;
}

void UIController::setCleanupOnReload()
{
	m_bCleanupOnReload = true;
}

void UIController::updateCurrentFont()
{
	m_eCurrentFont = m_eTargetFont;
}

bool UIController::UsingBitmapFont()
{
	return m_eCurrentFont == eFont_Bitmap;
}

// TICKING
void UIController::tick()
{
	SetupFont(); // If necessary, change font.

	if ( (m_navigateToHomeOnReload || m_bCleanupOnReload) && !ui.IsReloadingSkin() )
	{
		ui.CleanUpSkinReload();

		if (m_navigateToHomeOnReload || !g_NetworkManager.IsInSession())
		{
			ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_MainMenu);
		}
		else
		{
			ui.CloseAllPlayersScenes();
		}

		updateCurrentFont();

		m_navigateToHomeOnReload = false;
		m_bCleanupOnReload		 = false;
	}

	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		if(m_bCloseAllScenes[i])
		{
			m_groups[i]->closeAllScenes();
			m_groups[i]->getTooltips()->SetTooltips(-1);
			m_bCloseAllScenes[i] = false;
		}
	}

	if(m_accumulatedTicks == 0) tickInput();
	m_accumulatedTicks = 0;

	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		m_groups[i]->tick();

		// TODO: May wish to skip ticking other groups here
	}

	// Clear out the cached movie file data
	int64_t currentTime = System::currentTimeMillis();
    for (auto it = m_cachedMovieData.begin(); it != m_cachedMovieData.end();)
    {
		if(it->second.m_expiry < currentTime)
		{
			delete [] it->second.m_ba.data;
			it = m_cachedMovieData.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void UIController::loadSkins()
{
#ifdef __PS3__
	m_iggyLibraries[eLibrary_Platform] = loadSkin(L"skinPS3.swf", L"platformskin.swf");

	m_iggyLibraries[eLibrary_GraphicsDefault] = loadSkin(L"skinGraphics.swf", L"skinGraphics.swf");
	m_iggyLibraries[eLibrary_GraphicsHUD] = loadSkin(L"skinGraphicsHud.swf", L"skinGraphicsHud.swf");
	m_iggyLibraries[eLibrary_GraphicsInGame] = loadSkin(L"skinGraphicsInGame.swf", L"skinGraphicsInGame.swf");
	m_iggyLibraries[eLibrary_GraphicsTooltips] = loadSkin(L"skinGraphicsTooltips.swf", L"skinGraphicsTooltips.swf");
	m_iggyLibraries[eLibrary_GraphicsLabels] = loadSkin(L"skinGraphicsLabels.swf", L"skinGraphicsLabels.swf");
	m_iggyLibraries[eLibrary_Labels] = loadSkin(L"skinLabels.swf", L"skinLabels.swf");
	m_iggyLibraries[eLibrary_InGame] = loadSkin(L"skinInGame.swf", L"skinInGame.swf");
	m_iggyLibraries[eLibrary_HUD] = loadSkin(L"skinHud.swf", L"skinHud.swf");
	m_iggyLibraries[eLibrary_Tooltips] = loadSkin(L"skinTooltips.swf", L"skinTooltips.swf");
	m_iggyLibraries[eLibrary_Default] = loadSkin(L"skin.swf", L"skin.swf");

#elif defined __PSVITA__
	m_iggyLibraries[eLibrary_Platform] = loadSkin(L"skinVita.swf", L"platformskin.swf");

	m_iggyLibraries[eLibrary_GraphicsDefault] = loadSkin(L"skinGraphics.swf", L"skinGraphics.swf");
	m_iggyLibraries[eLibrary_GraphicsHUD] = loadSkin(L"skinGraphicsHud.swf", L"skinGraphicsHud.swf");
	m_iggyLibraries[eLibrary_GraphicsInGame] = loadSkin(L"skinGraphicsInGame.swf", L"skinGraphicsInGame.swf");
	m_iggyLibraries[eLibrary_GraphicsTooltips] = loadSkin(L"skinGraphicsTooltips.swf", L"skinGraphicsTooltips.swf");
	m_iggyLibraries[eLibrary_GraphicsLabels] = loadSkin(L"skinGraphicsLabels.swf", L"skinGraphicsLabels.swf");
	m_iggyLibraries[eLibrary_Labels] = loadSkin(L"skinLabels.swf", L"skinLabels.swf");
	m_iggyLibraries[eLibrary_InGame] = loadSkin(L"skinInGame.swf", L"skinInGame.swf");
	m_iggyLibraries[eLibrary_HUD] = loadSkin(L"skinHud.swf", L"skinHud.swf");
	m_iggyLibraries[eLibrary_Tooltips] = loadSkin(L"skinTooltips.swf", L"skinTooltips.swf");
	m_iggyLibraries[eLibrary_Default] = loadSkin(L"skin.swf", L"skin.swf");

#elif defined _WINDOWS64
	// HD platform skin — required by skinHD*.swf (1080p scene SWFs)
	m_iggyLibraries[eLibrary_Platform] = loadSkin(L"skinHDWin.swf", L"platformskinHD.swf");
	// Non-HD platform skin — required by skin*.swf (720p/480p scene SWFs)
	m_iggyLibraries[eLibraryFallback_Platform] = loadSkin(L"skinWin.swf", L"platformskin.swf");

	// Non-HD skin set (720p/480p scenes import these)
	m_iggyLibraries[eLibraryFallback_GraphicsDefault] = loadSkin(L"skinGraphics.swf", L"skinGraphics.swf");
	m_iggyLibraries[eLibraryFallback_GraphicsHUD] = loadSkin(L"skinGraphicsHud.swf", L"skinGraphicsHud.swf");
	m_iggyLibraries[eLibraryFallback_GraphicsInGame] = loadSkin(L"skinGraphicsInGame.swf", L"skinGraphicsInGame.swf");
	m_iggyLibraries[eLibraryFallback_GraphicsTooltips] = loadSkin(L"skinGraphicsTooltips.swf", L"skinGraphicsTooltips.swf");
	m_iggyLibraries[eLibraryFallback_GraphicsLabels] = loadSkin(L"skinGraphicsLabels.swf", L"skinGraphicsLabels.swf");
	m_iggyLibraries[eLibraryFallback_Labels] = loadSkin(L"skinLabels.swf", L"skinLabels.swf");
	m_iggyLibraries[eLibraryFallback_InGame] = loadSkin(L"skinInGame.swf", L"skinInGame.swf");
	m_iggyLibraries[eLibraryFallback_HUD] = loadSkin(L"skinHud.swf", L"skinHud.swf");
	m_iggyLibraries[eLibraryFallback_Tooltips] = loadSkin(L"skinTooltips.swf", L"skinTooltips.swf");
	m_iggyLibraries[eLibraryFallback_Default] = loadSkin(L"skin.swf", L"skin.swf");

	// HD skin set (1080p scenes import these)
	m_iggyLibraries[eLibrary_GraphicsDefault] = loadSkin(L"skinHDGraphics.swf", L"skinHDGraphics.swf");
	m_iggyLibraries[eLibrary_GraphicsHUD] = loadSkin(L"skinHDGraphicsHud.swf", L"skinHDGraphicsHud.swf");
	m_iggyLibraries[eLibrary_GraphicsInGame] = loadSkin(L"skinHDGraphicsInGame.swf", L"skinHDGraphicsInGame.swf");
	m_iggyLibraries[eLibrary_GraphicsTooltips] = loadSkin(L"skinHDGraphicsTooltips.swf", L"skinHDGraphicsTooltips.swf");
	m_iggyLibraries[eLibrary_GraphicsLabels] = loadSkin(L"skinHDGraphicsLabels.swf", L"skinHDGraphicsLabels.swf");
	m_iggyLibraries[eLibrary_Labels] = loadSkin(L"skinHDLabels.swf", L"skinHDLabels.swf");
	m_iggyLibraries[eLibrary_InGame] = loadSkin(L"skinHDInGame.swf", L"skinHDInGame.swf");
	m_iggyLibraries[eLibrary_HUD] = loadSkin(L"skinHDHud.swf", L"skinHDHud.swf");
	m_iggyLibraries[eLibrary_Tooltips] = loadSkin(L"skinHDTooltips.swf", L"skinHDTooltips.swf");
	m_iggyLibraries[eLibrary_Default] = loadSkin(L"skinHD.swf", L"skinHD.swf");

#elif defined _DURANGO
	m_iggyLibraries[eLibrary_Platform] = loadSkin(L"skinHDDurango.swf", L"platformskinHD.swf");

	m_iggyLibraries[eLibrary_GraphicsDefault] = loadSkin(L"skinHDGraphics.swf", L"skinHDGraphics.swf");
	m_iggyLibraries[eLibrary_GraphicsHUD] = loadSkin(L"skinHDGraphicsHud.swf", L"skinHDGraphicsHud.swf");
	m_iggyLibraries[eLibrary_GraphicsInGame] = loadSkin(L"skinHDGraphicsInGame.swf", L"skinHDGraphicsInGame.swf");
	m_iggyLibraries[eLibrary_GraphicsTooltips] = loadSkin(L"skinHDGraphicsTooltips.swf", L"skinHDGraphicsTooltips.swf");
	m_iggyLibraries[eLibrary_GraphicsLabels] = loadSkin(L"skinHDGraphicsLabels.swf", L"skinHDGraphicsLabels.swf");
	m_iggyLibraries[eLibrary_Labels] = loadSkin(L"skinHDLabels.swf", L"skinHDLabels.swf");
	m_iggyLibraries[eLibrary_InGame] = loadSkin(L"skinHDInGame.swf", L"skinHDInGame.swf");
	m_iggyLibraries[eLibrary_HUD] = loadSkin(L"skinHDHud.swf", L"skinHDHud.swf");
	m_iggyLibraries[eLibrary_Tooltips] = loadSkin(L"skinHDTooltips.swf", L"skinHDTooltips.swf");
	m_iggyLibraries[eLibrary_Default] = loadSkin(L"skinHD.swf", L"skinHD.swf");

#elif defined __ORBIS__
	m_iggyLibraries[eLibrary_Platform] = loadSkin(L"skinHDOrbis.swf", L"platformskinHD.swf");

	m_iggyLibraries[eLibrary_GraphicsDefault] = loadSkin(L"skinHDGraphics.swf", L"skinHDGraphics.swf");
	m_iggyLibraries[eLibrary_GraphicsHUD] = loadSkin(L"skinHDGraphicsHud.swf", L"skinHDGraphicsHud.swf");
	m_iggyLibraries[eLibrary_GraphicsInGame] = loadSkin(L"skinHDGraphicsInGame.swf", L"skinHDGraphicsInGame.swf");
	m_iggyLibraries[eLibrary_GraphicsTooltips] = loadSkin(L"skinHDGraphicsTooltips.swf", L"skinHDGraphicsTooltips.swf");
	m_iggyLibraries[eLibrary_GraphicsLabels] = loadSkin(L"skinHDGraphicsLabels.swf", L"skinHDGraphicsLabels.swf");
	m_iggyLibraries[eLibrary_Labels] = loadSkin(L"skinHDLabels.swf", L"skinHDLabels.swf");
	m_iggyLibraries[eLibrary_InGame] = loadSkin(L"skinHDInGame.swf", L"skinHDInGame.swf");
	m_iggyLibraries[eLibrary_HUD] = loadSkin(L"skinHDHud.swf", L"skinHDHud.swf");
	m_iggyLibraries[eLibrary_Tooltips] = loadSkin(L"skinHDTooltips.swf", L"skinHDTooltips.swf");
	m_iggyLibraries[eLibrary_Default] = loadSkin(L"skinHD.swf", L"skinHD.swf");
#endif
}

IggyLibrary UIController::loadSkin(const wstring &skinPath, const wstring &skinName)
{
	IggyLibrary lib = IGGY_INVALID_LIBRARY;
	// 4J Stu - We need to load the platformskin before the normal skin, as the normal skin requires some elements from the platform skin
	if(!skinPath.empty() && app.hasArchiveFile(skinPath))
	{
		byteArray baFile = app.getArchiveFile(skinPath);
		lib = IggyLibraryCreateFromMemoryUTF16( (IggyUTF16 *)skinName.c_str() , (void *)baFile.data, baFile.length, nullptr );

		delete[] baFile.data;
#ifdef _DEBUG
		IggyMemoryUseInfo memoryInfo;
		rrbool res;
		int iteration = 0;
		int64_t totalStatic = 0;
		while(res = IggyDebugGetMemoryUseInfo (nullptr,
                                               lib ,
                                               "" ,
                                               0 ,
                                               iteration ,
                                               &memoryInfo ))
		{
			totalStatic += memoryInfo.static_allocation_bytes;
			app.DebugPrintf(app.USER_SR, "%ls - %.*s, static: %dB, dynamic: %dB\n", skinPath.c_str(), memoryInfo.subcategory_stringlen, memoryInfo.subcategory, memoryInfo.static_allocation_bytes, memoryInfo.dynamic_allocation_bytes);
			++iteration;
		}

		app.DebugPrintf(app.USER_SR, "%ls - Total static: %dB (%dKB)\n", skinPath.c_str(), totalStatic, totalStatic/1024);
#endif
	}
	return lib;
}

void UIController::ReloadSkin()
{
	// Destroy all scene swf
	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		//m_bCloseAllScenes[i] = true;
		m_groups[i]->DestroyAll();
	}

	// Unload the current libraries
	// Some libraries reference others, so we destroy in reverse order
	for(int i = eLibrary_Count - 1; i >= 0; --i)
	{
		if(m_iggyLibraries[i] != IGGY_INVALID_LIBRARY) IggyLibraryDestroy(m_iggyLibraries[i]);
		m_iggyLibraries[i] = IGGY_INVALID_LIBRARY;
	}

#ifdef _WINDOWS64
	// 4J Stu - Don't load on a thread on windows. I haven't investigated this in detail, so a quick fix
	reloadSkinThreadProc(this);
#else

	m_reloadSkinThread = new C4JThread(reloadSkinThreadProc, (void*)this, "Reload skin thread");
	m_reloadSkinThread->SetProcessor(CPU_CORE_UI_SCENE);

	// Navigate to the timer scene so that we can display something while the loading is happening
	ui.NavigateToScene(0,eUIScene_Timer,(void *)1,eUILayer_Tooltips,eUIGroup_Fullscreen);
	//m_reloadSkinThread->Run();

	//// Load new skin
	//loadSkins();

	//// Reload all scene swf
	//for(int i = eUIGroup_Player1; i <= eUIGroup_Player4; ++i)
	//{
	//	m_groups[i]->ReloadAll();
	//}

	//// Always reload the fullscreen group
	//m_groups[eUIGroup_Fullscreen]->ReloadAll();
#endif
}

void UIController::StartReloadSkinThread()
{
	if(m_reloadSkinThread) m_reloadSkinThread->Run();
}

int UIController::reloadSkinThreadProc(void* lpParam)
{
	EnterCriticalSection(&ms_reloadSkinCS);		// MGH - added to prevent crash loading Iggy movies while the skins were being reloaded
	UIController *controller = static_cast<UIController *>(lpParam);
	// Load new skin
	controller->loadSkins();

	// Reload all scene swf
	for(int i = eUIGroup_Player1; i < eUIGroup_COUNT; ++i)
	{
		controller->m_groups[i]->ReloadAll();
	}

	// Always reload the fullscreen group
	controller->m_groups[eUIGroup_Fullscreen]->ReloadAll();

	// 4J Stu - Don't do this on windows, as we never navigated forwards to start with
#ifndef _WINDOW64
	controller->NavigateBack(0, false, eUIScene_COUNT, eUILayer_Tooltips);
#endif
	LeaveCriticalSection(&ms_reloadSkinCS);

	return 0;
}

bool UIController::IsReloadingSkin()
{
	return m_reloadSkinThread && (!m_reloadSkinThread->hasStarted() || m_reloadSkinThread->isRunning());
}

bool UIController::IsExpectingOrReloadingSkin()
{
	return Minecraft::GetInstance()->skins->getSelected()->isLoadingData() || Minecraft::GetInstance()->skins->needsUIUpdate() || IsReloadingSkin() || PendingFontChange();
}

void UIController::CleanUpSkinReload()
{
	delete m_reloadSkinThread;
	m_reloadSkinThread = nullptr;

	if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
	{
		if(!Minecraft::GetInstance()->skins->getSelected()->hasAudio())
		{
#ifdef _DURANGO
			DWORD result = StorageManager.UnmountInstalledDLC(L"TPACK");
#else
			DWORD result = StorageManager.UnmountInstalledDLC("TPACK");
#endif
		}
	}

	for(auto queuedData : m_queuedMessageBoxData)
	{
		ui.NavigateToScene(queuedData->iPad, eUIScene_MessageBox, &queuedData->info, queuedData->layer, eUIGroup_Fullscreen);
		delete queuedData->info.uiOptionA;
		delete queuedData;
	}
	m_queuedMessageBoxData.clear();
}

byteArray UIController::getMovieData(const wstring &filename)
{
	// Cache everything we load in the current tick
	int64_t targetTime = System::currentTimeMillis() + (1000LL * 60);
    auto it = m_cachedMovieData.find(filename);
    if(it == m_cachedMovieData.end() )
	{
		byteArray baFile = app.getArchiveFile(filename);
		CachedMovieData cmd;
		cmd.m_ba = baFile;
		cmd.m_expiry = targetTime;
		m_cachedMovieData[filename] = cmd;
		return baFile;
	}
	else
	{
		it->second.m_expiry = targetTime;
		return it->second.m_ba;
	}
}

// INPUT
void UIController::tickInput()
{
	// If system/commerce UI up, don't handle input
	//if(!m_bSysUIShowing && !m_bSystemUIShowing)
	if(!m_bSystemUIShowing)
	{
#ifdef ENABLE_IGGY_PERFMON
		if (m_iggyPerfmonEnabled)
		{
			if(InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(), ACTION_MENU_STICK_PRESS)) m_iggyPerfmonEnabled = !m_iggyPerfmonEnabled;
		}
		else
#endif
		{
#ifdef _WINDOWS64
            m_mouseClickConsumedByScene = false;
            if (!g_KBMInput.IsMouseGrabbed() && g_KBMInput.IsKBMActive())
            {
                UIScene *pScene = nullptr;

                // Search by layer priority across all groups (layer-first).
                // Tooltip layer is skipped because it holds non-interactive
                // overlays (button hints, timer) that should never capture mouse.
                // Old group-first order found those tooltips on eUIGroup_Fullscreen
                // before reaching in-game menus on eUIGroup_Player1.
                static const EUILayer mouseLayers[] = {
#ifndef _CONTENT_PACKAGE
                    eUILayer_Debug,
#endif
					eUILayer_Error,
					eUILayer_Alert,
					eUILayer_Popup,
					eUILayer_Fullscreen,
					eUILayer_Scene,
				};
				// Only check the fullscreen group and the primary (KBM) player's group.
			// Other splitscreen players use controllers — mouse must not affect them.
			const int mouseGroups[] = { static_cast<int>(eUIGroup_Fullscreen), ProfileManager.GetPrimaryPad() + 1 };
			for (int l = 0; l < _countof(mouseLayers) && !pScene; ++l)
			{
				for (int g = 0; g < _countof(mouseGroups) && !pScene; ++g)
				{
					pScene = m_groups[mouseGroups[g]]->GetTopScene(mouseLayers[l]);
				}
			}
				if (pScene && pScene->getMovie())
                {
                    int rawMouseX = g_KBMInput.GetMouseX();
                    int rawMouseY = g_KBMInput.GetMouseY();
                    F32 mouseX = static_cast<F32>(rawMouseX);
                    F32 mouseY = static_cast<F32>(rawMouseY);

                    extern HWND g_hWnd;
                    if (g_hWnd)
                    {
                        RECT rc;
                        GetClientRect(g_hWnd, &rc);
                        int winW = rc.right - rc.left;
                        int winH = rc.bottom - rc.top;
                        if (winW > 0 && winH > 0)
                        {
                            mouseX = mouseX * (m_fScreenWidth / static_cast<F32>(winW));
                            mouseY = mouseY * (m_fScreenHeight / static_cast<F32>(winH));
                        }
                    }

					// Only update hover focus when the mouse has actually moved,
					// so that mouse-wheel scrolling can change list selection
					// without the hover immediately snapping focus back.
					bool mouseMoved = (rawMouseX != m_lastHoverMouseX || rawMouseY != m_lastHoverMouseY);
					m_lastHoverMouseX = rawMouseX;
					m_lastHoverMouseY = rawMouseY;

					// Convert mouse window-pixel coords to Flash/SWF authoring coords.
					// In split-screen the scene is rendered at a tile-origin offset
					// and at a smaller display size, so we must:
					//   1. Map window pixels -> UIController screen space
					//   2. Subtract the viewport tile origin
					//   3. Scale from display dimensions to SWF authoring dimensions
					F32 sceneMouseX = static_cast<F32>(rawMouseX);
					F32 sceneMouseY = static_cast<F32>(rawMouseY);
					{
						extern HWND g_hWnd;
						RECT rc;
						if (g_hWnd && GetClientRect(g_hWnd, &rc))
						{
							int winW = rc.right - rc.left;
							int winH = rc.bottom - rc.top;
							if (winW > 0 && winH > 0)
							{
								// Step 1: window pixels -> screen space
								F32 screenX = sceneMouseX * (getScreenWidth() / static_cast<F32>(winW));
								F32 screenY = sceneMouseY * (getScreenHeight() / static_cast<F32>(winH));

								// Step 2 & 3: account for split-screen viewport
								C4JRender::eViewportType vp = pScene->GetParentLayer()->getViewport();
								S32 displayW = 0, displayH = 0;
								getRenderDimensions(vp, displayW, displayH);

								F32 vpOriginX, vpOriginY, vpW, vpH;
								GetViewportRect(getScreenWidth(), getScreenHeight(), vp, vpOriginX, vpOriginY, vpW, vpH);
								// All viewports use Fit16x9 for menu scenes
								S32 fitW, fitH, fitOffsetX, fitOffsetY;
								Fit16x9(vpW, vpH, fitW, fitH, fitOffsetX, fitOffsetY);
								S32 originX = static_cast<S32>(vpOriginX) + fitOffsetX;
								S32 originY = static_cast<S32>(vpOriginY) + fitOffsetY;
								displayW = fitW;
								displayH = fitH;

								if (displayW > 0 && displayH > 0)
								{
									sceneMouseX = (screenX - originX) * (static_cast<F32>(pScene->getRenderWidth()) / static_cast<F32>(displayW));
									sceneMouseY = (screenY - originY) * (static_cast<F32>(pScene->getRenderHeight()) / static_cast<F32>(displayH));
								}
							}
						}
					}

					// Get main panel offset (controls are positioned relative to it)
					S32 panelOffsetX = 0, panelOffsetY = 0;
					UIControl *pMainPanel = pScene->GetMainPanel();
					if (pMainPanel)
					{
						pMainPanel->UpdateControl();
						panelOffsetX = pMainPanel->getXPos();
						panelOffsetY = pMainPanel->getYPos();
					}

					// Mouse hover — hit test against C++ control bounds.
					// Simple controls use SetFocusToElement; list controls
					// use their own SetTouchFocus for Flash-side hit testing.
					if (mouseMoved)
					{
						m_bMouseHoverHorizontalList = false;
						vector<UIControl *> *controls = pScene->GetControls();
						if (controls)
						{
							int hitControlId = -1;
							S32 hitArea = INT_MAX;
							UIControl *hitCtrl = NULL;
							for (size_t i = 0; i < controls->size(); ++i)
							{
								UIControl *ctrl = (*controls)[i];
								if (!ctrl || ctrl->getHidden() || !ctrl->getVisible() || ctrl->getId() < 0)
									continue;

								UIControl::eUIControlType type = ctrl->getControlType();
								if (type != UIControl::eButton && type != UIControl::eTextInput &&
									type != UIControl::eCheckBox && type != UIControl::eSlider &&
									type != UIControl::eButtonList && type != UIControl::eTexturePackList)
									continue;

								// If the scene has an active panel (e.g. tab menus),
								// skip controls that aren't children of that panel.
								if (pMainPanel && ctrl->getParentPanel() != pMainPanel)
									continue;

								ctrl->UpdateControl();
								S32 cx = ctrl->getXPos() + panelOffsetX;
								S32 cy = ctrl->getYPos() + panelOffsetY;
								S32 cw = ctrl->getWidth();
								S32 ch = ctrl->getHeight();
								// TexturePackList origin is where the slot area starts,
								// not the top-left of the whole control — use GetRealHeight.
								if (type == UIControl::eTexturePackList)
									ch = static_cast<UIControl_TexturePackList*>(ctrl)->GetRealHeight();
								if (cw <= 0 || ch <= 0)
									continue;

								if (sceneMouseX >= cx && sceneMouseX <= cx + cw &&
									sceneMouseY >= cy && sceneMouseY <= cy + ch)
								{
									if (type == UIControl::eButtonList)
									{
										// ButtonList manages focus internally via Flash —
										// pass mouse coords so it can highlight the right item.
										static_cast<UIControl_ButtonList*>(ctrl)->SetTouchFocus(
											static_cast<S32>(sceneMouseX), static_cast<S32>(sceneMouseY), false);
										hitControlId = -1;
										hitArea = INT_MAX;
										hitCtrl = NULL;
										break; // ButtonList takes priority
									}
									if (type == UIControl::eTexturePackList)
									{
										// TexturePackList expects coords relative to its origin.
										UIControl_TexturePackList *pList = static_cast<UIControl_TexturePackList*>(ctrl);
										pScene->SetFocusToElement(ctrl->getId());
										pList->SetTouchFocus(
											static_cast<S32>(sceneMouseX - cx), static_cast<S32>(sceneMouseY - cy), false);
										m_bMouseHoverHorizontalList = true;
										hitControlId = -1;
										hitArea = INT_MAX;
										hitCtrl = NULL;
										break;
									}
									S32 area = cw * ch;
									if (area < hitArea)
									{
										hitControlId = ctrl->getId();
										hitArea = area;
										hitCtrl = ctrl;
										if (type == UIControl::eSlider)
											m_bMouseHoverHorizontalList = true;
									}
								}
							}

							if (hitControlId >= 0 && pScene->getControlFocus() != hitControlId)
							{
								// During direct editing, don't let hover move focus
								// away to other TextInputs (e.g. sign lines).
								if (hitCtrl && hitCtrl->getControlType() == UIControl::eTextInput
									&& pScene->isDirectEditBlocking())
								{
									// Skip — keep focus on the actively-edited input
								}
								else
								{
									pScene->SetFocusToElement(hitControlId);
									// TextInput: SetFocusToElement triggers ChangeState which
									// shows the caret. Hide it immediately — the render pass
									// happens after both tickInput and scene tick, so no flicker.
									if (hitCtrl && hitCtrl->getControlType() == UIControl::eTextInput)
									{
										static_cast<UIControl_TextInput*>(hitCtrl)->setCaretVisible(false);
									}
								}
							}
						}
					}

					bool leftPressed = g_KBMInput.IsMouseButtonPressed(KeyboardMouseInput::MOUSE_LEFT);
					bool leftDown = leftPressed || g_KBMInput.IsMouseButtonDown(KeyboardMouseInput::MOUSE_LEFT);

					if (m_mouseDraggingSliderScene != eUIScene_COUNT && m_mouseDraggingSliderScene != pScene->getSceneType())
					{
						m_mouseDraggingSliderScene = eUIScene_COUNT;
						m_mouseDraggingSliderId = -1;
					}

					if (leftPressed)
					{
						vector<UIControl *> *controls = pScene->GetControls();
						if (controls)
						{
							// Set Iggy dispatch focus for TextInput on click (not hover)
							// so ACTION_MENU_OK targets the correct text field.
							for (size_t i = 0; i < controls->size(); ++i)
							{
								UIControl *ctrl = (*controls)[i];
								if (!ctrl || ctrl->getControlType() != UIControl::eTextInput || !ctrl->getVisible())
									continue;
								if (pMainPanel && ctrl->getParentPanel() != pMainPanel)
									continue;
								ctrl->UpdateControl();
								S32 cx = ctrl->getXPos() + panelOffsetX;
								S32 cy = ctrl->getYPos() + panelOffsetY;
								S32 cw = ctrl->getWidth();
								S32 ch = ctrl->getHeight();
								if (cw > 0 && ch > 0 &&
									sceneMouseX >= cx && sceneMouseX <= cx + cw &&
									sceneMouseY >= cy && sceneMouseY <= cy + ch)
								{
									Iggy *movie = pScene->getMovie();
									IggyFocusHandle currentFocus = IGGY_FOCUS_NULL;
									IggyFocusableObject focusables[64];
									S32 numFocusables = 0;
									IggyPlayerGetFocusableObjects(movie, &currentFocus, focusables, 64, &numFocusables);
									for (S32 fi = 0; fi < numFocusables && fi < 64; ++fi)
									{
										if (sceneMouseX >= focusables[fi].x0 && sceneMouseX <= focusables[fi].x1 &&
											sceneMouseY >= focusables[fi].y0 && sceneMouseY <= focusables[fi].y1)
										{
											IggyPlayerSetFocusRS(movie, focusables[fi].object, 0);
											break;
										}
									}
									break;
								}
							}

							for (size_t i = 0; i < controls->size(); ++i)
							{
								UIControl *ctrl = (*controls)[i];
								if (!ctrl || ctrl->getControlType() != UIControl::eSlider || !ctrl->getVisible())
									continue;

								if (pMainPanel && ctrl->getParentPanel() != pMainPanel)
									continue;

								UIControl_Slider *pSlider = static_cast<UIControl_Slider *>(ctrl);
								pSlider->UpdateControl();
								S32 cx = pSlider->getXPos() + panelOffsetX;
								S32 cy = pSlider->getYPos() + panelOffsetY;
								S32 cw = pSlider->GetRealWidth();
								S32 ch = pSlider->getHeight();
								if (cw <= 0 || ch <= 0)
									continue;

								if (sceneMouseX >= cx && sceneMouseX <= cx + cw && sceneMouseY >= cy && sceneMouseY <= cy + ch)
								{
									m_mouseDraggingSliderScene = pScene->getSceneType();
									m_mouseDraggingSliderId = pSlider->getId();
									break;
								}
							}
						}
					}

					if (leftDown && m_mouseDraggingSliderScene == pScene->getSceneType() && m_mouseDraggingSliderId >= 0)
					{
						UIControl_Slider *pSlider = FindSliderById(pScene, m_mouseDraggingSliderId);
						if (pSlider && pSlider->getVisible())
						{
							pSlider->UpdateControl();
							S32 sliderX = pSlider->getXPos() + panelOffsetX;
							S32 sliderWidth = pSlider->GetRealWidth();
							if (sliderWidth > 0)
							{
								float fNewSliderPos = (sceneMouseX - static_cast<float>(sliderX)) / static_cast<float>(sliderWidth);
								if (fNewSliderPos < 0.0f) fNewSliderPos = 0.0f;
								if (fNewSliderPos > 1.0f) fNewSliderPos = 1.0f;
								pSlider->SetSliderTouchPos(fNewSliderPos);
							}
						}
						else
						{
							m_mouseDraggingSliderScene = eUIScene_COUNT;
							m_mouseDraggingSliderId = -1;
						}
					}
					else if (!leftDown)
					{
						m_mouseDraggingSliderScene = eUIScene_COUNT;
						m_mouseDraggingSliderId = -1;
					}

					// Let the scene handle mouse clicks for custom navigation (e.g. crafting slots)
					if (leftPressed && m_mouseDraggingSliderId < 0)
					{
						m_mouseClickConsumedByScene = pScene->handleMouseClick(sceneMouseX, sceneMouseY);
					}
				}
			}
#endif
			handleInput();
			++m_accumulatedTicks;
		}
	}
}

void UIController::handleInput()
{
	// For each user, loop over each key type and send messages based on the state
	for(unsigned int iPad = 0; iPad < XUSER_MAX_COUNT; ++iPad)
	{
#ifdef _DURANGO
		// 4J-JEV: Added exception for primary play who migh've uttered speech commands.
		if(iPad != ProfileManager.GetPrimaryPad()
			&& (!InputManager.IsPadConnected(iPad) || !InputManager.IsPadLocked(iPad)) ) continue;
#endif
		for(unsigned int key = 0; key <= ACTION_MAX_MENU; ++key)
		{
			handleKeyPress(iPad, key);
		}

#ifdef __PSVITA__
		//CD - Vita requires key press 40 - select [MINECRAFT_ACTION_GAME_INFO]
		handleKeyPress(iPad, MINECRAFT_ACTION_GAME_INFO);
#endif
	}

#ifdef _DURANGO
	if(!app.GetGameStarted())
	{
		bool repeat = false;
		int firstUnfocussedUnhandledPad = -1;

		// For durango, check for unmapped controllers
		for(unsigned int iPad = XUSER_MAX_COUNT; iPad < (XUSER_MAX_COUNT + InputManager.MAX_GAMEPADS); ++iPad)
		{
			if(InputManager.IsPadLocked(iPad) || !InputManager.IsPadConnected(iPad) ) continue;

			for(unsigned int key = 0; key <= ACTION_MAX_MENU; ++key)
			{

				bool pressed = InputManager.ButtonPressed(iPad,key); // Toggle
				bool released = InputManager.ButtonReleased(iPad,key); // Toggle

				if(pressed || released)
				{
					bool handled = false;

					// Send the key to the fullscreen group first
					m_groups[(int)eUIGroup_Fullscreen]->handleInput(iPad, key, repeat, pressed, released, handled);

					if(firstUnfocussedUnhandledPad < 0 && !m_groups[(int)eUIGroup_Fullscreen]->HasFocus(iPad))
					{
						firstUnfocussedUnhandledPad = iPad;
					}
				}
			}
		}

		if(ProfileManager.GetLockedProfile() >= 0 && !InputManager.IsPadLocked( ProfileManager.GetLockedProfile() ) && firstUnfocussedUnhandledPad >= 0)
		{
			ProfileManager.RequestSignInUI(false, false, false, false, true, nullptr, nullptr, firstUnfocussedUnhandledPad );
		}
	}
#endif
}

void UIController::handleKeyPress(unsigned int iPad, unsigned int key)
{

	bool down = false;
	bool pressed = false; // Toggle
	bool released = false; // Toggle
	bool repeat = false;

#ifdef __PSVITA__
	if(key==ACTION_MENU_OK)
	{
		bool bTouchScreenInput=false;

		// check the touchscreen

		// 4J-PB - use the touchscreen for quickselect
		SceTouchData* pTouchData = InputManager.GetTouchPadData(iPad,false);

		if((m_bTouchscreenPressed==false) && pTouchData->reportNum==1)
		{
			// no active touch? clear active and highlighted touch UI elements
			m_ActiveUIElement = nullptr;
			m_HighlightedUIElement = nullptr;

			// fullscreen first
			UIScene *pScene=m_groups[(int)eUIGroup_Fullscreen]->getCurrentScene();
			// also check tooltip scene if we're not touching anything in the main scene
			UIScene *pToolTips=m_groups[(int)eUIGroup_Fullscreen]->getTooltips();
			if(pScene)
			{
				// scene touch check
				if(TouchBoxHit(pScene,pTouchData->report[0].x,pTouchData->report[0].y))
				{
					down=pressed=m_bTouchscreenPressed=true;
					bTouchScreenInput=true;
				}
				// tooltip touch check
				else if(TouchBoxHit(pToolTips,pTouchData->report[0].x,pTouchData->report[0].y))
				{
					down=pressed=m_bTouchscreenPressed=true;
					bTouchScreenInput=true;
				}
			}
			else
			{
				pScene=m_groups[(EUIGroup)(iPad+1)]->getCurrentScene();
				pToolTips=m_groups[(int)iPad+1]->getTooltips();
				if(pScene)
				{
					// scene touch check
					if(TouchBoxHit(pScene,pTouchData->report[0].x,pTouchData->report[0].y))
					{
						down=pressed=m_bTouchscreenPressed=true;
						bTouchScreenInput=true;
					}
					// tooltip touch check (if scene exists but not component has been touched)
					else if(TouchBoxHit(pToolTips,pTouchData->report[0].x,pTouchData->report[0].y))
					{
						down=pressed=m_bTouchscreenPressed=true;
						bTouchScreenInput=true;
					}
				}
				else if(pToolTips)
				{
					// tooltip touch check (if scene does not exist)
					if(TouchBoxHit(pToolTips,pTouchData->report[0].x,pTouchData->report[0].y))
					{
						down=pressed=m_bTouchscreenPressed=true;
						bTouchScreenInput=true;
					}
				}
			}
		}
		else if(m_bTouchscreenPressed && pTouchData->reportNum==1)
		{
			// fullscreen first
			UIScene *pScene=m_groups[(int)eUIGroup_Fullscreen]->getCurrentScene();
			// also check tooltip scene if we're not touching anything in the main scene
			UIScene *pToolTips=m_groups[(int)eUIGroup_Fullscreen]->getTooltips();
			if(pScene)
			{
				// scene touch check
				if(TouchBoxHit(pScene,pTouchData->report[0].x,pTouchData->report[0].y))
				{
					down=true;
					bTouchScreenInput=true;
				}
				// tooltip touch check (if scene exists but not component has been touched)
				else if(TouchBoxHit(pToolTips,pTouchData->report[0].x,pTouchData->report[0].y))
				{
					down=true;
					bTouchScreenInput=true;
				}
			}
			else
			{
				pScene=m_groups[(EUIGroup)(iPad+1)]->getCurrentScene();
				pToolTips=m_groups[(int)iPad+1]->getTooltips();
				if(pScene)
				{
					// scene touch check
					if(TouchBoxHit(pScene,pTouchData->report[0].x,pTouchData->report[0].y))
					{
						down=true;
						bTouchScreenInput=true;
					}
					// tooltip touch check (if scene exists but not component has been touched)
					else if(TouchBoxHit(pToolTips,pTouchData->report[0].x,pTouchData->report[0].y))
					{
						down=true;
						bTouchScreenInput=true;
					}
				}
				else if(pToolTips)
				{
					// tooltip touch check (if scene does not exist)
					if(TouchBoxHit(pToolTips,pTouchData->report[0].x,pTouchData->report[0].y))
					{
						down=true;
						bTouchScreenInput=true;
					}
				}
			}
		}
		else if(m_bTouchscreenPressed && pTouchData->reportNum==0)
		{
			// released
			bTouchScreenInput=true;
			m_bTouchscreenPressed=false;
			released=true;
		}

		if(pressed)
		{
			// Start repeat timer
			m_actionRepeatTimer[iPad][key] = GetTickCount() + UI_REPEAT_KEY_DELAY_MS;
		}
		else if (released)
		{
			// Stop repeat timer
			m_actionRepeatTimer[iPad][key] = 0;
		}
		else if (down)
		{
			// Check is enough time has elapsed to be a repeat key
			DWORD currentTime = GetTickCount();
			if(m_actionRepeatTimer[iPad][key] > 0 && currentTime > m_actionRepeatTimer[iPad][key])
			{
				repeat = true;
				pressed = true;
				m_actionRepeatTimer[iPad][key] = currentTime + UI_REPEAT_KEY_REPEAT_RATE_MS;
			}
		}

		// handle touch input
		HandleTouchInput(iPad, key, pressed, repeat, released);

		// ignore any other presses if the touchscreen has been used
		if(bTouchScreenInput) return;
	}
#endif

	down = InputManager.ButtonDown(iPad,key);
	pressed = InputManager.ButtonPressed(iPad,key); // Toggle
	released = InputManager.ButtonReleased(iPad,key); // Toggle

#ifdef _WINDOWS64
	if (iPad == 0)
	{
		int vk = 0;
		switch (key)
		{
		case ACTION_MENU_OK:    case ACTION_MENU_A: vk = VK_RETURN; break;
		case ACTION_MENU_CANCEL: case ACTION_MENU_B: vk = VK_ESCAPE; break;
		case ACTION_MENU_UP:    vk = VK_UP;     break;
		case ACTION_MENU_DOWN:  vk = VK_DOWN;   break;
		case ACTION_MENU_LEFT:  vk = VK_LEFT;   break;
		case ACTION_MENU_RIGHT: vk = VK_RIGHT;  break;
		case ACTION_MENU_X:     vk = 'R';       break;
		case ACTION_MENU_Y:     vk = VK_TAB;    break;
		case ACTION_MENU_LEFT_SCROLL:  vk = 'Q'; break;
		case ACTION_MENU_RIGHT_SCROLL: vk = 'E'; break;
		case ACTION_MENU_PAGEUP:   vk = VK_PRIOR; break;
		case ACTION_MENU_PAGEDOWN: vk = VK_NEXT;  break;
		}
		if (vk != 0)
		{
			if (g_KBMInput.IsKeyPressed(vk))  { pressed = true; down = true; }
			if (g_KBMInput.IsKeyReleased(vk)) { released = true; down = false; }
			if (!pressed && !released && g_KBMInput.IsKeyDown(vk)) { down = true; }
		}

		if ((key == ACTION_MENU_OK || key == ACTION_MENU_A) && !g_KBMInput.IsMouseGrabbed())
		{
			if (m_mouseDraggingSliderId < 0 && !m_mouseClickConsumedByScene)
			{
				if (g_KBMInput.IsMouseButtonPressed(KeyboardMouseInput::MOUSE_LEFT))  { pressed = true; down = true; }
				if (g_KBMInput.IsMouseButtonReleased(KeyboardMouseInput::MOUSE_LEFT)) { released = true; down = false; }
				if (!pressed && !released && g_KBMInput.IsMouseButtonDown(KeyboardMouseInput::MOUSE_LEFT)) { down = true; }
			}
		}

		// Right click → ACTION_MENU_X (pick up half stack in inventory)
		if (key == ACTION_MENU_X && !g_KBMInput.IsMouseGrabbed())
		{
			if (g_KBMInput.IsMouseButtonPressed(KeyboardMouseInput::MOUSE_RIGHT))  { pressed = true; down = true; }
			if (g_KBMInput.IsMouseButtonReleased(KeyboardMouseInput::MOUSE_RIGHT)) { released = true; down = false; }
			if (!pressed && !released && g_KBMInput.IsMouseButtonDown(KeyboardMouseInput::MOUSE_RIGHT)) { down = true; }
		}

		// Scroll wheel for list scrolling — only consume the wheel value when the
		// action key actually matches, so the other direction isn't lost.
		if (!g_KBMInput.IsMouseGrabbed() && (key == ACTION_MENU_OTHER_STICK_UP || key == ACTION_MENU_OTHER_STICK_DOWN))
		{
			int wheel = g_KBMInput.PeekMouseWheel();
			if (key == ACTION_MENU_OTHER_STICK_UP && wheel > 0)
			{
				g_KBMInput.ConsumeMouseWheel();
				pressed = true;
				down = true;
			}
			else if (key == ACTION_MENU_OTHER_STICK_DOWN && wheel < 0)
			{
				g_KBMInput.ConsumeMouseWheel();
				pressed = true;
				down = true;
			}

			// Remap scroll wheel to navigation actions. Use LEFT/RIGHT when
			// hovering a horizontal list (e.g. TexturePackList), UP/DOWN otherwise.
			if (pressed && g_KBMInput.IsKBMActive())
			{
				if (m_bMouseHoverHorizontalList)
					key = (key == ACTION_MENU_OTHER_STICK_UP) ? ACTION_MENU_LEFT : ACTION_MENU_RIGHT;
				else
					key = (key == ACTION_MENU_OTHER_STICK_UP) ? ACTION_MENU_UP : ACTION_MENU_DOWN;
			}
		}
	}
#endif

	if(pressed) app.DebugPrintf("Pressed %d\n",key);
	if(released) app.DebugPrintf("Released %d\n",key);
	// Repeat handling
	if(pressed)
	{
		// Start repeat timer
		m_actionRepeatTimer[iPad][key] = GetTickCount() + UI_REPEAT_KEY_DELAY_MS;
	}
	else if (released)
	{
		// Stop repeat timer
		m_actionRepeatTimer[iPad][key] = 0;
	}
	else if (down)
	{
		// Check is enough time has elapsed to be a repeat key
		DWORD currentTime = GetTickCount();
		if(m_actionRepeatTimer[iPad][key] > 0 && currentTime > m_actionRepeatTimer[iPad][key])
		{
			repeat = true;
			pressed = true;
			m_actionRepeatTimer[iPad][key] = currentTime + UI_REPEAT_KEY_REPEAT_RATE_MS;
		}
	}

#ifndef _CONTENT_PACKAGE

#ifdef ENABLE_IGGY_PERFMON
	if ( pressed && !repeat && key == ACTION_MENU_STICK_PRESS)
	{
		m_iggyPerfmonEnabled = !m_iggyPerfmonEnabled;
	}
#endif

	// 4J Stu - Removed this function
#if 0
#ifdef __PS3__
	//if (	pressed &&
	//	!repeat &&
	//	//app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_ToggleFont) &&
	//	key == ACTION_MENU_STICK_PRESS)
	//{
	//	static bool whichFont = true;
	//	if (whichFont)
	//	{
	//		IggyFontSetIndirectUTF8( "Mojangles_7", -1, IGGY_FONTFLAG_all, "Mojangles_TTF",-1 ,IGGY_FONTFLAG_none );
	//		IggyFontSetIndirectUTF8( "Mojangles_11", -1, IGGY_FONTFLAG_all, "Mojangles_TTF",-1 ,IGGY_FONTFLAG_none );
	//		whichFont = false;
	//	}
	//	else
	//	{
	//		IggyFontSetIndirectUTF8( "Mojangles_7", -1, IGGY_FONTFLAG_all, "Mojangles_7",-1 ,IGGY_FONTFLAG_none );
	//		IggyFontSetIndirectUTF8( "Mojangles_11", -1, IGGY_FONTFLAG_all, "Mojangles_11",-1 ,IGGY_FONTFLAG_none );
	//		whichFont = true;
	//	}
	//}
	//else
	if (	pressed &&
		!repeat &&
		//!(app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_ToggleFont)) &&
		key == ACTION_MENU_STICK_PRESS)
	{
		int64_t totalStatic = 0;
		int64_t totalDynamic = 0;
		app.DebugPrintf(app.USER_SR, "********************************\n");
		app.DebugPrintf(app.USER_SR, "BEGIN TOTAL SWF MEMORY USAGE\n\n");
		for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
		{
			m_groups[i]->PrintTotalMemoryUsage(totalStatic, totalDynamic);
		}
		for(unsigned int i = 0; i < eLibrary_Count; ++i)
		{
			int64_t libraryStatic = 0;
			int64_t libraryDynamic = 0;

			if(m_iggyLibraries[i] != IGGY_INVALID_LIBRARY)
			{

				IggyMemoryUseInfo memoryInfo;
				rrbool res;
				int iteration = 0;
				while(res = IggyDebugGetMemoryUseInfo ( nullptr ,
					m_iggyLibraries[i] ,
					"" ,
					0 ,
					iteration ,
					&memoryInfo ))
				{
					libraryStatic += memoryInfo.static_allocation_bytes;
					libraryDynamic += memoryInfo.dynamic_allocation_bytes;
					totalStatic += memoryInfo.static_allocation_bytes;
					totalDynamic += memoryInfo.dynamic_allocation_bytes;
					++iteration;
				}
			}

			app.DebugPrintf(app.USER_SR, "Library static: %dB , Library dynamic: %d, ID: %d\n", libraryStatic, libraryDynamic, i);
		}
		app.DebugPrintf(app.USER_SR, "Total static: %d , Total dynamic: %d\n", totalStatic, totalDynamic);
		app.DebugPrintf(app.USER_SR, "\n\nEND TOTAL SWF MEMORY USAGE\n");
		app.DebugPrintf(app.USER_SR, "********************************\n\n");
	}
	else
#endif
#endif
#endif
		//#endif
		if(repeat || pressed || released)
		{
			bool handled = false;

			// Send the key to the fullscreen group first
			m_groups[static_cast<int>(eUIGroup_Fullscreen)]->handleInput(iPad, key, repeat, pressed, released, handled);
			if(!handled)
			{
				// If it's not been handled yet, then pass the event onto the players specific group
				m_groups[(iPad+1)]->handleInput(iPad, key, repeat, pressed, released, handled);
			}
		}
}

rrbool RADLINK UIController::ExternalFunctionCallback( void * user_callback_data , Iggy * player , IggyExternalFunctionCallUTF16 * call)
{
	UIScene *scene = static_cast<UIScene *>(IggyPlayerGetUserdata(player));

	if(scene != nullptr)
	{
		scene->externalCallback(call);
	}

	return true;
}

// RENDERING
void UIController::renderScenes()
{
	PIXBeginNamedEvent(0, "Rendering Iggy scenes");
	// Only render player scenes if the game is started
	if(app.GetGameStarted() && !m_groups[eUIGroup_Fullscreen]->hidesLowerScenes())
	{
		for(int i = eUIGroup_Player1; i < eUIGroup_COUNT; ++i)
		{
			PIXBeginNamedEvent(0, "Rendering layer %d scenes", i);
			m_groups[i]->render();
			PIXEndNamedEvent();
		}
	}

	// Always render the fullscreen group
	PIXBeginNamedEvent(0, "Rendering fullscreen scenes");
	m_groups[eUIGroup_Fullscreen]->render();
	PIXEndNamedEvent();

	PIXEndNamedEvent();

#ifdef ENABLE_IGGY_PERFMON
	if (m_iggyPerfmonEnabled)
	{
		IggyPerfmonPad pm_pad;

		pm_pad.bits = 0;
		pm_pad.field.dpad_up           = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_UP);
		pm_pad.field.dpad_down         = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_DOWN);
		pm_pad.field.dpad_left         = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_LEFT);
		pm_pad.field.dpad_right        = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_RIGHT);
		pm_pad.field.button_up         = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_Y);
		pm_pad.field.button_down       = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_A);
		pm_pad.field.button_left       = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_X);
		pm_pad.field.button_right      = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_B);
		pm_pad.field.shoulder_left_hi  = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_LEFT_SCROLL);
		pm_pad.field.shoulder_right_hi = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_RIGHT_SCROLL);
		pm_pad.field.trigger_left_low  = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_PAGEUP);
		pm_pad.field.trigger_right_low = InputManager.ButtonPressed(ProfileManager.GetPrimaryPad(),ACTION_MENU_PAGEDOWN);
		//IggyPerfmonPadFromXInputStatePointer(pm_pad, &xi_pad);

		//gdraw_D3D_SetTileOrigin( fb,
		//	zb,
		//	PM_ORIGIN_X,
		//	PM_ORIGIN_Y );
		IggyPerfmonTickAndDraw(iggy_perfmon, gdraw_funcs, &pm_pad,
			PM_ORIGIN_X, PM_ORIGIN_Y, getScreenWidth(), getScreenHeight());     // perfmon draw area in window coords
	}
#endif
}

void UIController::getRenderDimensions(C4JRender::eViewportType viewport, S32 &width, S32 &height)
{
	F32 originX, originY, viewW, viewH;
	GetViewportRect(getScreenWidth(), getScreenHeight(), viewport, originX, originY, viewW, viewH);

	if(viewport == C4JRender::VIEWPORT_TYPE_FULLSCREEN)
	{
		S32 offsetX, offsetY;
		Fit16x9(viewW, viewH, width, height, offsetX, offsetY);
	}
	else
	{
		// Split-screen: use raw viewport dims — the SWF tiling code handles non-16:9
		width = static_cast<S32>(viewW);
		height = static_cast<S32>(viewH);
	}
}

void UIController::setupRenderPosition(C4JRender::eViewportType viewport)
{
	m_currentRenderViewport = viewport;
	m_bCustomRenderPosition = false;

	F32 vpOriginX, vpOriginY, vpW, vpH;
	GetViewportRect(getScreenWidth(), getScreenHeight(), viewport, vpOriginX, vpOriginY, vpW, vpH);

	S32 xPos, yPos;
	if(viewport == C4JRender::VIEWPORT_TYPE_FULLSCREEN)
	{
		S32 fitW, fitH, fitOffsetX, fitOffsetY;
		Fit16x9(vpW, vpH, fitW, fitH, fitOffsetX, fitOffsetY);
		xPos = static_cast<S32>(vpOriginX) + fitOffsetX;
		yPos = static_cast<S32>(vpOriginY) + fitOffsetY;
	}
	else
	{
		// Split-screen: position at viewport origin, no 16:9 fitting
		xPos = static_cast<S32>(vpOriginX);
		yPos = static_cast<S32>(vpOriginY);
	}

	m_tileOriginX = xPos;
	m_tileOriginY = yPos;
	setTileOrigin(xPos, yPos);
}

void UIController::setupRenderPosition(S32 xOrigin, S32 yOrigin)
{
	m_bCustomRenderPosition = true;
	m_tileOriginX = xOrigin;
	m_tileOriginY = yOrigin;
	setTileOrigin(xOrigin, yOrigin);
}

void UIController::setupCustomDrawGameState()
{
	// Rest the clear rect
	m_customRenderingClearRect.left = LONG_MAX;
	m_customRenderingClearRect.right = LONG_MIN;
	m_customRenderingClearRect.top = LONG_MAX;
	m_customRenderingClearRect.bottom = LONG_MIN;

#if defined _WINDOWS64 || _DURANGO
	PIXBeginNamedEvent(0,"StartFrame");
	RenderManager.StartFrame();
	PIXEndNamedEvent();
	gdraw_D3D11_setViewport_4J();
#elif defined __PS3__
	RenderManager.StartFrame();
#elif defined __PSVITA__
	RenderManager.StartFrame();
#elif defined __ORBIS__
	RenderManager.StartFrame(false);
	// Set up a viewport for the render that matches Iggy's own viewport, apart form using an opengl-style z-range (Iggy uses a DX-style range on PS4), so
	// that the renderer orthographic projection will work
	gdraw_orbis_setViewport_4J();
#endif
	RenderManager.Set_matrixDirty();

	// 4J Stu - We don't need to clear this here as iggy hasn't written anything to the depth buffer.
	// We DO however clear after we render which is why we still setup the rectangle here
	//RenderManager.Clear(GL_DEPTH_BUFFER_BIT, &m_customRenderingClearRect);
	//glClear(GL_DEPTH_BUFFER_BIT);

	PIXBeginNamedEvent(0,"Final setup");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_fScreenWidth, m_fScreenHeight, 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);
	PIXEndNamedEvent();
}

void UIController::setupCustomDrawMatrices(UIScene *scene, CustomDrawData *customDrawRegion)
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	// Clear just the region required for this control.
	float sceneWidth = static_cast<float>(scene->getRenderWidth());
	float sceneHeight = static_cast<float>(scene->getRenderHeight());

	LONG left, right, top, bottom;
#ifdef __PS3__
	if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		// 4J Stu - Our SD target on PS3 is double width
		left = m_tileOriginX + (sceneWidth + customDrawRegion->mat[(0*4)+3]*sceneWidth);
		right = left + ( (sceneWidth * customDrawRegion->mat[0]) ) * customDrawRegion->x1;
	}
	else
#endif
	{
		left = m_tileOriginX + (sceneWidth + customDrawRegion->mat[(0*4)+3]*sceneWidth)/2;
		right = left + ( (sceneWidth * customDrawRegion->mat[0])/2 ) * customDrawRegion->x1;
	}

	top = m_tileOriginY + (sceneHeight - customDrawRegion->mat[(1*4)+3]*sceneHeight)/2;
	bottom = top + (sceneHeight * -customDrawRegion->mat[(1*4) + 1])/2 * customDrawRegion->y1;

	m_customRenderingClearRect.left = min(m_customRenderingClearRect.left, left);
	m_customRenderingClearRect.right = max(m_customRenderingClearRect.right, right);;
	m_customRenderingClearRect.top = min(m_customRenderingClearRect.top, top);
	m_customRenderingClearRect.bottom = max(m_customRenderingClearRect.bottom, bottom);

	if(!m_bScreenWidthSetup)
	{
		Minecraft *pMinecraft=Minecraft::GetInstance();
		if(pMinecraft != nullptr)
		{
			m_fScreenWidth=static_cast<float>(pMinecraft->width_phys);
			m_fScreenHeight=static_cast<float>(pMinecraft->height_phys);
			m_bScreenWidthSetup = true;
		}
	}

	glLoadIdentity();
	glTranslatef(0, 0, -2000);
	// Iggy translations are based on a double-size target, with the origin in the centre
	glTranslatef((m_fScreenWidth + customDrawRegion->mat[(0*4)+3]*m_fScreenWidth)/2,(m_fScreenHeight - customDrawRegion->mat[(1*4)+3]*m_fScreenHeight)/2,0);
	// Iggy scales are based on a double-size target
	glScalef( (m_fScreenWidth * customDrawRegion->mat[0])/2,(m_fScreenHeight * -customDrawRegion->mat[(1*4) + 1])/2,1.0f);
}

void UIController::setupCustomDrawGameStateAndMatrices(UIScene *scene, CustomDrawData *customDrawRegion)
{
	setupCustomDrawGameState();
	setupCustomDrawMatrices(scene, customDrawRegion);
}

void UIController::endCustomDrawGameState()
{
#ifdef __ORBIS__
	// TO BE IMPLEMENTED
	RenderManager.Clear(GL_DEPTH_BUFFER_BIT);
#else
	RenderManager.Clear(GL_DEPTH_BUFFER_BIT, &m_customRenderingClearRect);
#endif
	//glClear(GL_DEPTH_BUFFER_BIT);
	glDepthMask(false);
	glDisable(GL_ALPHA_TEST);
}

void UIController::endCustomDrawMatrices()
{
}

void UIController::endCustomDrawGameStateAndMatrices()
{
	endCustomDrawMatrices();
	endCustomDrawGameState();
}

void RADLINK UIController::CustomDrawCallback(void *user_callback_data, Iggy *player, IggyCustomDrawCallbackRegion *region)
{
	UIScene *scene = static_cast<UIScene *>(IggyPlayerGetUserdata(player));

	if(scene != nullptr)
	{
		scene->customDraw(region);
	}
}

//Description
//Callback to create a user-defined texture to replace SWF-defined textures.
//Parameters
//width - Input value: optional number of pixels wide specified from AS3, or -1 if not defined. Output value: the number of pixels wide to pretend to Iggy that the bitmap is. SWF and AS3 scales bitmaps based on their pixel dimensions, so you can use this to substitute a texture that is higher or lower resolution that ActionScript thinks it is.
//height - Input value: optional number of pixels high specified from AS3, or -1 if not defined. Output value: the number of pixels high to pretend to Iggy that the bitmap is. SWF and AS3 scales bitmaps based on their pixel dimensions, so you can use this to substitute a texture that is higher or lower resolution that ActionScript thinks it is.
//destroy_callback_data - Optional additional output value you can set; the value will be passed along to the corresponding Iggy_TextureSubstitutionDestroyCallback (e.g. you can store the pointer to your own internal structure here).
//return - A platform-independent wrapped texture handle provided by GDraw, or nullptr (nullptr with throw an ActionScript 3 ArgumentError that the Flash developer can catch) Use by calling IggySetTextureSubstitutionCallbacks.
//
//Discussion
//
//If your texture includes an alpha channel, you must use a premultiplied alpha (where the R,G, and B channels have been multiplied by the alpha value); all Iggy shaders assume premultiplied alpha (and it looks better anyway).
GDrawTexture * RADLINK UIController::TextureSubstitutionCreateCallback ( void * user_callback_data , IggyUTF16 * texture_name , S32 * width , S32 * height , void * * destroy_callback_data )
{
	UIController *uiController = static_cast<UIController *>(user_callback_data);
    auto it = uiController->m_substitutionTextures.find(texture_name);

    if(it != uiController->m_substitutionTextures.end())
	{
		app.DebugPrintf("Found substitution texture %ls, with %d bytes\n", texture_name,it->second.length);

		BufferedImage image(it->second.data, it->second.length);
		if( image.getData() != nullptr )
		{
			image.preMultiplyAlpha();
			Textures *t = Minecraft::GetInstance()->textures;
			int id = t->getTexture(&image,C4JRender::TEXTURE_FORMAT_RxGyBzAw,false);

			// 4J Stu - All our flash controls that allow replacing textures use a special 64x64 symbol
			// Force this size here so that our images don't get scaled wildly
	#if (defined __ORBIS__ || defined _DURANGO )
			*width = 96;
			*height = 96;
	#else
			*width = 64;
			*height = 64;
	#endif

	#if defined _WINDOWS64
            // Only set the size to 96x96 for 1080p on Windows
            UIScene *scene = uiController->GetTopScene(0);
            if (scene->getSceneResolution() == UIScene::eSceneResolution_1080)
            {
                *width = 96;
                *height = 96;
			}
	#endif

			*destroy_callback_data = (void *)id;

			app.DebugPrintf("Found substitution texture %ls (%d) - %dx%d\n", static_cast<wchar_t *>(texture_name), id, image.getWidth(), image.getHeight());
			return ui.getSubstitutionTexture(id);
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		app.DebugPrintf("Could not find substitution texture %ls\n", static_cast<wchar_t *>(texture_name));
		return nullptr;
	}
}

//Description
//Callback received from Iggy when it stops using a user-defined texture.
void RADLINK UIController::TextureSubstitutionDestroyCallback ( void * user_callback_data , void * destroy_callback_data , GDrawTexture * handle )
{
	// Orbis complains about casting a pointer to an int
	LONGLONG llVal=(LONGLONG)destroy_callback_data;
	int id=static_cast<int>(llVal);
	app.DebugPrintf("Destroying iggy texture %d\n", id);

	ui.destroySubstitutionTexture(user_callback_data, handle);

	Minecraft* mc = Minecraft::GetInstance();
	if (mc && mc->textures)
	{
		mc->textures->releaseTexture( id );
	}
}

void UIController::registerSubstitutionTexture(const wstring &textureName, PBYTE pbData, DWORD dwLength)
{
	// Remove it if it already exists
	unregisterSubstitutionTexture(textureName,false);

	m_substitutionTextures[textureName] = byteArray(pbData, dwLength);
}

void UIController::unregisterSubstitutionTexture(const wstring &textureName, bool deleteData)
{
    auto it = m_substitutionTextures.find(textureName);

    if(it != m_substitutionTextures.end())
	{
		if(deleteData) delete [] it->second.data;
		m_substitutionTextures.erase(it);
	}
}

// NAVIGATION
bool UIController::NavigateToScene(int iPad, EUIScene scene, void *initData, EUILayer layer, EUIGroup group)
{
	static bool bSeenUpdateTextThisSession = false;
	#if 0 // Disable since we don't use this
	// If you're navigating to the multigamejoinload, and the player hasn't seen the updates message yet, display it now
	// display this message the first 3 times
	if((scene==eUIScene_LoadOrJoinMenu) && (bSeenUpdateTextThisSession==false) && ( app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_DisplayUpdateMessage)!=0))
	{
		scene=eUIScene_NewUpdateMessage;
		bSeenUpdateTextThisSession=true;
	}
	#endif

	// if you're trying to navigate to the inventory,the crafting, pause or game info or any of the trigger scenes and there's already a menu up (because you were pressing a few buttons at the same time) then ignore the navigate
	if(GetMenuDisplayed(iPad))
	{
		switch(scene)
		{
		case eUIScene_PauseMenu:
		case eUIScene_Crafting2x2Menu:
		case eUIScene_Crafting3x3Menu:
		case eUIScene_FurnaceMenu:
		case eUIScene_ContainerMenu:
		case eUIScene_LargeContainerMenu:
		case eUIScene_InventoryMenu:
		case eUIScene_CreativeMenu:
		case eUIScene_DispenserMenu:
		case eUIScene_SignEntryMenu:
		case eUIScene_InGameInfoMenu:
		case eUIScene_EnchantingMenu:
		case eUIScene_BrewingStandMenu:
		case eUIScene_AnvilMenu:
		case eUIScene_TradingMenu:
		case eUIScene_BeaconMenu:
		case eUIScene_HorseMenu:
			app.DebugPrintf("IGNORING NAVIGATE - we're trying to navigate to a user selected scene when there's already a scene up: pad:%d, scene:%d\n", iPad, scene);
			return false;
			break;
		}
	}

	switch(scene)
	{
	case eUIScene_FullscreenProgress:
		{
			// 4J Stu - The fullscreen progress scene should not interfere with any other scene stack, so should be placed in it's own group/layer
			layer = eUILayer_Fullscreen;
			group = eUIGroup_Fullscreen;
		}
		break;
	case eUIScene_ConnectingProgress:
		{
			// The connecting progress scene shouldn't interfere with other scenes
			layer = eUILayer_Fullscreen;
		}
		break;
	case eUIScene_EndPoem:
		{
			// The end poem scene shouldn't interfere with other scenes, but will be underneath the autosave progress
			group = eUIGroup_Fullscreen;
			layer = eUILayer_Scene;
		}
		break;
	};
	int menuDisplayedPad = XUSER_INDEX_ANY;
	if(group == eUIGroup_PAD)
	{
		if( app.GetGameStarted() )
		{
			// If the game isn't running treat as user 0, otherwise map index directly from pad
			if( ( iPad != 255 ) && ( iPad >= 0 ) )
			{
				menuDisplayedPad = iPad;
				group = static_cast<EUIGroup>(iPad + 1);
			}
			else group = eUIGroup_Fullscreen;
		}
		else
		{
			layer = eUILayer_Fullscreen;
			group = eUIGroup_Fullscreen;
		}
	}

	PerformanceTimer timer;

	EnterCriticalSection(&m_navigationLock);
	SetMenuDisplayed(menuDisplayedPad,true);
	bool success = m_groups[static_cast<int>(group)]->NavigateToScene(iPad, scene, initData, layer);
	if(success && group == eUIGroup_Fullscreen) setFullscreenMenuDisplayed(true);
	LeaveCriticalSection(&m_navigationLock);

	timer.PrintElapsedTime(L"Navigate to scene");

	return success;
	//return true;
}

bool UIController::NavigateBack(int iPad, bool forceUsePad, EUIScene eScene, EUILayer eLayer)
{
	bool navComplete = false;
	if( app.GetGameStarted() )
	{
		bool navComplete = m_groups[static_cast<int>(eUIGroup_Fullscreen)]->NavigateBack(iPad, eScene, eLayer);

		if(!navComplete && ( iPad != 255 ) && ( iPad >= 0 ) )
		{
			EUIGroup group = static_cast<EUIGroup>(iPad + 1);
			navComplete = m_groups[static_cast<int>(group)]->NavigateBack(iPad, eScene, eLayer);
			if(!m_groups[static_cast<int>(group)]->GetMenuDisplayed())SetMenuDisplayed(iPad,false);
		}
		// 4J-PB - autosave in fullscreen doesn't clear the menuDisplayed flag
		else
		{
			if(!m_groups[static_cast<int>(eUIGroup_Fullscreen)]->GetMenuDisplayed())
			{
				setFullscreenMenuDisplayed(false);
				for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
				{
					SetMenuDisplayed(i,m_groups[i+1]->GetMenuDisplayed());
				}
			}
		}
	}
	else
	{
		navComplete = m_groups[static_cast<int>(eUIGroup_Fullscreen)]->NavigateBack(iPad, eScene, eLayer);
		if(!m_groups[static_cast<int>(eUIGroup_Fullscreen)]->GetMenuDisplayed()) SetMenuDisplayed(XUSER_INDEX_ANY,false);
	}
	return navComplete;
}

void UIController::NavigateToHomeMenu()
{
	ui.CloseAllPlayersScenes();

	// Alert the app the we no longer want to be informed of ethernet connections
	app.SetLiveLinkRequired( false );

	Minecraft *pMinecraft = Minecraft::GetInstance();

	// 4J-PB - just about to switched to the default texture pack , so clean up anything texture pack related here

	// unload any texture pack audio
	// if there is audio in use, clear out the audio, and unmount the pack
	TexturePack *pTexPack=Minecraft::GetInstance()->skins->getSelected();


	DLCTexturePack *pDLCTexPack=nullptr;
	if(pTexPack->hasAudio())
	{
		// get the dlc texture pack, and store it
		pDLCTexPack=static_cast<DLCTexturePack *>(pTexPack);
	}

	// change to the default texture pack
	pMinecraft->skins->selectTexturePackById(TexturePackRepository::DEFAULT_TEXTURE_PACK_ID);


	if(pTexPack->hasAudio())
	{
		// need to stop the streaming audio - by playing streaming audio from the default texture pack now
		// reset the streaming sounds back to the normal ones
		pMinecraft->soundEngine->SetStreamingSounds(eStream_Overworld_Calm1,eStream_Overworld_piano3,
			eStream_Nether1,eStream_Nether4,
			eStream_end_dragon,eStream_end_end,
			eStream_CD_1, eStream_Overworld_Creative1, eStream_Overworld_Creative6, eStream_Overworld_Menu1, eStream_Overworld_Menu4);
		pMinecraft->soundEngine->playStreaming(L"", 0, 0, 0, 1, 1);

		// 		if(pDLCTexPack->m_pStreamedWaveBank!=nullptr)
		// 		{
		// 			pDLCTexPack->m_pStreamedWaveBank->Destroy();
		// 		}
		// 		if(pDLCTexPack->m_pSoundBank!=nullptr)
		// 		{
		// 			pDLCTexPack->m_pSoundBank->Destroy();
		// 		}
#ifdef _XBOX_ONE
		DWORD result = StorageManager.UnmountInstalledDLC(L"TPACK");
#else
		DWORD result = StorageManager.UnmountInstalledDLC("TPACK");
#endif

		app.DebugPrintf("Unmount result is %d\n",result);
	}

	g_NetworkManager.ForceFriendsSessionRefresh();

	if(pMinecraft->skins->needsUIUpdate())
	{
		m_navigateToHomeOnReload = true;
	}
	else
	{
		ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_MainMenu);
	}
}

UIScene *UIController::GetTopScene(int iPad, EUILayer layer, EUIGroup group)
{
	if(group == eUIGroup_PAD)
	{
		if( app.GetGameStarted() )
		{
			// If the game isn't running treat as user 0, otherwise map index directly from pad
			if( ( iPad != 255 ) && ( iPad >= 0 ) )
			{
				group = static_cast<EUIGroup>(iPad + 1);
			}
			else group = eUIGroup_Fullscreen;
		}
		else
		{
			layer = eUILayer_Fullscreen;
			group = eUIGroup_Fullscreen;
		}
	}
	return m_groups[static_cast<int>(group)]->GetTopScene(layer);
}

size_t UIController::RegisterForCallbackId(UIScene *scene)
{
	EnterCriticalSection(&m_registeredCallbackScenesCS);
	size_t newId = GetTickCount();
	newId &= 0xFFFFFF; // Chop off the top byte, we don't need any more accuracy than that
	newId |= (scene->getSceneType() << 24); // Add in the scene's type to help keep this unique
	m_registeredCallbackScenes[newId] = scene;
	LeaveCriticalSection(&m_registeredCallbackScenesCS);
	return newId;
}

void UIController::UnregisterCallbackId(size_t id)
{
	EnterCriticalSection(&m_registeredCallbackScenesCS);
    auto it = m_registeredCallbackScenes.find(id);
    if(it != m_registeredCallbackScenes.end() )
	{
		m_registeredCallbackScenes.erase(it);
	}
	LeaveCriticalSection(&m_registeredCallbackScenesCS);
}

UIScene *UIController::GetSceneFromCallbackId(size_t id)
{
	UIScene *scene = nullptr;
    auto it = m_registeredCallbackScenes.find(id);
    if(it != m_registeredCallbackScenes.end() )
	{
		scene = it->second;
	}
	return scene;
}

void UIController::EnterCallbackIdCriticalSection()
{
	EnterCriticalSection(&m_registeredCallbackScenesCS);
}

void UIController::LeaveCallbackIdCriticalSection()
{
	LeaveCriticalSection(&m_registeredCallbackScenesCS);
}

void UIController::CloseAllPlayersScenes()
{
	m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getTooltips()->SetTooltips(-1);
	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		//m_bCloseAllScenes[i] = true;
		m_groups[i]->closeAllScenes();
		m_groups[i]->getTooltips()->SetTooltips(-1);
	}

	if (!m_groups[eUIGroup_Fullscreen]->GetMenuDisplayed()) {
		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			SetMenuDisplayed(i,false);
		}
	}
	setFullscreenMenuDisplayed(false);
}

void UIController::CloseUIScenes(int iPad, bool forceIPad)
{
	EUIGroup group;
	if( app.GetGameStarted() || forceIPad )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}

	m_groups[static_cast<int>(group)]->closeAllScenes();
	m_groups[static_cast<int>(group)]->getTooltips()->SetTooltips(-1);

	// This should cause the popup to dissappear
	TutorialPopupInfo popupInfo;
	if(m_groups[static_cast<int>(group)]->getTutorialPopup()) m_groups[static_cast<int>(group)]->getTutorialPopup()->SetTutorialDescription(&popupInfo);

	if(group==eUIGroup_Fullscreen) setFullscreenMenuDisplayed(false);

	SetMenuDisplayed((group == eUIGroup_Fullscreen ? XUSER_INDEX_ANY : iPad), m_groups[static_cast<int>(group)]->GetMenuDisplayed());
}

void UIController::setFullscreenMenuDisplayed(bool displayed)
{
	// Show/hide the tooltips for the fullscreen group
	m_groups[static_cast<int>(eUIGroup_Fullscreen)]->showComponent(ProfileManager.GetPrimaryPad(),eUIComponent_Tooltips,eUILayer_Tooltips,displayed);

	// Show/hide tooltips for the other layers
	for(unsigned int i = (eUIGroup_Fullscreen+1); i < eUIGroup_COUNT; ++i)
	{
		m_groups[i]->showComponent(i,eUIComponent_Tooltips,eUILayer_Tooltips,!displayed);
	}
}

bool UIController::IsPauseMenuDisplayed(int iPad)
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	return m_groups[static_cast<int>(group)]->IsPauseMenuDisplayed();
}

bool UIController::IsContainerMenuDisplayed(int iPad)
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	return m_groups[static_cast<int>(group)]->IsContainerMenuDisplayed();
}

bool UIController::IsIgnorePlayerJoinMenuDisplayed(int iPad)
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	return m_groups[static_cast<int>(group)]->IsIgnorePlayerJoinMenuDisplayed();
}

bool UIController::IsIgnoreAutosaveMenuDisplayed(int iPad)
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	return m_groups[static_cast<int>(eUIGroup_Fullscreen)]->IsIgnoreAutosaveMenuDisplayed() || (group != eUIGroup_Fullscreen && m_groups[static_cast<int>(group)]->IsIgnoreAutosaveMenuDisplayed());
}

void UIController::SetIgnoreAutosaveMenuDisplayed(int iPad, bool displayed)
{
	app.DebugPrintf(app.USER_SR, "UIController::SetIgnoreAutosaveMenuDisplayed is not implemented\n");
}

bool UIController::IsSceneInStack(int iPad, EUIScene eScene)
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	return m_groups[static_cast<int>(group)]->IsSceneInStack(eScene);
}

bool UIController::GetMenuDisplayed(int iPad)
{
	return m_bMenuDisplayed[iPad];
}

void UIController::SetMenuDisplayed(int iPad,bool bVal)
{
	if(bVal)
	{
		if(iPad==XUSER_INDEX_ANY)
		{
			for(int i=0;i<XUSER_MAX_COUNT;i++)
			{
				InputManager.SetMenuDisplayed(i,true);
				m_bMenuDisplayed[i]=true;
				// 4J Stu - Fix for #11018 - Functional: When the controller is unplugged during active gameplay and plugged back in at the resulting pause menu, it will demonstrate dual-functionality.
				m_bMenuToBeClosed[i]=false;
			}
		}
		else
		{
			InputManager.SetMenuDisplayed(iPad,true);
			m_bMenuDisplayed[iPad]=true;
			// 4J Stu - Fix for #11018 - Functional: When the controller is unplugged during active gameplay and plugged back in at the resulting pause menu, it will demonstrate dual-functionality.
			m_bMenuToBeClosed[iPad]=false;
		}
	}
	else
	{
		if(iPad==XUSER_INDEX_ANY)
		{
			for(int i=0;i<XUSER_MAX_COUNT;i++)
			{
				m_bMenuToBeClosed[i]=true;
				m_iCountDown[i]=10;
			}
		}
		else
		{
			m_bMenuToBeClosed[iPad]=true;
			m_iCountDown[iPad]=10;

#ifdef _DURANGO
			// 4J-JEV: When in-game, allow player to toggle the 'Pause' and 'IngameInfo' menus via Kinnect.
			if (Minecraft::GetInstance()->running)
				InputManager.SetEnabledGtcButtons(_360_GTC_MENU | _360_GTC_PAUSE | _360_GTC_VIEW);
#endif
		}
	}
}

void UIController::CheckMenuDisplayed()
{
	for(int iPad=0;iPad<XUSER_MAX_COUNT;iPad++)
	{
		if(m_bMenuToBeClosed[iPad])
		{
			if(m_iCountDown[iPad]!=0)
			{
				m_iCountDown[iPad]--;
			}
			else
			{
				m_bMenuToBeClosed[iPad]=false;
				m_bMenuDisplayed[iPad]=false;
				InputManager.SetMenuDisplayed(iPad,false);
			}

		}
	}
}

void UIController::SetTooltipText( unsigned int iPad, unsigned int tooltip, int iTextID )
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	if(m_groups[static_cast<int>(group)]->getTooltips()) m_groups[static_cast<int>(group)]->getTooltips()->SetTooltipText(tooltip, iTextID);
}

void UIController::SetEnableTooltips( unsigned int iPad, BOOL bVal )
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	if(m_groups[static_cast<int>(group)]->getTooltips()) m_groups[static_cast<int>(group)]->getTooltips()->SetEnableTooltips(bVal);
}

void UIController::ShowTooltip( unsigned int iPad, unsigned int tooltip, bool show )
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	if(m_groups[static_cast<int>(group)]->getTooltips()) m_groups[static_cast<int>(group)]->getTooltips()->ShowTooltip(tooltip,show);
}

void UIController::SetTooltips( unsigned int iPad, int iA, int iB, int iX, int iY, int iLT, int iRT, int iLB, int iRB, int iLS, int iRS, int iBack, bool forceUpdate)
{
	EUIGroup group;

	// 4J-PB - strip out any that are not applicable on the platform
#ifndef _XBOX
	if(iX==IDS_TOOLTIPS_SELECTDEVICE) iX=-1;
	if(iX==IDS_TOOLTIPS_CHANGEDEVICE) iX=-1;

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	if(iY==IDS_TOOLTIPS_VIEW_GAMERCARD) iY=-1;
	if(iY==IDS_TOOLTIPS_VIEW_GAMERPROFILE) iY=-1;

#endif
#endif

	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	if(m_groups[static_cast<int>(group)]->getTooltips()) m_groups[static_cast<int>(group)]->getTooltips()->SetTooltips(iA, iB, iX, iY, iLT, iRT, iLB, iRB, iLS, iRS, iBack, forceUpdate);
}

void UIController::EnableTooltip( unsigned int iPad, unsigned int tooltip, bool enable )
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	if(m_groups[static_cast<int>(group)]->getTooltips()) m_groups[static_cast<int>(group)]->getTooltips()->EnableTooltip(tooltip,enable);
}

void UIController::RefreshTooltips(unsigned int iPad)
{
	app.DebugPrintf(app.USER_SR, "UIController::RefreshTooltips is not implemented\n");
}

void UIController::AnimateKeyPress(int iPad, int iAction, bool bRepeat, bool bPressed, bool bReleased)
{
	EUIGroup group;
	if(bPressed==false)
	{
		// only animating button press
		return;
	}
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	bool handled = false;
	if(m_groups[static_cast<int>(group)]->getTooltips()) m_groups[static_cast<int>(group)]->getTooltips()->handleInput(iPad, iAction, bRepeat, bPressed, bReleased, handled);
}

void UIController::OverrideSFX(int iPad, int iAction,bool bVal)
{
	EUIGroup group;

	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	bool handled = false;
	if(m_groups[static_cast<int>(group)]->getTooltips()) m_groups[static_cast<int>(group)]->getTooltips()->overrideSFX(iPad, iAction,bVal);
}

void UIController::PlayUISFX(ESoundEffect eSound)
{
	uint64_t time = System::currentTimeMillis();

	// Don't play multiple SFX on the same tick
	// (prevents horrible sounds when programmatically setting multiple checkboxes)
	if (time - m_lastUiSfx < 10) { return; }
	m_lastUiSfx = time;

	float pitch = 1.0f;
	if (eSound == eSFX_Focus)
	{
		pitch += (m_randomDistribution(m_randomGenerator) - 0.5f) / 10;
	}

	Minecraft::GetInstance()->soundEngine->playUI(eSound,1.0f,pitch);
}

void UIController::DisplayGamertag(unsigned int iPad, bool show)
{
	// The host decides whether these are on or off
	if( app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_DisplaySplitscreenGamertags) == 0)
	{
		show = false;
	}
	EUIGroup group = static_cast<EUIGroup>(iPad + 1);
	if(m_groups[static_cast<int>(group)]->getHUD()) m_groups[static_cast<int>(group)]->getHUD()->ShowDisplayName(show);

	// Update TutorialPopup in Splitscreen if no container is displayed (to make sure the Popup does not overlap with the Gamertag!)
	if(app.GetLocalPlayerCount() > 1 && m_groups[static_cast<int>(group)]->getTutorialPopup() && !m_groups[static_cast<int>(group)]->IsContainerMenuDisplayed())
	{
		m_groups[static_cast<int>(group)]->getTutorialPopup()->UpdateTutorialPopup();
	}
}

void UIController::SetSelectedItem(unsigned int iPad, const wstring &name)
{
	EUIGroup group;

	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	bool handled = false;
	if(m_groups[static_cast<int>(group)]->getHUD()) m_groups[static_cast<int>(group)]->getHUD()->SetSelectedLabel(name);
}

void UIController::UpdateSelectedItemPos(unsigned int iPad)
{
	app.DebugPrintf(app.USER_SR, "UIController::UpdateSelectedItemPos not implemented\n");
}

void UIController::HandleDLCMountingComplete()
{
	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		app.DebugPrintf("UIController::HandleDLCMountingComplete - m_groups[%d]\n",i);
		m_groups[i]->HandleDLCMountingComplete();
	}
}

void UIController::HandleDLCInstalled(int iPad)
{
	//app.DebugPrintf(app.USER_SR, "UIController::HandleDLCInstalled not implemented\n");
	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		m_groups[i]->HandleDLCInstalled();
	}
}


#ifdef _XBOX_ONE
void UIController::HandleDLCLicenseChange()
{
	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		app.DebugPrintf("UIController::HandleDLCLicenseChange - m_groups[%d]\n",i);
		m_groups[i]->HandleDLCLicenseChange();
	}
}
#endif

void UIController::HandleTMSDLCFileRetrieved(int iPad)
{
	app.DebugPrintf(app.USER_SR, "UIController::HandleTMSDLCFileRetrieved not implemented\n");
}

void UIController::HandleTMSBanFileRetrieved(int iPad)
{
	app.DebugPrintf(app.USER_SR, "UIController::HandleTMSBanFileRetrieved not implemented\n");
}

void UIController::HandleInventoryUpdated(int iPad)
{
	EUIGroup group = eUIGroup_Fullscreen;
	if( app.GetGameStarted() && ( iPad != 255 ) && ( iPad >= 0 ) )
	{
		group = static_cast<EUIGroup>(iPad + 1);
	}

	m_groups[group]->HandleMessage(eUIMessage_InventoryUpdated, nullptr);
}

void UIController::HandleGameTick()
{
	tickInput();

	for(unsigned int i = 0; i < eUIGroup_COUNT; ++i)
	{
		if(m_groups[i]->getHUD()) m_groups[i]->getHUD()->handleGameTick();
	}
}

void UIController::SetTutorial(int iPad, Tutorial *tutorial)
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	if(m_groups[static_cast<int>(group)]->getTutorialPopup()) m_groups[static_cast<int>(group)]->getTutorialPopup()->SetTutorial(tutorial);
}

void UIController::SetTutorialDescription(int iPad, TutorialPopupInfo *info)
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}

	if(m_groups[static_cast<int>(group)]->getTutorialPopup())
	{
		// tutorial popup needs to know if a container menu is being displayed
		m_groups[static_cast<int>(group)]->getTutorialPopup()->SetContainerMenuVisible(m_groups[static_cast<int>(group)]->IsContainerMenuDisplayed());
		m_groups[static_cast<int>(group)]->getTutorialPopup()->SetTutorialDescription(info);
	}
}

#ifndef _XBOX
void UIController::RemoveInteractSceneReference(int iPad, UIScene *scene)
{
	EUIGroup group;
	if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
	else group = eUIGroup_Fullscreen;
	if(m_groups[static_cast<int>(group)]->getTutorialPopup()) m_groups[static_cast<int>(group)]->getTutorialPopup()->RemoveInteractSceneReference(scene);
}
#endif

void UIController::SetTutorialVisible(int iPad, bool visible)
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	if(m_groups[static_cast<int>(group)]->getTutorialPopup()) m_groups[static_cast<int>(group)]->getTutorialPopup()->SetVisible(visible);
}

bool UIController::IsTutorialVisible(int iPad)
{
	EUIGroup group;
	if( app.GetGameStarted() )
	{
		// If the game isn't running treat as user 0, otherwise map index directly from pad
		if( ( iPad != 255 ) && ( iPad >= 0 ) ) group = static_cast<EUIGroup>(iPad + 1);
		else group = eUIGroup_Fullscreen;
	}
	else
	{
		group = eUIGroup_Fullscreen;
	}
	bool visible = false;
	if(m_groups[static_cast<int>(group)]->getTutorialPopup()) visible = m_groups[static_cast<int>(group)]->getTutorialPopup()->IsVisible();
	return visible;
}

void UIController::UpdatePlayerBasePositions()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

	for( BYTE idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		if(pMinecraft->localplayers[idx] != nullptr)
		{
			if(pMinecraft->localplayers[idx]->m_iScreenSection==C4JRender::VIEWPORT_TYPE_FULLSCREEN)
			{
				DisplayGamertag(idx,false);
			}
			else
			{
				DisplayGamertag(idx,true);
			}
			m_groups[idx+1]->SetViewportType(static_cast<C4JRender::eViewportType>(pMinecraft->localplayers[idx]->m_iScreenSection));
		}
		else
		{
			// 4J Stu - This is a legacy thing from our XUI implementation that we don't need
			// Changing the viewport to fullscreen for users that no longer exist is SLOW
			// This should probably be on all platforms, but I don't have time to test them all just now!
#ifndef __ORBIS__
			m_groups[idx+1]->SetViewportType(C4JRender::VIEWPORT_TYPE_FULLSCREEN);
#endif
			DisplayGamertag(idx,false);
		}
	}
}

void UIController::SetEmptyQuadrantLogo(int iSection)
{
	// 4J Stu - We shouldn't need to implement this
}

void UIController::HideAllGameUIElements()
{
	// 4J Stu - We might not need to implement this
	app.DebugPrintf(app.USER_SR, "UIController::HideAllGameUIElements not implemented\n");
}

void UIController::ShowOtherPlayersBaseScene(unsigned int iPad, bool show)
{
	// 4J Stu - We shouldn't need to implement this
}

void UIController::ShowTrialTimer(bool show)
{
	if(m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()) m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()->showTrialTimer(show);
}

void UIController::SetTrialTimerLimitSecs(unsigned int uiSeconds)
{
	UIController::m_dwTrialTimerLimitSecs = uiSeconds;
}

void UIController::UpdateTrialTimer(unsigned int iPad)
{
	WCHAR wcTime[20];

	DWORD dwTimeTicks=static_cast<DWORD>(app.getTrialTimer());

	if(dwTimeTicks>m_dwTrialTimerLimitSecs)
	{
		dwTimeTicks=m_dwTrialTimerLimitSecs;
	}

	dwTimeTicks=m_dwTrialTimerLimitSecs-dwTimeTicks;

#ifndef _CONTENT_PACKAGE
	if(true)
#else
	// display the time - only if there's less than 3 minutes
	if(dwTimeTicks<180)
#endif
	{
		int iMins=dwTimeTicks/60;
		int iSeconds=dwTimeTicks%60;
		swprintf( wcTime, 20, L"%d:%02d",iMins,iSeconds);
		if(m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()) m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()->setTrialTimer(wcTime);
	}
	else
	{
		if(m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()) m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()->setTrialTimer(L"");
	}

	// are we out of time?
	if((dwTimeTicks==0))
	{
		// Trial over
		// bring up the pause menu to stop the trial over message box being called again?
		if(!ui.GetMenuDisplayed( iPad ) )
		{
			ui.NavigateToScene(iPad, eUIScene_PauseMenu, nullptr, eUILayer_Scene);

			app.SetAction(iPad,eAppAction_TrialOver);
		}
	}
}

void UIController::ReduceTrialTimerValue()
{
	DWORD dwTimeTicks=static_cast<int>(app.getTrialTimer());

	if(dwTimeTicks>m_dwTrialTimerLimitSecs)
	{
		dwTimeTicks=m_dwTrialTimerLimitSecs;
	}

	m_dwTrialTimerLimitSecs-=dwTimeTicks;
}

void UIController::ShowAutosaveCountdownTimer(bool show)
{
	if(m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()) m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()->showTrialTimer(show);
}

void UIController::UpdateAutosaveCountdownTimer(unsigned int uiSeconds)
{
#if !(defined(_XBOX_ONE) || defined(__ORBIS__))
	WCHAR wcAutosaveCountdown[100];
	swprintf( wcAutosaveCountdown, 100, app.GetString(IDS_AUTOSAVE_COUNTDOWN),uiSeconds);
	if(m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()) m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()->setTrialTimer(wcAutosaveCountdown);
#endif
}

void UIController::ShowSavingMessage(unsigned int iPad, C4JStorage::ESavingMessage eVal)
{
	bool show = false;
	switch(eVal)
	{
	case C4JStorage::ESavingMessage_None:
		show = false;
		break;
	case C4JStorage::ESavingMessage_Short:
	case C4JStorage::ESavingMessage_Long:
		show = true;
		break;
	}
	if(m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()) m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()->showSaveIcon(show);
}

void UIController::ShowPlayerDisplayname(bool show)
{
	if(m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()) m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()->showPlayerDisplayName(show);
}

void UIController::SetWinUserIndex(unsigned int iPad)
{
	m_winUserIndex = iPad;
}

unsigned int UIController::GetWinUserIndex()
{
	return m_winUserIndex;
}

void UIController::ShowUIDebugConsole(bool show)
{
#ifndef _CONTENT_PACKAGE

	if(show)
	{
		m_uiDebugConsole = static_cast<UIComponent_DebugUIConsole *>(m_groups[eUIGroup_Fullscreen]->addComponent(0, eUIComponent_DebugUIConsole, eUILayer_Debug));
	}
	else
	{
		m_groups[eUIGroup_Fullscreen]->removeComponent(eUIComponent_DebugUIConsole, eUILayer_Debug);
		m_uiDebugConsole = nullptr;
	}
#endif
}

void UIController::ShowUIDebugMarketingGuide(bool show)
{
#ifndef _CONTENT_PACKAGE

	if(show)
	{
		m_uiDebugMarketingGuide = static_cast<UIComponent_DebugUIMarketingGuide *>(m_groups[eUIGroup_Fullscreen]->addComponent(0, eUIComponent_DebugUIMarketingGuide, eUILayer_Debug));
	}
	else
	{
		m_groups[eUIGroup_Fullscreen]->removeComponent(eUIComponent_DebugUIMarketingGuide, eUILayer_Debug);
		m_uiDebugMarketingGuide = nullptr;
	}
#endif
}

void UIController::logDebugString(const string &text)
{
	if(m_uiDebugConsole) m_uiDebugConsole->addText(text);
}

bool UIController::PressStartPlaying(unsigned int iPad)
{
	return m_iPressStartQuadrantsMask&(1<<iPad)?true:false;
}

void UIController::ShowPressStart(unsigned int iPad)
{
	m_iPressStartQuadrantsMask|=1<<iPad;
	if(m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()) m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()->showPressStart(iPad, true);
}

void UIController::HidePressStart()
{
	ClearPressStart();
	if(m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()) m_groups[static_cast<int>(eUIGroup_Fullscreen)]->getPressStartToPlay()->showPressStart(0, false);
}

void UIController::ClearPressStart()
{
	m_iPressStartQuadrantsMask = 0;
}

C4JStorage::EMessageResult UIController::RequestAlertMessage(UINT uiTitle, UINT uiText, UINT *uiOptionA,UINT uiOptionC, DWORD dwPad, int( *Func)(LPVOID,int,const C4JStorage::EMessageResult),LPVOID lpParam, WCHAR *pwchFormatString)
{
	return RequestMessageBox(uiTitle, uiText, uiOptionA, uiOptionC, dwPad, Func, lpParam, pwchFormatString, 0, false);
}

C4JStorage::EMessageResult UIController::RequestErrorMessage(UINT uiTitle, UINT uiText, UINT *uiOptionA,UINT uiOptionC, DWORD dwPad, int( *Func)(LPVOID,int,const C4JStorage::EMessageResult),LPVOID lpParam, WCHAR *pwchFormatString)
{
	return RequestMessageBox(uiTitle, uiText, uiOptionA, uiOptionC, dwPad, Func, lpParam, pwchFormatString, 0, true);
}

C4JStorage::EMessageResult UIController::RequestMessageBox(UINT uiTitle, UINT uiText, UINT *uiOptionA,UINT uiOptionC, DWORD dwPad,
														   int( *Func)(LPVOID,int,const C4JStorage::EMessageResult),LPVOID lpParam, WCHAR *pwchFormatString,DWORD dwFocusButton, bool bIsError)

{
	MessageBoxInfo param;
	param.uiTitle = uiTitle;
	param.uiText = uiText;
	param.uiOptionA = uiOptionA;
	param.uiOptionC = uiOptionC;
	param.dwPad = dwPad;
	param.Func = Func;
	param.lpParam = lpParam;
	param.pwchFormatString = pwchFormatString;
	param.dwFocusButton = dwFocusButton;

	EUILayer layer = bIsError?eUILayer_Error:eUILayer_Alert;

	bool completed = false;
	if(ui.IsReloadingSkin())
	{
		// Queue this message box
		QueuedMessageBoxData *queuedData = new QueuedMessageBoxData();
		queuedData->info = param;
		queuedData->info.uiOptionA = new UINT[param.uiOptionC];
		memcpy(queuedData->info.uiOptionA, param.uiOptionA, param.uiOptionC * sizeof(UINT));
		queuedData->iPad = dwPad;
		queuedData->layer = eUILayer_Error; // Ensures that these don't get wiped out by a CloseAllScenes call
		m_queuedMessageBoxData.push_back(queuedData);
	}
	else
	{
		completed = ui.NavigateToScene(dwPad, eUIScene_MessageBox, &param, layer, eUIGroup_Fullscreen);
	}

	if( completed )
	{
		// This may happen if we had to queue the message box, or there was already a message box displaying and so the NavigateToScene returned false;
		return C4JStorage::EMessage_Pending;
	}
	else
	{
		return C4JStorage::EMessage_Busy;
	}
}

C4JStorage::EMessageResult UIController::RequestUGCMessageBox(UINT title/* = -1 */, UINT message/* = -1 */, int iPad/* = -1*/, int( *Func)(LPVOID,int,const C4JStorage::EMessageResult)/* = nullptr*/, LPVOID lpParam/* = nullptr*/)
{
	// Default title / messages
	if (title == -1)
	{
		title = IDS_FAILED_TO_CREATE_GAME_TITLE;
	}

	if (message == -1)
	{
		message = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE;
	}

	// Default pad to primary player
	if (iPad == -1) iPad = ProfileManager.GetPrimaryPad();

#ifdef __ORBIS__
	// Show the vague UGC system message in addition to our message
	ProfileManager.DisplaySystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_PSN_UGC_RESTRICTION, iPad );
	return C4JStorage::EMessage_ResultAccept;
#elif defined(__PSVITA__)
	ProfileManager.ShowSystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_PSN_CHAT_RESTRICTION, iPad );
	UINT uiIDA[1];
	uiIDA[0]=IDS_CONFIRM_OK;
	return ui.RequestAlertMessage( title, IDS_CHAT_RESTRICTION_UGC, uiIDA, 1, iPad, Func, lpParam);
#else
	UINT uiIDA[1];
	uiIDA[0]=IDS_CONFIRM_OK;
	return ui.RequestAlertMessage( title, message, uiIDA, 1, iPad, Func, lpParam);
#endif
}

C4JStorage::EMessageResult UIController::RequestContentRestrictedMessageBox(UINT title/* = -1 */, UINT message/* = -1 */, int iPad/* = -1*/, int( *Func)(LPVOID,int,const C4JStorage::EMessageResult)/* = nullptr*/, LPVOID lpParam/* = nullptr*/)
{
	// Default title / messages
	if (title == -1)
	{
		title = IDS_FAILED_TO_CREATE_GAME_TITLE;
	}

	if (message == -1)
	{
#if defined(_XBOX_ONE) || defined(_WINDOWS64)
		// IDS_CONTENT_RESTRICTION doesn't exist on XB1
		message = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE;
#else
		message = IDS_CONTENT_RESTRICTION;
#endif
	}

	// Default pad to primary player
	if (iPad == -1) iPad = ProfileManager.GetPrimaryPad();

#ifdef __ORBIS__
	// Show the vague UGC system message in addition to our message
	ProfileManager.DisplaySystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_PSN_UGC_RESTRICTION, iPad );
	return C4JStorage::EMessage_ResultAccept;
#elif defined(__PSVITA__)
	ProfileManager.ShowSystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_PSN_AGE_RESTRICTION, iPad );
	return C4JStorage::EMessage_ResultAccept;
#else
	UINT uiIDA[1];
	uiIDA[0]=IDS_CONFIRM_OK;
	return ui.RequestAlertMessage( title, message, uiIDA, 1, iPad, Func, lpParam);
#endif
}

void UIController::setFontCachingCalculationBuffer(int length)
{
	/* 4J-JEV: As described in an email from Sean.
	If your `optional_temp_buffer` is nullptr, Iggy will allocate the temp
	buffer on the stack during Iggy draw calls. The size of the buffer it
	will allocate is 16 bytes times `max_chars` in 32-bit, and 24 bytes
	times `max_chars` in 64-bit. If the stack of the thread making the
	draw call is not large enough, Iggy will crash or otherwise behave
	incorrectly.
	*/
#if defined __ORBIS__ || defined _DURANGO || defined _WIN64
	static const int CHAR_SIZE = 24;
#else
	static const int CHAR_SIZE = 16;
#endif

	if (m_tempBuffer != nullptr) delete [] m_tempBuffer;
	if (length<0)
	{
		if (m_defaultBuffer == nullptr) m_defaultBuffer = new char[CHAR_SIZE*5000];
		IggySetFontCachingCalculationBuffer(5000, m_defaultBuffer, CHAR_SIZE*5000);
	}
	else
	{
		m_tempBuffer = new char[CHAR_SIZE*length];
		IggySetFontCachingCalculationBuffer(length, m_tempBuffer, CHAR_SIZE*length);
	}
}

// Returns the first scene of given type if it exists, nullptr otherwise
UIScene *UIController::FindScene(EUIScene sceneType)
{
	UIScene *pScene = nullptr;

	for (int i = 0; i < eUIGroup_COUNT; i++)
	{
		pScene = m_groups[i]->FindScene(sceneType);
#ifdef __PS3__
		if (pScene != nullptr) return pScene;
#else
		if (pScene != nullptr) return pScene;
#endif
	}

	return pScene;
}

#ifdef __PSVITA__

void UIController::TouchBoxAdd(UIControl *pControl,UIScene *pUIScene)
{
	EUIGroup eUIGroup=pUIScene->GetParentLayerGroup();
	EUILayer eUILayer=pUIScene->GetParentLayer()->m_iLayer;
	EUIScene eUIscene=pUIScene->getSceneType();

	TouchBoxAdd(pControl,eUIGroup,eUILayer,eUIscene, pUIScene->GetMainPanel());
}

void UIController::TouchBoxAdd(UIControl *pControl,EUIGroup eUIGroup,EUILayer eUILayer,EUIScene eUIscene, UIControl *pMainPanelControl)
{
	UIELEMENT *puiElement = new UIELEMENT;
	puiElement->pControl = pControl;

	S32 iControlWidth = pControl->getWidth();
	S32 iControlHeight = pControl->getHeight();
	S32 iMainPanelOffsetX = 0;
	S32 iMainPanelOffsetY= 0;

	// 4J-TomK add main panel offset if controls do not live in the root scene
	if(pMainPanelControl)
	{
		iMainPanelOffsetX = pMainPanelControl->getXPos();
		iMainPanelOffsetY = pMainPanelControl->getYPos();
	}

	// 4J-TomK override control width / height where needed
	if(puiElement->pControl->getControlType() == UIControl::eSlider)
	{
		// Sliders are never scaled but masked, so we have to get the real width from AS
		UIControl_Slider *pSlider = (UIControl_Slider *)puiElement->pControl;
		iControlWidth = pSlider->GetRealWidth();
	}
	else if(puiElement->pControl->getControlType() == UIControl::eTexturePackList)
	{
		// The origin of the TexturePackList is NOT in the top left corner but where the slot area starts. therefore we need the height of the slot area itself.
		UIControl_TexturePackList *pTexturePackList = (UIControl_TexturePackList *)puiElement->pControl;
		iControlHeight = pTexturePackList->GetRealHeight();
	}
	else if(puiElement->pControl->getControlType() == UIControl::eDynamicLabel)
	{
		// The height and width of this control changes per how to play page
		UIControl_DynamicLabel *pDynamicLabel = (UIControl_DynamicLabel *)puiElement->pControl;
		iControlWidth = pDynamicLabel->GetRealWidth();
		iControlHeight = pDynamicLabel->GetRealHeight();
	}
	else if(puiElement->pControl->getControlType() == UIControl::eHTMLLabel)
	{
		// The height and width of this control changes per how to play page
		UIControl_HTMLLabel *pHtmlLabel = (UIControl_HTMLLabel *)puiElement->pControl;
		iControlWidth = pHtmlLabel->GetRealWidth();
		iControlHeight = pHtmlLabel->GetRealHeight();
	}

	puiElement->x1=(S32)((float)pControl->getXPos() + (float)iMainPanelOffsetX);
	puiElement->y1=(S32)((float)pControl->getYPos() + (float)iMainPanelOffsetY);
	puiElement->x2=(S32)(((float)pControl->getXPos() + (float)iControlWidth + (float)iMainPanelOffsetX));
	puiElement->y2=(S32)(((float)pControl->getYPos() + (float)iControlHeight + (float)iMainPanelOffsetY));

	if(puiElement->pControl->getControlType() == UIControl::eNoControl)
	{
		app.DebugPrintf("NO CONTROL!");
	}

	if(puiElement->x1 == puiElement->x2 || puiElement->y1 == puiElement->y2)
	{
		app.DebugPrintf("NOT adding touchbox %d,%d,%d,%d\n",puiElement->x1,puiElement->y1,puiElement->x2,puiElement->y2);
	}
	else
	{
		app.DebugPrintf("Adding touchbox %d,%d,%d,%d\n",puiElement->x1,puiElement->y1,puiElement->x2,puiElement->y2);
		m_TouchBoxes[eUIGroup][eUILayer][eUIscene].push_back(puiElement);
	}
}

void UIController::TouchBoxRebuild(UIScene *pUIScene)
{
	EUIGroup eUIGroup=pUIScene->GetParentLayerGroup();
	EUILayer eUILayer=pUIScene->GetParentLayer()->m_iLayer;
	EUIScene eUIscene=pUIScene->getSceneType();

	// if we delete an element, it's possible that the scene has re-arranged all the elements, so we need to rebuild the boxes
	ui.TouchBoxesClear(pUIScene);

	// rebuild boxes
	for ( UIControl *control : *pUIScene->GetControls() )
	{
		if(control->getControlType() == UIControl::eButton ||
			control->getControlType() == UIControl::eSlider ||
			control->getControlType() == UIControl::eCheckBox ||
			control->getControlType() == UIControl::eTexturePackList ||
			control->getControlType() == UIControl::eButtonList ||
			control->getControlType() == UIControl::eTextInput ||
			control->getControlType() == UIControl::eDynamicLabel ||
			control->getControlType() == UIControl::eHTMLLabel ||
			control->getControlType() == UIControl::eLeaderboardList ||
			control->getControlType() == UIControl::eTouchControl)
		{
			if(control->getVisible())
			{
				// 4J-TomK update the control (it might have been moved by flash / AS)
				control->UpdateControl();

				ui.TouchBoxAdd(control,eUIGroup,eUILayer,eUIscene, pUIScene->GetMainPanel());
			}
		}
	}
}

void UIController::TouchBoxesClear(UIScene *pUIScene)
{
	EUIGroup eUIGroup=pUIScene->GetParentLayerGroup();
	EUILayer eUILayer=pUIScene->GetParentLayer()->m_iLayer;
	EUIScene eUIscene=pUIScene->getSceneType();

	for ( UIELEMENT *element : m_TouchBoxes[eUIGroup][eUILayer][eUIscene] )
	{
		delete element;
	}
	m_TouchBoxes[eUIGroup][eUILayer][eUIscene].clear();
}

bool UIController::TouchBoxHit(UIScene *pUIScene,S32 x, S32 y)
{
	EUIGroup eUIGroup=pUIScene->GetParentLayerGroup();
	EUILayer eUILayer=pUIScene->GetParentLayer()->m_iLayer;
	EUIScene eUIscene=pUIScene->getSceneType();

	// 4J-TomK let's do the transformation from touch resolution to screen resolution here, so our touchbox values always are in screen resolution!
	x *= (m_fScreenWidth/1920.0f);
	y *= (m_fScreenHeight/1080.0f);

	if(m_TouchBoxes[eUIGroup][eUILayer][eUIscene].size()>0)
	{
		for ( UIELEMENT *element : m_TouchBoxes[eUIGroup][eUILayer][eUIscene] )
		{
			if(element->pControl->getHidden() == false && element->pControl->getVisible()) // ignore removed controls
			{
				if((x>=element->x1) &&(x<=element->x2) && (y>=element->y1) && (y<=element->y2))
				{
					if(!m_bTouchscreenPressed)
					{
						app.DebugPrintf("SET m_ActiveUIElement (Layer: %i) at x = %i y = %i\n", (int)eUILayer, (int)x, (int)y);
						m_ActiveUIElement = element;
					}
					// remember the currently highlighted element
					m_HighlightedUIElement = element;

					return true;
				}
			}
		}
	}

	//app.DebugPrintf("MISS at x = %i y = %i\n", (int)x, (int)y);
	m_HighlightedUIElement = nullptr;
	return false;
}

//
// Handle Touch Input
//
void UIController::HandleTouchInput(unsigned int iPad, unsigned int key, bool bPressed, bool bRepeat, bool bReleased)
{
	// no input? no handling!
	if(!bPressed && !bRepeat && !bReleased)
	{
		// override for instand repeat without delay!
		if(m_bTouchscreenPressed && m_ActiveUIElement && (
			m_ActiveUIElement->pControl->getControlType() == UIControl::eSlider ||
			m_ActiveUIElement->pControl->getControlType() == UIControl::eButtonList ||
			m_ActiveUIElement->pControl->getControlType() == UIControl::eTexturePackList ||
			m_ActiveUIElement->pControl->getControlType() == UIControl::eDynamicLabel ||
			m_ActiveUIElement->pControl->getControlType() == UIControl::eHTMLLabel ||
			m_ActiveUIElement->pControl->getControlType() == UIControl::eLeaderboardList ||
			m_ActiveUIElement->pControl->getControlType() == UIControl::eTouchControl))
			bRepeat = true;	// the above controls need to be controllable without having the finger over them
		else
			return;
	}

	SceTouchData* pTouchData = InputManager.GetTouchPadData(iPad,false);
	S32 x = pTouchData->report[0].x * (m_fScreenWidth/1920.0f);
	S32 y = pTouchData->report[0].y * (m_fScreenHeight/1080.0f);

	if(bPressed && !bRepeat && !bReleased)	// PRESSED HANDLING
	{
		app.DebugPrintf("touch input pressed\n");
		switch(m_ActiveUIElement->pControl->getControlType())
		{
		case UIControl::eButton:
			// set focus
			UIControl_Button *pButton=(UIControl_Button *)m_ActiveUIElement->pControl;
			pButton->getParentScene()->SetFocusToElement(m_ActiveUIElement->pControl->getId());
			// override bPressed to false. we only want the button to trigger on touch release!
			bPressed = false;
			break;
		case UIControl::eSlider:
			// set focus
			UIControl_Slider *pSlider=(UIControl_Slider *)m_ActiveUIElement->pControl;
			pSlider->getParentScene()->SetFocusToElement(m_ActiveUIElement->pControl->getId());
			break;
		case UIControl::eCheckBox:
			// set focus
			UIControl_CheckBox *pCheckbox=(UIControl_CheckBox *)m_ActiveUIElement->pControl;
			pCheckbox->getParentScene()->SetFocusToElement(m_ActiveUIElement->pControl->getId());
			// override bPressed. we only want the checkbox to trigger on touch release!
			bPressed = false;
			break;
		case UIControl::eButtonList:
			// set focus to list
			UIControl_ButtonList *pButtonList=(UIControl_ButtonList *)m_ActiveUIElement->pControl;
			//pButtonList->getParentScene()->SetFocusToElement(m_ActiveUIElement->pControl->getId());
			// tell list where we tapped it so it can set focus to the correct button
			pButtonList->SetTouchFocus((float)x, (float)y, false);
			// override bPressed. we only want the ButtonList to trigger on touch release!
			bPressed = false;
			break;
		case UIControl::eTexturePackList:
			// set focus to list
			UIControl_TexturePackList *pTexturePackList=(UIControl_TexturePackList *)m_ActiveUIElement->pControl;
			pTexturePackList->getParentScene()->SetFocusToElement(m_ActiveUIElement->pControl->getId());
			// tell list where we tapped it so it can set focus to the correct texture pack
			pTexturePackList->SetTouchFocus((float)x - (float)m_ActiveUIElement->x1, (float)y - (float)m_ActiveUIElement->y1, false);
			// override bPressed. we only want the TexturePack List to trigger on touch release!
			bPressed = false;
			break;
		case UIControl::eTextInput:
			// set focus
			UIControl_TextInput *pTextInput=(UIControl_TextInput *)m_ActiveUIElement->pControl;
			pTextInput->getParentScene()->SetFocusToElement(m_ActiveUIElement->pControl->getId());
			// override bPressed to false. we only want the textinput to trigger on touch release!
			bPressed = false;
			break;
		case UIControl::eDynamicLabel:
			// handle dynamic label scrolling
			UIControl_DynamicLabel *pDynamicLabel=(UIControl_DynamicLabel *)m_ActiveUIElement->pControl;
			pDynamicLabel->TouchScroll(y, true);
			// override bPressed to false
			bPressed = false;
			break;
		case UIControl::eHTMLLabel:
			// handle dynamic label scrolling
			UIControl_HTMLLabel *pHtmlLabel=(UIControl_HTMLLabel *)m_ActiveUIElement->pControl;
			pHtmlLabel->TouchScroll(y, true);
			// override bPressed to false
			bPressed = false;
			break;
		case UIControl::eLeaderboardList:
			// set focus to list
			UIControl_LeaderboardList *pLeaderboardList=(UIControl_LeaderboardList *)m_ActiveUIElement->pControl;
			// tell list where we tapped it so it can set focus to the correct button
			pLeaderboardList->SetTouchFocus((float)x, (float)y, false);
			// override bPressed. we only want the ButtonList to trigger on touch release!
			bPressed = false;
			break;
		case UIControl::eTouchControl:
			// pass on touch input to relevant parent scene so we can handle it there!
			m_ActiveUIElement->pControl->getParentScene()->handleTouchInput(iPad, x, y, m_ActiveUIElement->pControl->getId(), bPressed, bRepeat, bReleased);
			// override bPressed to false
			bPressed = false;
			break;
		default:
			app.DebugPrintf("PRESSED - UNHANDLED UI ELEMENT\n");
			break;
		}
	}
	else if(bRepeat)	// REPEAT HANDLING
	{
		switch(m_ActiveUIElement->pControl->getControlType())
		{
		case UIControl::eButton:
			/* no action */
			break;
		case UIControl::eSlider:
			// handle slider movement
			UIControl_Slider *pSlider=(UIControl_Slider *)m_ActiveUIElement->pControl;
			float fNewSliderPos = ((float)x - (float)m_ActiveUIElement->x1) / (float)pSlider->GetRealWidth();
			pSlider->SetSliderTouchPos(fNewSliderPos);
			break;
		case UIControl::eCheckBox:
			/* no action */
			bRepeat = false;
			bPressed = false;
			break;
		case UIControl::eButtonList:
			// handle button list scrolling
			UIControl_ButtonList *pButtonList=(UIControl_ButtonList *)m_ActiveUIElement->pControl;
			pButtonList->SetTouchFocus((float)x, (float)y, true);
			break;
		case UIControl::eTexturePackList:
			// handle texturepack list scrolling
			UIControl_TexturePackList *pTexturePackList=(UIControl_TexturePackList *)m_ActiveUIElement->pControl;
			pTexturePackList->SetTouchFocus((float)x - (float)m_ActiveUIElement->x1, (float)y - (float)m_ActiveUIElement->y1, true);
			break;
		case UIControl::eTextInput:
			/* no action */
			bRepeat = false;
			bPressed = false;
			break;
		case UIControl::eDynamicLabel:
			// handle dynamic label scrolling
			UIControl_DynamicLabel *pDynamicLabel=(UIControl_DynamicLabel *)m_ActiveUIElement->pControl;
			pDynamicLabel->TouchScroll(y, true);
			// override bPressed & bRepeat to false
			bPressed = false;
			bRepeat = false;
			break;
		case UIControl::eHTMLLabel:
			// handle dynamic label scrolling
			UIControl_HTMLLabel *pHtmlLabel=(UIControl_HTMLLabel *)m_ActiveUIElement->pControl;
			pHtmlLabel->TouchScroll(y, true);
			// override bPressed & bRepeat to false
			bPressed = false;
			bRepeat = false;
			break;
		case UIControl::eLeaderboardList:
			// handle button list scrolling
			UIControl_LeaderboardList *pLeaderboardList=(UIControl_LeaderboardList *)m_ActiveUIElement->pControl;
			pLeaderboardList->SetTouchFocus((float)x, (float)y, true);
			break;
		case UIControl::eTouchControl:
			// override bPressed to false
			bPressed = false;
			// pass on touch input to relevant parent scene so we can handle it there!
			m_ActiveUIElement->pControl->getParentScene()->handleTouchInput(iPad, x, y, m_ActiveUIElement->pControl->getId(), bPressed, bRepeat, bReleased);
			// override bRepeat to false
			bRepeat = false;
			break;
		default:
			app.DebugPrintf("REPEAT - UNHANDLED UI ELEMENT\n");
			break;
		}
	}
	if(bReleased)	// RELEASED HANDLING
	{
		app.DebugPrintf("touch input released\n");
		switch(m_ActiveUIElement->pControl->getControlType())
		{
		case UIControl::eButton:
			// trigger button on release (ONLY if the finger is still on it!)
			if(m_HighlightedUIElement && m_ActiveUIElement->pControl == m_HighlightedUIElement->pControl)
				bPressed = true;
			break;
		case UIControl::eSlider:
			/* no action */
			break;
		case UIControl::eCheckBox:
			// trigger checkbox on release (ONLY if the finger is still on it!)
			if(m_HighlightedUIElement && m_ActiveUIElement->pControl == m_HighlightedUIElement->pControl)
			{
				UIControl_CheckBox *pCheckbox=(UIControl_CheckBox *)m_ActiveUIElement->pControl;
				if(pCheckbox->IsEnabled())	// only proceed if checkbox is enabled!
					pCheckbox->TouchSetCheckbox(!pCheckbox->IsChecked());
			}
			bReleased = false;
			break;
		case UIControl::eButtonList:
			// trigger buttonlist on release (ONLY if the finger is still on it!)
			if(m_HighlightedUIElement && m_ActiveUIElement->pControl == m_HighlightedUIElement->pControl)
			{
				UIControl_ButtonList *pButtonList=(UIControl_ButtonList *)m_ActiveUIElement->pControl;
				if(pButtonList->CanTouchTrigger(x,y))
					bPressed = true;
			}
			break;
		case UIControl::eTexturePackList:
			// trigger texturepack list on release (ONLY if the finger is still on it!)
			if(m_HighlightedUIElement && m_ActiveUIElement->pControl == m_HighlightedUIElement->pControl)
			{
				UIControl_TexturePackList *pTexturePackList=(UIControl_TexturePackList *)m_ActiveUIElement->pControl;
				if(pTexturePackList->CanTouchTrigger((float)x - (float)m_ActiveUIElement->x1, (float)y - (float)m_ActiveUIElement->y1))
					bPressed = true;
			}
			break;
		case UIControl::eTextInput:
			// trigger TextInput on release (ONLY if the finger is still on it!)
			if(m_HighlightedUIElement && m_ActiveUIElement->pControl == m_HighlightedUIElement->pControl)
				bPressed = true;
			break;
		case UIControl::eDynamicLabel:
			// handle dynamic label scrolling
			UIControl_DynamicLabel *pDynamicLabel=(UIControl_DynamicLabel *)m_ActiveUIElement->pControl;
			pDynamicLabel->TouchScroll(y, false);
			break;
		case UIControl::eHTMLLabel:
			// handle dynamic label scrolling
			UIControl_HTMLLabel *pHtmlLabel=(UIControl_HTMLLabel *)m_ActiveUIElement->pControl;
			pHtmlLabel->TouchScroll(y, false);
			break;
		case UIControl::eLeaderboardList:
			/* no action */
			break;
		case UIControl::eTouchControl:
			// trigger only if touch is released over the same component!
			if(m_HighlightedUIElement && m_ActiveUIElement->pControl == m_HighlightedUIElement->pControl)
			{
				// pass on touch input to relevant parent scene so we can handle it there!
				m_ActiveUIElement->pControl->getParentScene()->handleTouchInput(iPad, x, y, m_ActiveUIElement->pControl->getId(), bPressed, bRepeat, bReleased);
			}
			// override bReleased to false
			bReleased = false;
			break;
		default:
			app.DebugPrintf("RELEASED - UNHANDLED UI ELEMENT\n");
			break;
		}
	}

	// only proceed if there's input to be handled
	if(bPressed || bRepeat || bReleased)
	{
		SendTouchInput(iPad, key, bPressed, bRepeat, bReleased);
	}
}

void UIController::SendTouchInput(unsigned int iPad, unsigned int key, bool bPressed, bool bRepeat, bool bReleased)
{
	bool handled = false;

	// Send the key to the fullscreen group first
	m_groups[(int)eUIGroup_Fullscreen]->handleInput(iPad, key, bRepeat, bPressed, bReleased, handled);
	if(!handled)
	{
		// If it's not been handled yet, then pass the event onto the players specific group
		m_groups[(iPad+1)]->handleInput(iPad, key, bRepeat, bPressed, bReleased, handled);
	}
}


#endif
