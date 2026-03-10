#pragma once

using namespace std;

#include "Audio/Consoles_SoundEngine.h"

#include <xuiapp.h>
#include ".\Tutorial\TutorialEnum.h"

#ifdef _XBOX
#include ".\XUI\XUI_Helper.h"
#include ".\XUI\XUI_HelpCredits.h"
#endif
#include "UI\UIStructs.h"

#include "..\..\Minecraft.World\DisconnectPacket.h"
#include <xsocialpost.h>

#include "..\StringTable.h"
#include ".\DLC\DLCManager.h"
#include ".\GameRules\ConsoleGameRulesConstants.h"
#include ".\GameRules\GameRuleManager.h"
#include "..\SkinBox.h"
#include "..\ArchiveFile.h"

typedef struct _JoinFromInviteData
{
	DWORD dwUserIndex; // dwUserIndex
	DWORD dwLocalUsersMask;   // dwUserMask
	const INVITE_INFO *pInviteInfo;  // pInviteInfo
}
JoinFromInviteData;

class Player;
class Inventory;
class Level;
class FurnaceTileEntity;
class Container;
class DispenserTileEntity;
class SignTileEntity;
class BrewingStandTileEntity;
class CommandBlockEntity;
class HopperTileEntity;
class MinecartHopper;
class EntityHorse;
class BeaconTileEntity;
class LocalPlayer;
class DLCPack;
class LevelRuleset;
class ConsoleSchematicFile;
class Model;
class ModelPart;
class StringTable;
class Merchant;

class CMinecraftAudio;

class CMinecraftApp

#ifdef _XBOX
	: public CXuiModule
#endif
{
private:
	static int s_iHTMLFontSizesA[eHTMLSize_COUNT];

public:

	CMinecraftApp();

	static const float fSafeZoneX; // 5% of 1280
	static const float fSafeZoneY; // 5% of 720

	typedef std::vector <PMEMDATA> VMEMFILES;
	typedef std::vector <PNOTIFICATION> VNOTIFICATIONS;

	// storing skin files
	std::vector <wstring > vSkinNames;
	DLCManager m_dlcManager;

	// storing credits text from the DLC
	std::vector <wstring > m_vCreditText; // hold the credit text lines so we can avoid duplicating them


	// In builds prior to TU5, the size of the GAME_SETTINGS struct was 204 bytes. We added a few new values to the internal struct in TU5, and even though we
	// changed the size of the ucUnused array to be decreased by the size of the values we added, the packing of the struct has introduced some extra
	// padding that resulted in the GAME_SETTINGS struct being 208 bytes. The knock-on effect from this was that all the stats, which come after the game settings
	// in the profile data, we being read offset by 4 bytes. We need to ensure that the GAME_SETTINGS struct does not grow larger than 204 bytes or if we need it
	// to then we need to rebuild the profile data completely and increase the profile version. There should be enough free space to grow larger for a few more updates
	// as long as we take into account the padding issues and check that settings are still stored at the same positions when we read them
	static const int GAME_SETTINGS_PROFILE_DATA_BYTES = 204;

#ifdef _EXTENDED_ACHIEVEMENTS
	/* 4J-JEV:
	* We need more space in the profile data because of the new achievements and statistics
	* necessary for the new expanded achievement set.
	*/
	static const int GAME_DEFINED_PROFILE_DATA_BYTES = 2*972; // per user
#else
	static const int GAME_DEFINED_PROFILE_DATA_BYTES = 972; // per user
#endif
	unsigned int uiGameDefinedDataChangedBitmask;

	void			DebugPrintf(const char *szFormat, ...);
	void			DebugPrintfVerbose(bool bVerbose, const char *szFormat, ...);	// Conditional printf
	void			DebugPrintf(int user, const char *szFormat, ...);

	static const int USER_NONE = 0;  // disables printf
	static const int USER_GENERAL = 1;
	static const int USER_JV = 2;
	static const int USER_MH = 3;
	static const int USER_PB = 4;
	static const int USER_RR = 5;
	static const int USER_SR = 6;
	static const int USER_UI = 7; // 4J Stu - This also makes it appear on the UI console

	void			HandleButtonPresses();
	bool			IntroRunning()																									{ return m_bIntroRunning;}
	void			SetIntroRunning(bool bSet)																						{m_bIntroRunning=bSet;}
#ifdef _CONTENT_PACKAGE
#ifndef _FINAL_BUILD
	bool			PartnernetPasswordRunning()																									{ return m_bPartnernetPasswordRunning;}
	void			SetPartnernetPasswordRunning(bool bSet)																						{m_bPartnernetPasswordRunning=bSet;}
#endif
#endif

	bool			IsAppPaused();
	void			SetAppPaused(bool val);
	static int		DisplaySavingMessage(LPVOID pParam,const C4JStorage::ESavingMessage eMsg, int iPad);
	bool			GetGameStarted()																								{return m_bGameStarted;}
	void			SetGameStarted(bool bVal)																						{ if(bVal) DebugPrintf("SetGameStarted - true\n"); else DebugPrintf("SetGameStarted - false\n"); m_bGameStarted = bVal; m_bIsAppPaused = !bVal;}
	int				GetLocalPlayerCount(void);
	bool			LoadInventoryMenu(int iPad,shared_ptr<LocalPlayer> player, bool bNavigateBack=false);
	bool			LoadCreativeMenu(int iPad,shared_ptr<LocalPlayer> player,bool bNavigateBack=false);
	bool			LoadEnchantingMenu(int iPad,shared_ptr<Inventory> inventory, int x, int y, int z, Level *level, const wstring &name);
	bool			LoadFurnaceMenu(int iPad,shared_ptr<Inventory> inventory, shared_ptr<FurnaceTileEntity> furnace);
	bool			LoadBrewingStandMenu(int iPad,shared_ptr<Inventory> inventory, shared_ptr<BrewingStandTileEntity> brewingStand);
	bool			LoadContainerMenu(int iPad,shared_ptr<Container> inventory, shared_ptr<Container> container);
	bool			LoadTrapMenu(int iPad,shared_ptr<Container> inventory, shared_ptr<DispenserTileEntity> trap);
	bool			LoadCrafting2x2Menu(int iPad,shared_ptr<LocalPlayer> player);
	bool			LoadCrafting3x3Menu(int iPad,shared_ptr<LocalPlayer> player, int x, int y, int z);
	bool			LoadFireworksMenu(int iPad,shared_ptr<LocalPlayer> player, int x, int y, int z);
	bool			LoadSignEntryMenu(int iPad,shared_ptr<SignTileEntity> sign);
	bool			LoadRepairingMenu(int iPad,shared_ptr<Inventory> inventory, Level *level, int x, int y, int z);
	bool			LoadTradingMenu(int iPad, shared_ptr<Inventory> inventory, shared_ptr<Merchant> trader, Level *level, const wstring &name);

	bool			LoadCommandBlockMenu(int iPad, shared_ptr<CommandBlockEntity> commandBlock) { return false; }
	bool			LoadHopperMenu(int iPad ,shared_ptr<Inventory> inventory, shared_ptr<HopperTileEntity> hopper);
	bool			LoadHopperMenu(int iPad ,shared_ptr<Inventory> inventory, shared_ptr<MinecartHopper> hopper);
	bool			LoadHorseMenu(int iPad ,shared_ptr<Inventory> inventory, shared_ptr<Container> container, shared_ptr<EntityHorse> horse);
	bool			LoadBeaconMenu(int iPad ,shared_ptr<Inventory> inventory, shared_ptr<BeaconTileEntity> beacon);

	bool			GetTutorialMode()																									{ return m_bTutorialMode;}
	void			SetTutorialMode(bool bSet)																							{m_bTutorialMode=bSet;}

	void			SetSpecialTutorialCompletionFlag(int iPad, int index);

	static			LPCWSTR			GetString(int iID);

	eGameMode		GetGameMode()																										{ return m_eGameMode;}
	void			SetGameMode(eGameMode eMode)																						{ m_eGameMode=eMode;}

	eXuiAction      GetGlobalXuiAction()																								{return m_eGlobalXuiAction;}
	void			SetGlobalXuiAction(eXuiAction action)																				{m_eGlobalXuiAction=action;}
	eXuiAction      GetXuiAction(int iPad)																								{return m_eXuiAction[iPad];}
    void SetAction(int iPad, eXuiAction action, LPVOID param = nullptr);
    void SetTMSAction(int iPad, eTMSAction action) { m_eTMSAction[iPad] = action; }
	eTMSAction      GetTMSAction(int iPad)																								{return m_eTMSAction[iPad];}
	eXuiServerAction GetXuiServerAction(int iPad)																						{return m_eXuiServerAction[iPad];}
	LPVOID			GetXuiServerActionParam(int iPad)																					{return m_eXuiServerActionParam[iPad];}
	void			SetXuiServerAction(int iPad, eXuiServerAction action, LPVOID param = nullptr)											{m_eXuiServerAction[iPad]=action; m_eXuiServerActionParam[iPad] = param;}
	eXuiServerAction GetGlobalXuiServerAction()																							{return m_eGlobalXuiServerAction;}
	void			SetGlobalXuiServerAction(eXuiServerAction action)																			{m_eGlobalXuiServerAction=action;}

	DisconnectPacket::eDisconnectReason	GetDisconnectReason()																			{ return m_disconnectReason; }
	void			SetDisconnectReason(DisconnectPacket::eDisconnectReason bVal)														{ m_disconnectReason = bVal; }

	bool			GetChangingSessionType()																							{ return m_bChangingSessionType; }
	void			SetChangingSessionType(bool bVal)																					{ m_bChangingSessionType = bVal; }

	bool			GetReallyChangingSessionType()																						{ return m_bReallyChangingSessionType; }
	void			SetReallyChangingSessionType(bool bVal)																				{ m_bReallyChangingSessionType = bVal; }


	// 4J Stu - Added so that we can call this when a confirmation box is selected
	static void		SetActionConfirmed(LPVOID param);
	void			HandleXuiActions(void);

	// 4J Stu - Functions used for Minecon and other promo work
	bool			GetLoadSavesFromFolderEnabled()																						{ return m_bLoadSavesFromFolderEnabled; }
	void			SetLoadSavesFromFolderEnabled(bool bVal)																			{ m_bLoadSavesFromFolderEnabled = bVal; }

	// 4J Stu - Useful for debugging
	bool			GetWriteSavesToFolderEnabled()																						{ return m_bWriteSavesToFolderEnabled; }
	void			SetWriteSavesToFolderEnabled(bool bVal)																				{ m_bWriteSavesToFolderEnabled = bVal; }
	bool			GetMobsDontAttackEnabled()																							{ return m_bMobsDontAttack; }
	void			SetMobsDontAttackEnabled(bool bVal)																				{ m_bMobsDontAttack = bVal; }
	bool			GetUseDPadForDebug()																							{ return m_bUseDPadForDebug; }
	void			SetUseDPadForDebug(bool bVal)																				{ m_bUseDPadForDebug = bVal; }
	bool			GetMobsDontTickEnabled()																							{ return m_bMobsDontTick; }
	void			SetMobsDontTickEnabled(bool bVal)																			{ m_bMobsDontTick = bVal; }

	bool			GetFreezePlayers()																									{ return m_bFreezePlayers; }
	void			SetFreezePlayers(bool bVal)																							{ m_bFreezePlayers = bVal; }

	// debug -0 show safe area
	void			ShowSafeArea(BOOL bShow)
	{
#ifdef _XBOX
		CXuiSceneBase::ShowSafeArea( bShow );
#endif
	}
	// 4J-PB - to capture the social post screenshot
	virtual void			CaptureScreenshot(int iPad) {};
	//void			GetPreviewImage(int iPad,XSOCIAL_PREVIEWIMAGE *preview);

	void			InitGameSettings();
	static int		OldProfileVersionCallback(LPVOID pParam,unsigned char *pucData, const unsigned short usVersion, const int iPad);

#if ( defined  __PS3__  || defined __ORBIS__ || defined _DURANGO || defined __PSVITA__ )
	wstring			toStringOptionsStatus(const C4JStorage::eOptionsCallback &eStatus);
	static int		DefaultOptionsCallback(LPVOID pParam,C4JStorage::PROFILESETTINGS *pSettings, const int iPad);
	int				SetDefaultOptions(C4JStorage::PROFILESETTINGS *pSettings,const int iPad,bool bWriteProfile=true);
#ifdef __ORBIS__
	static int		OptionsDataCallback(LPVOID pParam,int iPad,unsigned short usVersion,C4JStorage::eOptionsCallback eStatus,int iBlocksRequired);
	int				GetOptionsBlocksRequired(int iPad);
#else
	static int		OptionsDataCallback(LPVOID pParam,int iPad,unsigned short usVersion,C4JStorage::eOptionsCallback eStatus);
#endif

	C4JStorage::eOptionsCallback GetOptionsCallbackStatus(int iPad);

	void			SetOptionsCallbackStatus(int iPad, C4JStorage::eOptionsCallback eStatus);
#else
	static int		DefaultOptionsCallback(LPVOID pParam,C_4JProfile::PROFILESETTINGS *pSettings, const int iPad);
	int				SetDefaultOptions(C_4JProfile::PROFILESETTINGS *pSettings,const int iPad);
#endif
	virtual void	SetRichPresenceContext(int iPad, int contextId) = 0;


	void			SetGameSettings(int iPad,eGameSetting eVal,unsigned char ucVal);
	unsigned char	GetGameSettings(int iPad,eGameSetting eVal);
	unsigned char	GetGameSettings(eGameSetting eVal); // for the primary pad
	void			SetPlayerSkin(int iPad,const wstring &name);
	void			SetPlayerSkin(int iPad,DWORD dwSkinId);
	void			SetPlayerCape(int iPad,const wstring &name);
	void			SetPlayerCape(int iPad,DWORD dwCapeId);
	void			SetPlayerFavoriteSkin(int iPad, int iIndex,unsigned int uiSkinID);
	unsigned int	GetPlayerFavoriteSkin(int iPad,int iIndex);
	unsigned char	GetPlayerFavoriteSkinsPos(int iPad);
	void			SetPlayerFavoriteSkinsPos(int iPad,int iPos);
	unsigned int	GetPlayerFavoriteSkinsCount(int iPad);
	void			ValidateFavoriteSkins(int iPad); // check the DLC is available for the skins

	// Mash-up pack worlds hide/display
	void			HideMashupPackWorld(int iPad, unsigned int iMashupPackID);
	void			EnableMashupPackWorlds(int iPad);
	unsigned int	GetMashupPackWorlds(int iPad);

	// Minecraft language select
	void			SetMinecraftLanguage(int iPad, unsigned char ucLanguage);
	unsigned char	GetMinecraftLanguage(int iPad);
	void			SetMinecraftLocale(int iPad, unsigned char ucLanguage);
	unsigned char	GetMinecraftLocale(int iPad);

	// 4J-PB - set a timer when the user navigates the quickselect, so we can bring the opacity back to defaults for a short time
	unsigned int	GetOpacityTimer(int iPad)																						{ return m_uiOpacityCountDown[iPad]; }
	void			SetOpacityTimer(int iPad)																						{ m_uiOpacityCountDown[iPad]=120;  } // 6 seconds
	void			TickOpacityTimer(int iPad)																						{ if(m_uiOpacityCountDown[iPad]>0) m_uiOpacityCountDown[iPad]--;}

public:
	wstring			GetPlayerSkinName(int iPad);
	DWORD			GetPlayerSkinId(int iPad);
	wstring			GetPlayerCapeName(int iPad);
	DWORD			GetPlayerCapeId(int iPad);
	DWORD			GetAdditionalModelParts(int iPad);
	void			CheckGameSettingsChanged(bool bOverride5MinuteTimer=false, int iPad=XUSER_INDEX_ANY);
	void			ApplyGameSettingsChanged(int iPad);
	void			ClearGameSettingsChangedFlag(int iPad);
	void			ActionGameSettings(int iPad,eGameSetting eVal);
	unsigned int    GetGameSettingsDebugMask(int iPad=-1,bool bOverridePlayer=false);
	void			SetGameSettingsDebugMask(int iPad, unsigned int uiVal);
	void			ActionDebugMask(int iPad, bool bSetAllClear=false);

	//
	bool IsLocalMultiplayerAvailable();

	// for sign in change monitoring
	static void		SignInChangeCallback(LPVOID pParam, bool bVal, unsigned int uiSignInData);
	static void ClearSignInChangeUsersMask();
	static int SignoutExitWorldThreadProc( void* lpParameter );
	static int PrimaryPlayerSignedOutReturned(void *pParam, int iPad, const C4JStorage::EMessageResult);
	static int EthernetDisconnectReturned(void *pParam, int iPad, const C4JStorage::EMessageResult);
	static void		ProfileReadErrorCallback(void *pParam);

	// FATAL LOAD ERRORS
	virtual void FatalLoadError();

	// Notifications from the game listener to be passed to the qnet listener
	static void NotificationsCallback(LPVOID pParam,DWORD dwNotification, unsigned int uiParam);

	// for the ethernet being disconnected
	static void		LiveLinkChangeCallback(LPVOID pParam,BOOL bConnected);
	bool		    GetLiveLinkRequired()																								{return m_bLiveLinkRequired;}
	void			SetLiveLinkRequired(bool required)																					{m_bLiveLinkRequired=required;}

	static void		UpsellReturnedCallback(LPVOID pParam, eUpsellType type, eUpsellResponse result, int iUserData);

#if defined __PS3__ || defined __PSVITA__ || defined __ORBIS__
	static int NowDisplayFullVersionPurchase(void *pParam, bool bContinue, int iPad);
	static int MustSignInFullVersionPurchaseReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
#endif
#if defined __PS3__ || defined __PSVITA__ || defined __ORBIS__
	static int MustSignInFullVersionPurchaseReturnedExitTrial(void *pParam,int iPad,C4JStorage::EMessageResult result);
#endif

#ifdef _DEBUG_MENUS_ENABLED
	bool			DebugSettingsOn()														{ return m_bDebugOptions;}
	bool			DebugArtToolsOn();
#else
	bool			DebugSettingsOn()														{ return false;}
	bool			DebugArtToolsOn()														{ return false;}
#endif
	void			SetDebugSequence(const char *pchSeq);
	static int		DebugInputCallback(LPVOID pParam);
	//bool			UploadFileToGlobalStorage(int iQuadrant, C4JStorage::eGlobalStorage eStorageFacility, wstring *wsFile  );

	// Installed DLC
	bool StartInstallDLCProcess(int iPad);
	static int DLCInstalledCallback(LPVOID pParam,int iOfferC,int iPad);
	void HandleDLCLicenseChange();
	static int DLCMountedCallback(LPVOID pParam,int iPad,DWORD dwErr,DWORD dwLicenceMask);
	void MountNextDLC(int iPad);
	//static int DLCReadCallback(LPVOID pParam,C4JStorage::DLC_FILE_DETAILS *pDLCData);
	void HandleDLC(DLCPack *pack);
	bool DLCInstallPending()	{return m_bDLCInstallPending;}
	bool DLCInstallProcessCompleted()	{return m_bDLCInstallProcessCompleted;}
	void ClearDLCInstalled()	{ m_bDLCInstallProcessCompleted=false;}
	static int MarketplaceCountsCallback(LPVOID pParam,C4JStorage::DLC_TMS_DETAILS *,int iPad);

	bool AlreadySeenCreditText(const wstring &wstemp);

	void ClearNewDLCAvailable(void)				{ m_bNewDLCAvailable=false; m_bSeenNewDLCTip=true;}
	bool GetNewDLCAvailable()					{ return m_bNewDLCAvailable;}
	void DisplayNewDLCTipAgain()				{ m_bSeenNewDLCTip=false;}
	bool DisplayNewDLCTip()						{ if(!m_bSeenNewDLCTip) { m_bSeenNewDLCTip=true; return true;} else return false;}

	// functions to store launch data, and to exit the game - required due to possibly being on a demo disc
	virtual void StoreLaunchData();
	virtual void ExitGame();

	bool isXuidNotch(PlayerUID xuid);
	bool isXuidDeadmau5(PlayerUID xuid);

	void AddMemoryTextureFile(const wstring &wName, PBYTE pbData, DWORD dwBytes);
	void RemoveMemoryTextureFile(const wstring &wName);
	void GetMemFileDetails(const wstring &wName,PBYTE *ppbData,DWORD *pdwBytes);
	bool IsFileInMemoryTextures(const wstring &wName);

	// Texture Pack Data files (icon, banner, comparison shot & text)
	void AddMemoryTPDFile(int iConfig,PBYTE pbData,DWORD dwBytes);
	void RemoveMemoryTPDFile(int iConfig);
	bool IsFileInTPD(int iConfig);
	void GetTPD(int iConfig,PBYTE *ppbData,DWORD *pdwBytes);
	int GetTPDSize() {return m_MEM_TPD.size();}
#ifndef __PS3__
	int GetTPConfigVal(WCHAR *pwchDataFile);
#endif

	bool DefaultCapeExists();
	//void InstallDefaultCape(); // attempt  to install the default cape once per game launch

	// invites
	//void ProcessInvite(JoinFromInviteData *pJoinData);
	void ProcessInvite(DWORD dwUserIndex, DWORD dwLocalUsersMask, const INVITE_INFO * pInviteInfo);

	// Add credits for DLC installed
	void AddCreditText(LPCWSTR lpStr);

private:
	PlayerUID m_xuidNotch;
#ifdef _DURANGO
	unordered_map<PlayerUID, PBYTE, PlayerUID::Hash> m_GTS_Files;
#else
	unordered_map<PlayerUID, PBYTE> m_GTS_Files;
#endif

	// for storing memory textures - player skin
	unordered_map<wstring, PMEMDATA> m_MEM_Files;
	// for storing texture pack data files
	unordered_map<int, PMEMDATA> m_MEM_TPD;
	CRITICAL_SECTION csMemFilesLock; // For locking access to the above map
	CRITICAL_SECTION csMemTPDLock; // For locking access to the above map

	VNOTIFICATIONS m_vNotifications;

public:
	// launch data
	BYTE* m_pLaunchData;
	DWORD m_dwLaunchDataSize;

public:
	// BAN LIST
	void AddLevelToBannedLevelList(int iPad,PlayerUID xuid, char *pszLevelName, bool bWriteToTMS);
	bool IsInBannedLevelList(int iPad, PlayerUID xuid, char *pszLevelName);
	void RemoveLevelFromBannedLevelList(int iPad, PlayerUID xuid, char *pszLevelName);
	void InvalidateBannedList(int iPad);
	void SetUniqueMapName(char *pszUniqueMapName);
	char *GetUniqueMapName(void);
#ifdef _XBOX_ONE
	void AddLevelToBannedLevelList(int iPad, PBANNEDLISTDATA pBannedListData, bool bWriteToTMS);
#endif


public:
	bool GetResourcesLoaded() {return m_bResourcesLoaded;}
	void SetResourcesLoaded(bool bVal) {m_bResourcesLoaded=bVal;}

public:
	bool m_bGameStarted;
	bool m_bIntroRunning;
	bool m_bTutorialMode;
	bool m_bIsAppPaused;

	bool m_bChangingSessionType;
	bool m_bReallyChangingSessionType;

	bool m_bDisplayFullVersionPurchase; // for after signing in during the trial, and trying to unlock full version on an upsell

	void loadMediaArchive();
	void loadStringTable();

protected:
	ArchiveFile *m_mediaArchive;
	StringTable *m_stringTable;

public:
	int getArchiveFileSize(const wstring &filename);
	bool hasArchiveFile(const wstring &filename);
	byteArray getArchiveFile(const wstring &filename);

private:

	static int BannedLevelDialogReturned(void *pParam,int iPad,const C4JStorage::EMessageResult);
	static int TexturePackDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

	VBANNEDLIST *m_vBannedListA[XUSER_MAX_COUNT];

	void HandleButtonPresses(int iPad);

	bool m_bResourcesLoaded;

	// Global string table for this application.
	//CXuiStringTable StringTable;


	// Container scene for some menu

	//	CXuiScene debugContainerScene;


	//bool m_bSplitScreenEnabled;


#ifdef _CONTENT_PACKAGE
#ifndef _FINAL_BUILD
	bool m_bPartnernetPasswordRunning;
#endif
#endif

	eGameMode m_eGameMode; // single or multiplayer

	static unsigned int m_uiLastSignInData;

	// We've got sizeof(GAME_SETTINGS) bytes reserved at the start of the gamedefined data per player for settings
	GAME_SETTINGS *GameSettingsA[XUSER_MAX_COUNT];

	// For promo work
	bool m_bLoadSavesFromFolderEnabled;

	// For debugging
	bool m_bWriteSavesToFolderEnabled;
	bool m_bMobsDontAttack;
	bool m_bUseDPadForDebug;
	bool m_bMobsDontTick;
	bool m_bFreezePlayers;

	// 4J : WESTY : For taking screen shots.
	//bool m_bInterfaceRenderingOff;
	//bool m_bHandRenderingOff;

	DisconnectPacket::eDisconnectReason m_disconnectReason;

public:
	virtual void RunFrame() {};



	static const DWORD m_dwOfferID = 0x00000001;

	// timer
	void InitTime();
	void UpdateTime();

	// trial timer
	void SetTrialTimerStart(void);
	float getTrialTimer(void);

	// notifications from the game for qnet
	VNOTIFICATIONS *GetNotifications()	{return &m_vNotifications;}

private:


	// To avoid problems with threads being kicked off from xuis that alter things that may be in progress within the run_middle,
	// we'll action these at the end of the game loop
	eXuiAction m_eXuiAction[XUSER_MAX_COUNT];
	eTMSAction m_eTMSAction[XUSER_MAX_COUNT];
	LPVOID m_eXuiActionParam[XUSER_MAX_COUNT];
	eXuiAction m_eGlobalXuiAction;
	eXuiServerAction m_eXuiServerAction[XUSER_MAX_COUNT];
	LPVOID m_eXuiServerActionParam[XUSER_MAX_COUNT];
	eXuiServerAction m_eGlobalXuiServerAction;

	bool m_bLiveLinkRequired;

	static int		UnlockFullExitReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		UnlockFullSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		UnlockFullInviteReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		TrialOverReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		ExitAndJoinFromInvite(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		ExitAndJoinFromInviteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		ExitAndJoinFromInviteAndSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		ExitAndJoinFromInviteDeclineSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		FatalErrorDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		WarningTrialTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

	JoinFromInviteData m_InviteData;
	bool m_bDebugOptions; // toggle debug things on or off

	// Trial timer
	float m_fTrialTimerStart,mfTrialPausedTime;
	typedef struct TimeInfo
	{
		LARGE_INTEGER qwTime;
		LARGE_INTEGER qwAppTime;

		float fAppTime;
		float fElapsedTime;
		float fSecsPerTick;
	} TIMEINFO;

	TimeInfo m_Time;

protected:
	static const int MAX_TIPS_GAMETIP = 50;
	static const int MAX_TIPS_TRIVIATIP = 20;
	static TIPSTRUCT m_GameTipA[MAX_TIPS_GAMETIP];
	static TIPSTRUCT m_TriviaTipA[MAX_TIPS_TRIVIATIP];
	static Random *TipRandom;
public:
	void InitialiseTips();
	UINT GetNextTip();
	int GetHTMLColour(eMinecraftColour colour);
	int GetHTMLColor(eMinecraftColour colour) { return GetHTMLColour(colour); }
	int GetHTMLFontSize(EHTMLFontSize size);
	wstring FormatHTMLString(int iPad, const wstring &desc, int shadowColour = 0xFFFFFFFF);
	wstring GetActionReplacement(int iPad, unsigned char ucAction);
	wstring GetVKReplacement(unsigned int uiVKey);
	wstring GetIconReplacement(unsigned int uiIcon);

	float getAppTime() { return m_Time.fAppTime; }
	void UpdateTrialPausedTimer() { mfTrialPausedTime+= m_Time.fElapsedTime;}

	static int RemoteSaveThreadProc( void* lpParameter );
	static void ExitGameFromRemoteSave( LPVOID lpParameter );
	static int ExitGameFromRemoteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
private:
	UINT m_TipIDA[MAX_TIPS_GAMETIP+MAX_TIPS_TRIVIATIP];
	UINT m_uiCurrentTip;
	static int TipsSortFunction(const void* a, const void* b);

	// XML
public:

	// Hold a vector of terrain feature positions
	void AddTerrainFeaturePosition(_eTerrainFeatureType,int,int);
	void ClearTerrainFeaturePosition();
	_eTerrainFeatureType IsTerrainFeature(int x,int z);
	bool GetTerrainFeaturePosition(_eTerrainFeatureType eType, int *pX, int *pZ);
	std::vector <FEATURE_DATA *> m_vTerrainFeatures;

	static HRESULT RegisterMojangData(WCHAR *, PlayerUID, WCHAR *, WCHAR *);
	MOJANG_DATA *GetMojangDataForXuid(PlayerUID xuid);
	static HRESULT RegisterConfigValues(WCHAR *pType, int iValue);

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	HRESULT RegisterDLCData(char *pchDLCName, unsigned int uiSortIndex, char *pchImageURL);
	bool GetDLCFullOfferIDForSkinID(const wstring &FirstSkin,ULONGLONG *pullVal);
	DLC_INFO *GetDLCInfoForTrialOfferID(ULONGLONG ullOfferID_Trial);
	DLC_INFO *GetDLCInfoForFullOfferID(ULONGLONG ullOfferID_Full);
#elif defined(_XBOX_ONE)
	static HRESULT RegisterDLCData(eDLCContentType, WCHAR *, WCHAR *, WCHAR *, WCHAR *, int, unsigned int);
	//bool GetDLCFullOfferIDForSkinID(const wstring &FirstSkin,WCHAR *pwchProductId);
	bool GetDLCFullOfferIDForSkinID(const wstring &FirstSkin,wstring &wsProductId);
	DLC_INFO *GetDLCInfoForFullOfferID(WCHAR *pwchProductId);
	DLC_INFO *GetDLCInfoForProductName(WCHAR *pwchProductName);
#else
	static HRESULT RegisterDLCData(WCHAR *, WCHAR *, int, uint64_t, uint64_t, WCHAR *, unsigned int, int, WCHAR *pDataFile);
	bool GetDLCFullOfferIDForSkinID(const wstring &FirstSkin,ULONGLONG *pullVal);
	DLC_INFO *GetDLCInfoForTrialOfferID(ULONGLONG ullOfferID_Trial);
	DLC_INFO *GetDLCInfoForFullOfferID(ULONGLONG ullOfferID_Full);
#endif

	unsigned int GetDLCCreditsCount();
	SCreditTextItemDef * GetDLCCredits(int iIndex);

	// TMS
	void ReadDLCFileFromTMS(int iPad,eTMSAction action, bool bCallback=false);
	void ReadXuidsFileFromTMS(int iPad,eTMSAction action,bool bCallback=false);

	// images for save thumbnail/social post
	virtual void CaptureSaveThumbnail() =0;
	virtual void GetSaveThumbnail(PBYTE*,DWORD*)=0;
	virtual void ReleaseSaveThumbnail()=0;
	virtual void GetScreenshot(int iPad,PBYTE *pbData,DWORD *pdwSize)=0;

	virtual void ReadBannedList(int iPad, eTMSAction action=static_cast<eTMSAction>(0), bool bCallback=false)=0;

private:

	std::vector <SCreditTextItemDef *> vDLCCredits;

#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
	static unordered_map<PlayerUID,MOJANG_DATA *, PlayerUID::Hash > MojangData;
	static unordered_map<int, char * >  DLCTextures_PackID; // for mash-up packs & texture packs
	static unordered_map<string,DLC_INFO * > DLCInfo;
	static unordered_map<wstring, ULONGLONG >  DLCInfo_SkinName; // skin name, full offer id
#elif defined(_DURANGO)
	static unordered_map<PlayerUID,MOJANG_DATA *, PlayerUID::Hash > MojangData;
	static unordered_map<int, wstring >  DLCTextures_PackID; // for mash-up packs & texture packs
	//static unordered_map<wstring,DLC_INFO * > DLCInfo_Trial; // full offerid, dlc_info
	static unordered_map<wstring,DLC_INFO * > DLCInfo_Full; // full offerid, dlc_info
	static unordered_map<wstring, wstring >  DLCInfo_SkinName; // skin name, full offer id
#else
	static unordered_map<PlayerUID,MOJANG_DATA * > MojangData;
	static unordered_map<int, ULONGLONG >  DLCTextures_PackID; // for mash-up packs & texture packs
	static unordered_map<ULONGLONG,DLC_INFO * > DLCInfo_Trial; // full offerid, dlc_info
	static unordered_map<ULONGLONG,DLC_INFO * > DLCInfo_Full; // full offerid, dlc_info
	static unordered_map<wstring, ULONGLONG >  DLCInfo_SkinName; // skin name, full offer id
#endif
	//	bool m_bRead_TMS_XUIDS_XML; // track whether we have already read the TMS xuids.xml file
	//	bool m_bRead_TMS_DLCINFO_XML; // track whether we have already read the TMS DLC.xml file

	bool m_bDefaultCapeInstallAttempted; // have we attempted to install the default cape from tms

	//bool m_bwasHidingGui; // 4J Stu - Removed 1.8.2 bug fix (TU6) as not needed
	bool m_bDLCInstallProcessCompleted;
	bool m_bDLCInstallPending;
	int m_iTotalDLC;
	int m_iTotalDLCInstalled;

public:
	// 4J Stu - We need to be able to detect when a guest player signs in or out causing other guest players to change their xuid
	// The simplest way to do this is to check if their guest number has changed, so store the last known one here
	// 4J Stu - Now storing the whole XUSER_SIGNIN_INFO so we can detect xuid changes
	XUSER_SIGNIN_INFO m_currentSigninInfo[XUSER_MAX_COUNT];

	//void OverrideFontRenderer(bool set, bool immediate = true);
	//	void ToggleFontRenderer() { OverrideFontRenderer(!m_bFontRendererOverridden,false); }
	BANNEDLIST BannedListA[XUSER_MAX_COUNT];

private:
	// 	XUI_FontRenderer *m_fontRenderer;
	// 	bool m_bFontRendererOverridden;
	// 	bool m_bOverrideFontRenderer;


	bool m_bRead_BannedListA[XUSER_MAX_COUNT];
	char m_pszUniqueMapName[14];
	bool m_BanListCheck[XUSER_MAX_COUNT];

public:
	void SetBanListCheck(int iPad,bool bVal) {m_BanListCheck[iPad]=bVal;}
	bool GetBanListCheck(int iPad)			{ return m_BanListCheck[iPad];}
	// AUTOSAVE
public:
	void SetAutosaveTimerTime(void);
	bool AutosaveDue(void);
	unsigned int SecondsToAutosave();
private:
	unsigned int m_uiAutosaveTimer;
	unsigned int m_uiOpacityCountDown[XUSER_MAX_COUNT];

	// DLC
	bool m_bNewDLCAvailable;
	bool m_bSeenNewDLCTip;

	// Host options
private:
	unsigned int m_uiGameHostSettings;
	static unsigned char m_szPNG[8];

#ifdef _LARGE_WORLDS
	unsigned int	m_GameNewWorldSize;
	bool			m_bGameNewWorldSizeUseMoat;
	unsigned int	m_GameNewHellScale;
#endif
	unsigned int	FromBigEndian(unsigned int uiValue);

public:


	void			SetGameHostOption(eGameHostOption eVal,unsigned int uiVal);
	void			SetGameHostOption(unsigned int &uiHostSettings, eGameHostOption eVal,unsigned int uiVal);
	unsigned int	GetGameHostOption(eGameHostOption eVal);
	unsigned int	GetGameHostOption(unsigned int uiHostSettings, eGameHostOption eVal);

#ifdef _LARGE_WORLDS
	void			SetGameNewWorldSize(unsigned int newSize, bool useMoat) { m_GameNewWorldSize = newSize; m_bGameNewWorldSizeUseMoat = useMoat; }
	unsigned int	GetGameNewWorldSize()									{ return m_GameNewWorldSize; }
	unsigned int	GetGameNewWorldSizeUseMoat()							{ return m_bGameNewWorldSizeUseMoat; }
	void			SetGameNewHellScale(unsigned int newScale)				{ m_GameNewHellScale = newScale; }
	unsigned int	GetGameNewHellScale()									{ return m_GameNewHellScale; }
#endif
	void			SetResetNether(bool bResetNether) {m_bResetNether=bResetNether;}
	bool			GetResetNether() {return m_bResetNether;}
	bool			CanRecordStatsAndAchievements();

	// World seed from png image
	void GetImageTextData(PBYTE pbImageData, DWORD dwImageBytes,unsigned char *pszSeed,unsigned int &uiHostOptions,bool &bHostOptionsRead,DWORD &uiTexturePack);
	unsigned int CreateImageTextData(PBYTE bTextMetadata, int64_t seed, bool hasSeed, unsigned int uiHostOptions, unsigned int uiTexturePackId);

	// Game rules
	GameRuleManager m_gameRules;

public:
	void processSchematics(LevelChunk *levelChunk);
	void processSchematicsLighting(LevelChunk *levelChunk);
	void loadDefaultGameRules();
	vector<LevelGenerationOptions *> *getLevelGenerators() { return m_gameRules.getLevelGenerators(); }
	void setLevelGenerationOptions(LevelGenerationOptions *levelGen);
	LevelRuleset *getGameRuleDefinitions() { return m_gameRules.getGameRuleDefinitions(); }
	LevelGenerationOptions *getLevelGenerationOptions() { return m_gameRules.getLevelGenerationOptions(); }
	LPCWSTR	GetGameRulesString(const wstring &key);

private:
	BYTE m_playerColours[MINECRAFT_NET_MAX_PLAYERS]; // An array of QNet small-id's
	unsigned int m_playerGamePrivileges[MINECRAFT_NET_MAX_PLAYERS];

public:
	void UpdatePlayerInfo(BYTE networkSmallId, SHORT playerColourIndex, unsigned int playerGamePrivileges);
	short GetPlayerColour(BYTE networkSmallId);
	unsigned int GetPlayerPrivileges(BYTE networkSmallId);

	wstring getEntityName(eINSTANCEOF type);



	unsigned int	AddDLCRequest(eDLCMarketplaceType eContentType, bool bPromote=false);
	bool			RetrieveNextDLCContent();
	bool			CheckTMSDLCCanStop();
	static int		DLCOffersReturned(void *pParam, int iOfferC, DWORD dwType, int iPad);
	DWORD			GetDLCContentType(eDLCContentType eType) { return m_dwContentTypeA[eType];}
	eDLCContentType	Find_eDLCContentType(DWORD dwType);
	int				GetDLCOffersCount()	{ return m_iDLCOfferC;}
	bool			DLCContentRetrieved(eDLCMarketplaceType eType);
	void			TickDLCOffersRetrieved();
	void			ClearAndResetDLCDownloadQueue();
	bool			RetrieveNextTMSPPContent();
	void			TickTMSPPFilesRetrieved();
	void			ClearTMSPPFilesRetrieved();
	unsigned int	AddTMSPPFileTypeRequest(eDLCContentType eType, bool bPromote=false);
	int				GetDLCInfoTexturesOffersCount();
#if defined( __PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	DLC_INFO *GetDLCInfo(int iIndex);
	DLC_INFO *GetDLCInfo(char *);
	DLC_INFO *GetDLCInfoFromTPackID(int iTPID);
	bool GetDLCNameForPackID(const int iPackID,char **ppchKeyID);
	char * GetDLCInfoTextures(int iIndex);
	int GetDLCInfoCount();
#else

#ifdef _XBOX_ONE
	static int		TMSPPFileReturned(LPVOID pParam,int iPad,int iUserData,LPVOID, WCHAR *wchFilename);
	unordered_map<wstring,DLC_INFO * > *GetDLCInfo();
#else
	static int		TMSPPFileReturned(LPVOID pParam,int iPad,int iUserData,C4JStorage::PTMSPP_FILEDATA pFileData, LPCSTR szFilename);
#endif
	DLC_INFO *GetDLCInfoTrialOffer(int iIndex);
	DLC_INFO *GetDLCInfoFullOffer(int iIndex);

	int				GetDLCInfoTrialOffersCount();
	int				GetDLCInfoFullOffersCount();
#ifdef _XBOX_ONE
	bool GetDLCFullOfferIDForPackID(const int iPackID,wstring &wsProductId);
	wstring GetDLCInfoTexturesFullOffer(int iIndex);

#else
	bool GetDLCFullOfferIDForPackID(const int iPackID,ULONGLONG *pullVal);
	ULONGLONG GetDLCInfoTexturesFullOffer(int iIndex);
#endif
#endif

	void SetCorruptSaveDeleted(bool bVal) {m_bCorruptSaveDeleted=bVal;}
	bool GetCorruptSaveDeleted(void) {return m_bCorruptSaveDeleted;}

	void EnterSaveNotificationSection();
	void LeaveSaveNotificationSection();
private:
	CRITICAL_SECTION m_saveNotificationCriticalSection;
	int m_saveNotificationDepth;
	// Download Status

	//Request current_download;
	vector<DLCRequest *> m_DLCDownloadQueue;
	vector<TMSPPRequest *> m_TMSPPDownloadQueue;
	static DWORD m_dwContentTypeA[e_Marketplace_MAX];
	int m_iDLCOfferC;
	bool m_bAllDLCContentRetrieved;
	bool m_bAllTMSContentRetrieved;
	bool m_bTickTMSDLCFiles;
	CRITICAL_SECTION csDLCDownloadQueue;
	CRITICAL_SECTION csTMSPPDownloadQueue;
	CRITICAL_SECTION csAdditionalModelParts;
	CRITICAL_SECTION csAdditionalSkinBoxes;
	CRITICAL_SECTION csAnimOverrideBitmask;
	bool m_bCorruptSaveDeleted;

	DWORD m_dwAdditionalModelParts[XUSER_MAX_COUNT];

	BYTE *m_pBannedListFileBuffer;
	DWORD m_dwBannedListFileSize;

public:
	DWORD m_dwDLCFileSize;
	BYTE *m_pDLCFileBuffer;

	// 	static int CallbackReadXuidsFileFromTMS(LPVOID lpParam, WCHAR *wchFilename, int iPad, bool bResult, int iAction);
	// 	static int CallbackDLCFileFromTMS(LPVOID lpParam, WCHAR *wchFilename, int iPad, bool bResult, int iAction);
	// 	static int CallbackBannedListFileFromTMS(LPVOID lpParam, WCHAR *wchFilename, int iPad, bool bResult, int iAction);

	// Storing additional model parts per skin texture
	void SetAdditionalSkinBoxes(DWORD dwSkinID, SKIN_BOX *SkinBoxA, DWORD dwSkinBoxC);
	vector<ModelPart *> * SetAdditionalSkinBoxes(DWORD dwSkinID, vector<SKIN_BOX *> *pvSkinBoxA);
	vector<ModelPart *> *GetAdditionalModelParts(DWORD dwSkinID);
	vector<SKIN_BOX *> *GetAdditionalSkinBoxes(DWORD dwSkinID);
	void SetAnimOverrideBitmask(DWORD dwSkinID,unsigned int uiAnimOverrideBitmask);
	unsigned int GetAnimOverrideBitmask(DWORD dwSkinID);

	static DWORD getSkinIdFromPath(const wstring &skin);
	static wstring getSkinPathFromId(DWORD skinId);

	virtual int LoadLocalTMSFile(WCHAR *wchTMSFile)=0;
	virtual int LoadLocalTMSFile(WCHAR *wchTMSFile, eFileExtensionType eExt)=0;
	virtual void FreeLocalTMSFiles(eTMSFileType eType)=0;
	virtual int GetLocalTMSFileIndex(WCHAR *wchTMSFile,bool bFilenameIncludesExtension,eFileExtensionType eEXT)=0;

	virtual bool GetTMSGlobalFileListRead() { return true;}
	virtual bool GetTMSDLCInfoRead() { return true;}
	virtual bool GetTMSXUIDsFileRead() { return true;}

	bool GetBanListRead(int iPad) { return m_bRead_BannedListA[iPad];}
	void SetBanListRead(int iPad,bool bVal) { m_bRead_BannedListA[iPad]=bVal;}
	void ClearBanList(int iPad) { BannedListA[iPad].pBannedList=nullptr;BannedListA[iPad].dwBytes=0;}

	DWORD GetRequiredTexturePackID()	{return m_dwRequiredTexturePackID;}
	void SetRequiredTexturePackID(DWORD dwID)	{m_dwRequiredTexturePackID=dwID;}

	virtual void GetFileFromTPD(eTPDFileType eType,PBYTE pbData,DWORD dwBytes,PBYTE *ppbData,DWORD *pdwBytes ) {*ppbData = nullptr; *pdwBytes = 0;}

	//XTITLE_DEPLOYMENT_TYPE getDeploymentType() { return m_titleDeploymentType; }

private:
	// vector of additional skin model parts, indexed by the skin texture id
	unordered_map<DWORD, vector<ModelPart *> *> m_AdditionalModelParts;
	unordered_map<DWORD, vector<SKIN_BOX *> *> m_AdditionalSkinBoxes;
	unordered_map<DWORD, unsigned int> m_AnimOverrides;


	bool m_bResetNether;
	DWORD m_dwRequiredTexturePackID;
#ifdef _XBOX_ONE
	vector <PBYTE> m_vTMSPPData;
#endif

#if ( defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined __PSVITA__)
	C4JStorage::eOptionsCallback m_eOptionsStatusA[XUSER_MAX_COUNT];

#ifdef __ORBIS__
	int m_eOptionsBlocksRequiredA[XUSER_MAX_COUNT];
#endif
#endif


	// 4J-PB - language and locale functions
public:

	void LocaleAndLanguageInit();
	void getLocale(vector<wstring> &vecWstrLocales);
	DWORD get_eMCLang(WCHAR *pwchLocale);
	DWORD get_xcLang(WCHAR *pwchLocale);

	void SetTickTMSDLCFiles(bool bVal);

	wstring getFilePath(DWORD packId, wstring filename, bool bAddDataFolder, wstring mountPoint = L"TPACK:");

private:
	unordered_map<int, wstring>m_localeA;
	unordered_map<wstring, int>m_eMCLangA;
	unordered_map<wstring, int>m_xcLangA;
	wstring getRootPath(DWORD packId, bool allowOverride, bool bAddDataFolder, wstring mountPoint);
public:

#ifdef _XBOX
	// 	bool m_bTransferSavesToXboxOne;
	// 	unsigned int m_uiTransferSlotC;

#elif defined (__PS3__)

#elif defined _DURANGO

#elif defined _WINDOWS64
	//CMinecraftAudio audio;
#else // PS4

#endif

#ifdef _XBOX_ONE
public:
	void SetReachedMainMenu();
	bool HasReachedMainMenu();
private:
	bool m_hasReachedMainMenu;
#endif
};

//singleton
//extern CMinecraftApp app;
