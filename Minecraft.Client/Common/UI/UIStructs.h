#pragma once

#pragma message("UIStructs.h")

#include "UIEnums.h"

class Container;
class Inventory;
class BrewingStandTileEntity;
class DispenserTileEntity;
class FurnaceTileEntity;
class SignTileEntity;
class LevelGenerationOptions;
class LocalPlayer;
class Merchant;
class EntityHorse;
class BeaconTileEntity;
class Slot;
class AbstractContainerMenu;

// 4J Stu - Structs shared by Iggy and Xui scenes.
typedef struct _UIVec2D
{
	float x;
	float y;

	_UIVec2D& operator+=(const _UIVec2D &rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
} UIVec2D;

// Brewing
typedef struct _BrewingScreenInput
{
	shared_ptr<Inventory> inventory;
	shared_ptr<BrewingStandTileEntity> brewingStand;
	int iPad;
	bool bSplitscreen;
} BrewingScreenInput;

// Chest
typedef struct _ContainerScreenInput
{
	shared_ptr<Container> inventory;
	shared_ptr<Container> container;
	int					  iPad;
	bool bSplitscreen;
} ContainerScreenInput;

// Dispenser
typedef struct _TrapScreenInput
{
	shared_ptr<Container> inventory;
	shared_ptr<DispenserTileEntity> trap;
	int iPad;
	bool bSplitscreen;
} TrapScreenInput;

// Inventory and creative inventory
typedef struct _InventoryScreenInput
{
	shared_ptr<LocalPlayer> player;
	bool bNavigateBack; // If we came here from the crafting screen, go back to it, rather than closing the xui menus
	int iPad;
	bool bSplitscreen;
} InventoryScreenInput;

// Enchanting		
typedef struct _EnchantingScreenInput
{
	shared_ptr<Inventory> inventory;
	Level *level;
	int x;
	int y;
	int z;
	int iPad;
	bool bSplitscreen;
	wstring name;
} 
EnchantingScreenInput;

// Furnace
typedef struct _FurnaceScreenInput
{
	shared_ptr<Inventory> inventory;
	shared_ptr<FurnaceTileEntity> furnace;
	int iPad;
	bool bSplitscreen;
} FurnaceScreenInput;

// Crafting
typedef struct _CraftingPanelScreenInput
{
	shared_ptr<LocalPlayer> player;
	int						iContainerType; // RECIPE_TYPE_2x2 or RECIPE_TYPE_3x3
	bool					bSplitscreen;
	int						iPad;
	int						x;
	int						y;
	int						z;
} 
CraftingPanelScreenInput;

// Fireworks
typedef struct _FireworksScreenInput
{
	shared_ptr<LocalPlayer> player;
	bool					bSplitscreen;
	int						iPad;
	int						x;
	int						y;
	int						z;
} 
FireworksScreenInput;

// Trading
typedef struct _TradingScreenInput
{
	shared_ptr<Inventory> inventory;
	shared_ptr<Merchant> trader;
	Level *level;
	int iPad;
	bool bSplitscreen;
}
TradingScreenInput;

// Anvil
typedef struct _AnvilScreenInput
{
	shared_ptr<Inventory> inventory;
	Level *level;
	int x;
	int y;
	int z;
	int iPad;
	bool bSplitscreen;
}
AnvilScreenInput;

// Hopper
typedef struct _HopperScreenInput
{
	shared_ptr<Inventory> inventory;
	shared_ptr<Container> hopper;
	int iPad;
	bool bSplitscreen;
}
HopperScreenInput;

// Horse
typedef struct _HorseScreenInput
{
	shared_ptr<Inventory> inventory;
	shared_ptr<Container> container;
	shared_ptr<EntityHorse> horse;
	int iPad;
	bool bSplitscreen;
}
HorseScreenInput;

// Beacon
typedef struct _BeaconScreenInput
{
	shared_ptr<Inventory> inventory;
	shared_ptr<BeaconTileEntity> beacon;
	int iPad;
	bool bSplitscreen;
}
BeaconScreenInput;

// Sign
typedef struct _SignEntryScreenInput
{
	shared_ptr<SignTileEntity> sign;
	int iPad;
} SignEntryScreenInput;

// Connecting progress
typedef struct _ConnectionProgressParams
{
	int iPad;
	int stringId;
	bool showTooltips;
	bool setFailTimer;
	int timerTime;
	void (*cancelFunc)(LPVOID param);
	LPVOID cancelFuncParam;

	_ConnectionProgressParams()
	{
		iPad = 0;
		stringId = -1;
		showTooltips = false;
		setFailTimer = false;
		timerTime = 0;
		cancelFunc = nullptr;
		cancelFuncParam = nullptr;
	}
} ConnectionProgressParams;

// Fullscreen progress
typedef struct _UIFullscreenProgressCompletionData
{
	BOOL bRequiresUserAction;
	BOOL bShowBackground;
	BOOL bShowLogo;
	BOOL bShowTips;
	ProgressionCompletionType type;
	int iPad;
	EUIScene scene;

	_UIFullscreenProgressCompletionData()
	{
		bRequiresUserAction = FALSE;
		bShowBackground = TRUE;
		bShowLogo = TRUE;
		bShowTips = TRUE;
		type = e_ProgressCompletion_NoAction;
	}
} UIFullscreenProgressCompletionData;

// Create world
typedef struct _CreateWorldMenuInitData
{
	BOOL bOnline;
	BOOL bIsPrivate;
	int iPad;
} 
CreateWorldMenuInitData;

// Join/Load saves list
typedef struct _SaveListDetails
{
	int saveId;
	PBYTE pbThumbnailData;
	DWORD dwThumbnailSize;
#ifdef _DURANGO
	wchar_t UTF16SaveName[128];
	wchar_t UTF16SaveFilename[MAX_SAVEFILENAME_LENGTH];
#else
	char UTF8SaveName[128];
#ifndef _XBOX
	char UTF8SaveFilename[MAX_SAVEFILENAME_LENGTH];
#endif
#endif

	_SaveListDetails()
	{
		saveId = 0;
		pbThumbnailData = nullptr;
		dwThumbnailSize = 0;
#ifdef _DURANGO
		ZeroMemory(UTF16SaveName,sizeof(wchar_t)*128);
		ZeroMemory(UTF16SaveFilename,sizeof(wchar_t)*MAX_SAVEFILENAME_LENGTH);
#else
		ZeroMemory(UTF8SaveName,128);
#ifndef _XBOX
		ZeroMemory(UTF8SaveFilename,MAX_SAVEFILENAME_LENGTH);
#endif
#endif
	}

} SaveListDetails;

// Load world
typedef struct _LoadMenuInitData
{
	int iPad;
	int iSaveGameInfoIndex;
	LevelGenerationOptions *levelGen;
	SaveListDetails *saveDetails;
} 
LoadMenuInitData;

// Join Games
typedef struct _JoinMenuInitData
{
	FriendSessionInfo *selectedSession;
	int iPad;
#ifdef _WINDOWS64
	int serverIndex; // Index of the server in servers.db, -1 if not a saved server
#endif
} JoinMenuInitData;

// Native keyboard (Windows64 replacement for InputManager.RequestKeyboard WinAPI dialog)
#ifdef _WINDOWS64
typedef struct _UIKeyboardInitData
{
	const wchar_t* title;
	const wchar_t* defaultText;
	int maxChars;
	int(*callback)(LPVOID, const bool);
	LPVOID lpParam;
	bool pcMode; // When true, disables on-screen keyboard buttons (PC keyboard users only need the text field)

	_UIKeyboardInitData() : title(nullptr), defaultText(nullptr), maxChars(25), callback(nullptr), lpParam(nullptr), pcMode(false) {}
} UIKeyboardInitData;

// Stores the text typed in UIScene_Keyboard so callbacks can retrieve it
// without calling InputManager.GetText (which shows the WinAPI dialog result).
extern wchar_t g_Win64KeyboardResult[256];
inline void Win64_GetKeyboardText(uint16_t* outBuf, int maxChars)
{
	wcsncpy_s((wchar_t*)outBuf, maxChars, g_Win64KeyboardResult, _TRUNCATE);
}

// Returns true if any XInput controller is currently connected.
// Used to decide whether to show the in-game keyboard UI or fall back to PC input.
#include <Xinput.h>
inline bool Win64_IsControllerConnected()
{
	XINPUT_STATE state;
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		memset(&state, 0, sizeof(state));
		if (XInputGetState(i, &state) == ERROR_SUCCESS)
			return true;
	}
	return false;
}
#endif // _WINDOWS64

// More Options
typedef struct _LaunchMoreOptionsMenuInitData
{
	bool bOnlineGame;
	bool bInviteOnly;
	bool bAllowFriendsOfFriends;

	bool bGenerateOptions;
	bool bStructures;
	bool bFlatWorld;
	bool bBonusChest;

	bool bPVP;
	bool bTrust;
	bool bFireSpreads;
	bool bTNT;

	bool bHostPrivileges;
	bool bResetNether;

	bool bMobGriefing;
	bool bKeepInventory;
	bool bDoMobSpawning;
	bool bDoMobLoot;
	bool bDoTileDrops;
	bool bNaturalRegeneration;
	bool bDoDaylightCycle;

	bool bOnlineSettingChangedBySystem;

	int iPad;

	DWORD dwTexturePack;

	wstring seed;
	int worldSize;
	bool bDisableSaving;

	EGameHostOptionWorldSize currentWorldSize;
	EGameHostOptionWorldSize newWorldSize;
	bool newWorldSizeOverwriteEdges;

	_LaunchMoreOptionsMenuInitData()
	{
		memset(this,0,sizeof(_LaunchMoreOptionsMenuInitData));
		bOnlineGame = true;
		bAllowFriendsOfFriends = true;
		bPVP = true;
		bFireSpreads = true;
		bTNT = true;
		iPad = -1;
		worldSize = 3;
		seed = L"";
		bDisableSaving = false;
		newWorldSize = e_worldSize_Unknown;
		newWorldSizeOverwriteEdges = false;

		bMobGriefing = true;
		bKeepInventory = false;
		bDoMobSpawning = true;
		bDoMobLoot = true;
		bDoTileDrops = true;
		bNaturalRegeneration = true;
		bDoDaylightCycle = true;
	}
} 
LaunchMoreOptionsMenuInitData;

typedef struct _LoadingInputParams
{
	C4JThreadStartFunc* func;
	LPVOID lpParam;
	UIFullscreenProgressCompletionData *completionData;

	int cancelText;
	void (*cancelFunc)(LPVOID param);
	void (*completeFunc)(LPVOID param);
	LPVOID m_cancelFuncParam;
	LPVOID m_completeFuncParam;
	bool waitForThreadToDelete;

	_LoadingInputParams()
	{
		func = nullptr;
		lpParam = nullptr;
		completionData = nullptr;

		cancelText = -1;
		cancelFunc = nullptr;
		completeFunc = nullptr;
		m_cancelFuncParam = nullptr;
		m_completeFuncParam = nullptr;
		waitForThreadToDelete = false;
	}
} LoadingInputParams;

// Tutorial
#ifndef _XBOX
class UIScene;
#endif
class Tutorial;
typedef struct _TutorialPopupInfo
{
#ifdef _XBOX
	CXuiScene *interactScene;
#else
	UIScene *interactScene;
#endif
	LPCWSTR desc;
	LPCWSTR title;
	int icon;
	int iAuxVal /* = 0 */;
	bool isFoil /* = false */;
	bool allowFade /* = true */;
	bool isReminder /*= false*/;
	Tutorial *tutorial;

	_TutorialPopupInfo()
	{
		interactScene = nullptr;
		desc = L"";
		title = L"";
		icon = -1;
		iAuxVal = 0;
		isFoil = false;
		allowFade = true;
		isReminder = false;
		tutorial = nullptr;
	}

} TutorialPopupInfo;

// Quadrant sign in
typedef struct _SignInInfo
{
	int( *Func)(LPVOID,const bool, const int iPad);
	LPVOID lpParam;
	bool requireOnline;
} SignInInfo;

// Credits
typedef struct 
{
	LPCWSTR				m_Text;				// Should contain string, optionally with %s to add in translated string ... e.g. "Andy West - %s"
	int					m_iStringID[2];		// May be NO_TRANSLATED_STRING if we do not require to add any translated string.
	ECreditTextTypes	m_eType;
}
SCreditTextItemDef;

// Message box
typedef struct _MessageBoxInfo
{
	UINT uiTitle;
	UINT uiText;
	UINT *uiOptionA;
	UINT uiOptionC;
	DWORD dwPad;
	int( *Func)(LPVOID,int,const C4JStorage::EMessageResult);
	LPVOID lpParam;
	//C4JStringTable *pStringTable; // 4J Stu - We don't need this for our internal message boxes
	WCHAR *pwchFormatString;
	DWORD dwFocusButton;
} MessageBoxInfo;

typedef struct _DLCOffersParam
{
	int iPad;
	int iOfferC;
	int iType;		
} 
DLCOffersParam;

typedef struct _InGamePlayerOptionsInitData
{
	int iPad;
	BYTE networkSmallId;
	unsigned int playerPrivileges;
} InGamePlayerOptionsInitData;

typedef struct _DebugSetCameraPosition
{
	int player;
	double m_camX, m_camY, m_camZ, m_yRot, m_elev;
} DebugSetCameraPosition;

typedef struct _TeleportMenuInitData
{
	int iPad;
	bool teleportToPlayer;
} TeleportMenuInitData;

typedef struct _CustomDrawData
{
	float x0, y0, x1, y1;     // the bounding box of the original DisplayObject, in object space
	float mat[16];
} CustomDrawData;

typedef struct _ItemEditorInput
{
	int iPad;
	Slot *slot;
	AbstractContainerMenu *menu;
} ItemEditorInput;