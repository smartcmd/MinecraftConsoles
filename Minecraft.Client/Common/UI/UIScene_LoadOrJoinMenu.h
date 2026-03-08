#pragma once

#include "UIScene.h"

class LevelGenerationOptions;


#if defined __PS3__ || defined __ORBIS__ || defined(__PSVITA__)
#define SONY_REMOTE_STORAGE_DOWNLOAD
#endif 
#if defined __PS3__ || __PSVITA__
#define SONY_REMOTE_STORAGE_UPLOAD
#endif


class UIScene_LoadOrJoinMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_SavesList,
		eControl_GamesList,
#if defined(_XBOX_ONE) || defined(__ORBIS__)
		eControl_SpaceIndicator,
#endif
	};

	enum EState
	{
		e_SavesIdle,
		e_SavesRepopulate,
		e_SavesRepopulateAfterMashupHide,
		e_SavesRepopulateAfterDelete,
		e_SavesRepopulateAfterTransferDownload,
	};

	enum eActions
	{
		eAction_None=0,
		eAction_ViewInvites,
		eAction_JoinGame,
	};
	eActions m_eAction;

	static const int JOIN_LOAD_CREATE_BUTTON_INDEX = 0;

	SaveListDetails *m_saveDetails;
	int m_iSaveDetailsCount;

protected:
	UIControl_SaveList m_buttonListSaves;
	UIControl_SaveList m_buttonListGames;
	UIControl_Label m_labelSavesListTitle, m_labelJoinListTitle, m_labelNoGames;
	UIControl m_controlSavesTimer, m_controlJoinTimer;
#if defined(_XBOX_ONE) || defined(__ORBIS__)
	UIControl_SpaceIndicatorBar m_spaceIndicatorSaves;
#endif

private:
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)		
		UI_MAP_ELEMENT( m_buttonListSaves, "SavesList")
		UI_MAP_ELEMENT( m_buttonListGames, "JoinList")

		UI_MAP_ELEMENT( m_labelSavesListTitle, "SavesListTitle")
		UI_MAP_ELEMENT( m_labelJoinListTitle, "JoinListTitle")
		UI_MAP_ELEMENT( m_labelNoGames, "NoGames")

		UI_MAP_ELEMENT( m_controlSavesTimer, "SavesTimer")
		UI_MAP_ELEMENT( m_controlJoinTimer, "JoinTimer")

#if defined(_XBOX_ONE) || defined(__ORBIS__)
		UI_MAP_ELEMENT( m_spaceIndicatorSaves, "SaveSizeBar")
#endif
	UI_END_MAP_ELEMENTS_AND_NAMES()

	int m_iDefaultButtonsC;
	int m_iMashUpButtonsC;
	int m_iState;

	vector<FriendSessionInfo *> *m_currentSessions;
	vector<LevelGenerationOptions *> m_generators;
	vector<File *> *m_saves;
	
	bool m_bIgnoreInput;
	bool m_bAllLoaded;
	bool m_bRetrievingSaveThumbnails;
	bool m_bSaveThumbnailReady;
	bool m_bShowingPartyGamesOnly;
	bool m_bInParty;
	JoinMenuInitData *m_initData;
	bool m_bMultiplayerAllowed;
	int m_iTexturePacksNotInstalled;
	int m_iRequestingThumbnailId;
	SAVE_DETAILS *m_pSaveDetails;
	bool m_bSavesDisplayed;
	bool m_bExitScene;
	bool m_bCopying;
	bool m_bCopyingCancelled;
	int m_iSaveInfoC;
	int m_iSaveListIndex;
	int m_iGameListIndex;
	//int *m_iConfigA; // track the texture packs that we don't have installed
#ifndef _XBOX_ONE
	bool m_bSaveTransferInProgress;
	bool m_bSaveTransferCancelled;
#endif
	bool m_bUpdateSaveSize;

public:
	UIScene_LoadOrJoinMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_LoadOrJoinMenu();
	
	virtual void updateTooltips();
	virtual void updateComponents();

	virtual void handleDestroy();
	virtual void handleLoseFocus();
	virtual void handleGainFocus(bool navBack);
	virtual void handleTimerComplete(int id);
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);
	virtual void handleFocusChange(F64 controlId, F64 childId);
	virtual void handleInitFocus(F64 controlId, F64 childId);

	virtual EUIScene getSceneType() { return eUIScene_LoadOrJoinMenu;}

	static void UpdateGamesListCallback(LPVOID pParam);
#ifdef _XBOX_ONE
	void HandleDLCLicenseChange();
#endif
	virtual void tick();

private:
	void Initialise();
	void GetSaveInfo();
	void UpdateGamesList();
	void AddDefaultButtons();
	bool DoesSavesListHaveFocus();
	bool DoesMashUpWorldHaveFocus();
	bool DoesGamesListHaveFocus();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:

	static int LoadSaveDataThumbnailReturned(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes);
	static int LoadSaveCallback(LPVOID lpParam,bool bRes);
	static int DeleteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int SaveOptionsDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int TexturePackDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);	
	static int DeleteSaveDataReturned(LPVOID lpParam,bool bRes);
	static int RenameSaveDataReturned(LPVOID lpParam,bool bRes);
	static int KeyboardCompleteWorldNameCallback(LPVOID lpParam,bool bRes);
#ifdef __PSVITA__
	static int MustSignInTexturePack(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int MustSignInReturnedTexturePack(void *pParam,bool bContinue, int iPad);
	static int SignInAdhocReturned(void *pParam,bool bContinue, int iPad);
#endif

protected:
	void handlePress(F64 controlId, F64 childId);
	void LoadLevelGen(LevelGenerationOptions *levelGen);
	void LoadSaveFromDisk(File *saveFile, ESavePlatform savePlatform = SAVE_FILE_PLATFORM_LOCAL);
#if defined(__PS3__) || defined(__PSVITA__) || defined(__ORBIS__)
	void LoadSaveFromCloud();
#endif
public:
	virtual void HandleDLCMountingComplete();

#ifdef __ORBIS__
	void LoadRemoteFileFromDisk(char* remoteFilename);
#endif

private:
	void CheckAndJoinGame(int gameIndex);

#ifdef _WINDOWS64
	static const int ADD_SERVER_BUTTON_INDEX = 0;
	enum eAddServerPhase { eAddServer_Idle, eAddServer_IP, eAddServer_Port, eAddServer_Name };
	eAddServerPhase m_addServerPhase;
	wstring m_addServerIP;
	wstring m_addServerPort;
	void BeginAddServer();
	void AppendServerToFile(const wstring& ip, const wstring& port, const wstring& name);
	static int AddServerKeyboardCallback(LPVOID lpParam, bool bRes);
#endif

#if defined(__PS3__) || defined(__PSVITA__) || defined(__ORBIS__)
	static int MustSignInReturnedPSN(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int PSN_SignInReturned(void *pParam,bool bContinue, int iPad);
	static void remoteStorageGetSaveCallback(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code);
#endif
 
#ifdef __ORBIS__
	//static int PSPlusReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
#endif
#ifdef _XBOX_ONE
	typedef struct _SaveTransferStateContainer
	{
		int m_iProgress;
		bool m_bSaveTransferInProgress;
		bool m_bSaveTransferCancelled;
		int m_iPad;
		C4JStorage::eSaveTransferState m_eSaveTransferState;
		UIScene_LoadOrJoinMenu *m_pClass;
	} SaveTransferStateContainer;
	enum ESaveTransferFiles
	{
		eSaveTransferFile_Marker,
		eSaveTransferFile_Metadata,
		eSaveTransferFile_SaveData,
	};
	static ESaveTransferFiles s_eSaveTransferFile;
	static unsigned long s_ulFileSize;
	static byteArray s_transferData;
	static wstring m_wstrStageText;
	LoadMenuInitData *m_loadMenuInitData;

#ifdef _DEBUG_MENUS_ENABLED
	static C4JStorage::SAVETRANSFER_FILE_DETAILS m_debugTransferDetails;
#endif

	void LaunchSaveTransfer();
	static int DownloadXbox360SaveThreadProc( LPVOID lpParameter );
	static void RequestFileSize( SaveTransferStateContainer *pClass, wchar_t *filename );
	static void RequestFileData( SaveTransferStateContainer *pClass, wchar_t *filename );
	static int SaveTransferReturned(LPVOID lpParam,C4JStorage::SAVETRANSFER_FILE_DETAILS *pSaveTransferDetails);
	static int SaveTransferUpdateProgress(LPVOID lpParam,unsigned long ulBytesReceived);
	static void CancelSaveTransferCallback(LPVOID lpParam);
	static int NeedSyncMessageReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int CancelSaveTransferCompleteCallback(LPVOID lpParam);

#endif



#ifdef SONY_REMOTE_STORAGE_DOWNLOAD
	enum eSaveTransferState
	{
		eSaveTransfer_Idle,
		eSaveTransfer_Busy,
		eSaveTransfer_GetRemoteSaveInfo,
		eSaveTransfer_GettingRemoteSaveInfo,
		eSaveTransfer_CreateDummyFile,
		eSaveTransfer_CreatingDummyFile,
		eSaveTransfer_GettingFileSize,
		eSaveTransfer_FileSizeRetrieved,
		eSaveTransfer_GetFileData,
		eSaveTransfer_GettingFileData,
		eSaveTransfer_FileDataRetrieved,
		eSaveTransfer_GetSavesInfo,
		eSaveTransfer_GettingSavesInfo,
		eSaveTransfer_LoadSaveFromDisc,
		eSaveTransfer_LoadingSaveFromDisc,
		eSaveTransfer_CreatingNewSave,
		eSaveTransfer_Converting,
		eSaveTransfer_Saving,
		eSaveTransfer_Succeeded,
		eSaveTransfer_Cancelled,
		eSaveTransfer_Error,
		eSaveTransfer_ErrorDeletingSave,
		eSaveTransfer_ErrorMesssage,
		eSaveTransfer_Finished,

	};
	eSaveTransferState m_eSaveTransferState;
	static unsigned long m_ulFileSize;
	static wstring m_wstrStageText;
	static bool		m_bSaveTransferRunning;
	int m_iProgress;
	char m_downloadedUniqueFilename[64];//SCE_SAVE_DATA_DIRNAME_DATA_MAXSIZE];
	bool m_saveTransferDownloadCancelled;
	void LaunchSaveTransfer();
	static int CreateDummySaveDataCallback(LPVOID lpParam,bool bRes);
	static int CrossSaveGetSavesInfoCallback(LPVOID lpParam, SAVE_DETAILS *pSaveDetails,bool bRes);
	static int LoadCrossSaveDataCallback(void *pParam,bool bIsCorrupt, bool bIsOwner);
	static int CrossSaveFinishedCallback(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int CrossSaveDeleteOnErrorReturned(LPVOID lpParam,bool bRes);
	static int RemoteSaveNotFoundCallback(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int DownloadSonyCrossSaveThreadProc( LPVOID lpParameter );
	static void SaveTransferReturned(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code);
	static ConsoleSaveFile* SonyCrossSaveConvert();

	static void CancelSaveTransferCallback(LPVOID lpParam);
public:
	static bool isSaveTransferRunning() { return m_bSaveTransferRunning; }
private:
#endif

#ifdef SONY_REMOTE_STORAGE_UPLOAD
	enum eSaveUploadState
	{
		eSaveUpload_Idle,
		eSaveUpload_UploadingFileData,
		eSaveUpload_FileDataUploaded,
		eSaveUpload_Cancelled,
		eSaveUpload_Error,
		esaveUpload_Finished
	};

	eSaveUploadState m_eSaveUploadState;
	bool m_saveTransferUploadCancelled;

	void LaunchSaveUpload();
	static int UploadSonyCrossSaveThreadProc( LPVOID lpParameter );
	static void SaveUploadReturned(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code);
	static void CancelSaveUploadCallback(LPVOID lpParam);
	static int SaveTransferDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int CrossSaveUploadFinishedCallback(void *pParam,int iPad,C4JStorage::EMessageResult result);
#endif

#if defined _XBOX_ONE || defined __ORBIS__
	static int CopySaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int CopySaveThreadProc( LPVOID lpParameter );
	static int CopySaveDataReturned( LPVOID lpParameter, bool success, C4JStorage::ESaveGameState state );
	static bool CopySaveDataProgress(LPVOID lpParam, int percent);
	static void CancelCopySaveCallback(LPVOID lpParam);
	static int CopySaveErrorDialogFinishedCallback(void *pParam,int iPad,C4JStorage::EMessageResult result);
#endif
};
