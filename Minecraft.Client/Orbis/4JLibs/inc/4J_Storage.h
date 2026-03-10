#pragma once
using namespace std;

#include <save_data.h>

#define MAX_DISPLAYNAME_LENGTH 128 // SCE_SAVE_DATA_SUBTITLE_MAXSIZE on PS4
#define MAX_SAVEFILENAME_LENGTH 32 // SCE_SAVE_DATA_DIRNAME_DATA_MAXSIZE
#define USER_INDEX_ANY 0x000000FF
#define RESULT	LONG

class StringTable;

typedef struct  
{
	char UTF8SaveFilename[MAX_SAVEFILENAME_LENGTH];
	char UTF8SaveTitle[MAX_DISPLAYNAME_LENGTH];
	time_t modifiedTime;
	PBYTE thumbnailData;
	unsigned int thumbnailSize;
	int blocksUsed;
	//int sizeKB;
}
SAVE_INFO,*PSAVE_INFO;

typedef struct  
{
	int totalBlocksUsed;
	int newSaveBlocksUsed;
	int iSaveC;
	PSAVE_INFO SaveInfoA;
	PSAVE_INFO pCurrentSaveInfo;		// Pointer to SAVE_INFO for the save that has just been loaded, or nullptr if no save has been loaded (ie this is a newly created game)
}
SAVE_DETAILS,*PSAVE_DETAILS;

class CONTENT_DATA
{
public:
	enum Type
	{
		e_contentLocked,
		e_contentUnlocked
	};
	int DeviceID;
	DWORD dwContentType;
	WCHAR wszDisplayName[256];
	CHAR szFileName[SCE_FIOS_PATH_MAX];
};

typedef CONTENT_DATA XCONTENT_DATA, *PXCONTENT_DATA;			// TODO - put back in when actually interfacing with game

#define MARKETPLACE_CONTENTOFFER_INFO int

// Current version of the dlc data creator
#define CURRENT_DLC_VERSION_NUM 3

// MGH - moved these here from Orbis_App.h
enum e_SONYDLCType
{
	eSONYDLCType_SkinPack=0,
	eSONYDLCType_TexturePack,
	eSONYDLCType_MashUpPack,
	eSONYDLCType_All
};

typedef struct
{
	char chDLCKeyname[16+1]; // extra 1 for terminator
	//char chDLCTitle[64];
	e_SONYDLCType eDLCType;
	char chDLCPicname[16+1]; // extra 1 for terminator
	int iFirstSkin;
	int iConfig; // used for texture pack data files
#ifdef __ORBIS__
	PBYTE pbImageData;
	DWORD dwImageBytes;
#endif
}
SONYDLC;


class C4JStorage
{
public:

	struct PROFILESETTINGS
	{
		int iYAxisInversion;
		int iControllerSensitivity;
		int iVibration;
		bool bSwapSticks;
	};

	// Structs defined in the DLC_Creator, but added here to be used in the app
	typedef struct  
	{
		unsigned int	uiFileSize;
		DWORD			dwType;
		DWORD			dwWchCount; // count of WCHAR in next array
		WCHAR			wchFile[1];
	}
	DLC_FILE_DETAILS, *PDLC_FILE_DETAILS;

	typedef struct
	{
		DWORD	dwType;
		DWORD	dwWchCount; // count of WCHAR in next array;
		WCHAR	wchData[1]; // will be an array of size dwBytes
	}
	DLC_FILE_PARAM, *PDLC_FILE_PARAM;
	// End of DLC_Creator structs

	typedef struct  
	{
		DWORD dwVersion;
		DWORD dwNewOffers;
		DWORD dwTotalOffers;
		DWORD dwInstalledTotalOffers;
		BYTE bPadding[1024-sizeof(DWORD)*4]; // future expansion
	}
	DLC_TMS_DETAILS;

	typedef struct  
	{
		DWORD dwSize;
		PBYTE pbData;
	}
	TMSPP_FILEDATA, *PTMSPP_FILEDATA;

	enum eTMS_FILETYPEVAL
	{
		TMS_FILETYPE_BINARY=0,
		TMS_FILETYPE_CONFIG=1,
		TMS_FILETYPE_JSON=2,
		TMS_FILETYPE_MAX,
	};

	enum eGlobalStorage
	{
		//eGlobalStorage_GameClip=0,
		eGlobalStorage_Title=0,
		eGlobalStorage_TitleUser,
		eGlobalStorage_Max
	};

	enum EMessageResult
	{
		EMessage_Undefined=0,
		EMessage_Busy,
		EMessage_Pending,
		EMessage_Cancelled,
		EMessage_ResultAccept,
		EMessage_ResultDecline,
		EMessage_ResultThirdOption,
		EMessage_ResultFourthOption
	};

	enum ESaveGameState
	{
		ESaveGame_Idle=0,

		ESaveGame_Save,	
		ESaveGame_SaveCompleteSuccess,
		ESaveGame_SaveCompleteFail,
		ESaveGame_SaveIncomplete,
		ESaveGame_SaveIncomplete_WaitingOnResponse,

		ESaveGame_SaveSubfiles,	
		ESaveGame_SaveSubfilesCompleteSuccess,
		ESaveGame_SaveSubfilesCompleteFail,
		ESaveGame_SaveSubfilesIncomplete,
		ESaveGame_SaveSubfilesIncomplete_WaitingOnResponse,

		ESaveGame_Load,
		ESaveGame_LoadCompleteSuccess,
		ESaveGame_LoadCompleteFail,

		ESaveGame_Delete,
		ESaveGame_DeleteSuccess,
		ESaveGame_DeleteFail,

		ESaveGame_Rename,
		ESaveGame_RenameSuccess,
		ESaveGame_RenameFail,

		ESaveGame_Copy,
		ESaveGame_CopyCompleteSuccess,
		ESaveGame_CopyCompleteFail,
		ESaveGame_CopyCompleteFailQuota,
		ESaveGame_CopyCompleteFailLocalStorage,

		ESaveGame_GetSaveThumbnail		// Not used as an actual state in the PS4, but the game expects this to be returned to indicate success when getting a thumbnail
	};

	enum EOptionsState
	{
		EOptions_Idle=0,
		EOptions_Save,	
		EOptions_Load,	
		EOptions_Delete,	
		EOptions_NoSpace,	
		EOptions_Corrupt,	
	};

	enum ESaveGameStatus
	{
		EDeleteGame_Idle=0,
		EDeleteGame_InProgress,
	};

	enum EDLCStatus
	{
		EDLC_Error=0,
		EDLC_Idle,
		EDLC_NoOffers,
		EDLC_AlreadyEnumeratedAllOffers,
		EDLC_NoInstalledDLC,
		EDLC_Pending,
		EDLC_LoadInProgress,
		EDLC_Loaded,
		EDLC_ChangedDevice
	};

	enum ESavingMessage
	{
		ESavingMessage_None=0,
		ESavingMessage_Short,
		ESavingMessage_Long
	};

	enum ESaveIncompleteType
	{
		ESaveIncomplete_None,
		ESaveIncomplete_OutOfQuota,
		ESaveIncomplete_OutOfLocalStorage,
		ESaveIncomplete_Unknown
	};

	enum ETMSStatus
	{
		ETMSStatus_Idle=0,
		ETMSStatus_Fail,
		ETMSStatus_ReadInProgress,
		ETMSStatus_ReadFileListInProgress,
		ETMSStatus_WriteInProgress,
		ETMSStatus_Fail_ReadInProgress,
		ETMSStatus_Fail_ReadFileListInProgress,
		ETMSStatus_Fail_ReadDetailsNotRetrieved,
		ETMSStatus_Fail_WriteInProgress,
		ETMSStatus_DeleteInProgress,
		ETMSStatus_Pending,
	};

	enum eTMS_FileType
	{
		eTMS_FileType_Normal=0,
		eTMS_FileType_Graphic,
	};

	enum ESGIStatus
	{
		ESGIStatus_Error=0,
		ESGIStatus_Idle,
		ESGIStatus_ReadInProgress,
		ESGIStatus_NoSaves,
	};

	enum
	{
		PROFILE_READTYPE_ALL,
		PROFILE_READTYPE_XBOXSETTINGS // just read the settings (after a notification of settings change)
	};

	enum eOptionsCallback
	{
		eOptions_Callback_Idle,
		eOptions_Callback_Write,
		eOptions_Callback_Write_Fail_NoSpace,
		eOptions_Callback_Write_Fail,
		eOptions_Callback_Read,		
		eOptions_Callback_Read_Fail,		
		eOptions_Callback_Read_FileNotFound,		
		eOptions_Callback_Read_Corrupt,		
		eOptions_Callback_Read_CorruptDeletePending,
		eOptions_Callback_Read_CorruptDeleted
	};

	///////////////////////////////////////////////////////////////////////////// Global storage manager //////////////////////////////////////////////////////////////////////////////

	C4JStorage();
	void						Tick(void);																																			// General storage manager tick to be called from game
	
	///////////////////////////////////////////////////////////////////////////// Savegame data ///////////////////////////////////////////////////////////////////////////////////////

	// Initialisation
	void						Init(unsigned int uiSaveVersion,LPCWSTR pwchDefaultSaveName,char *pszSavePackName,int iMinimumSaveSize,												// General manager initialisation
									 int( *Func)(LPVOID, const ESavingMessage, int),LPVOID lpParam,LPCSTR szGroupID);																		
	void						SetGameSaveFolderTitle(WCHAR *wszGameSaveFolderTitle);																								// Sets the title to be set in the param.sfo of saves (this doesn't vary, the sub-title is used for the user cho
	void						SetSaveCacheFolderTitle(WCHAR *wszSaveCacheFolderTitle);																							// Sets the title to be set in the param.sfo of the save cache
	void						SetOptionsFolderTitle(WCHAR *wszOptionsFolderTitle);		
	void						SetCorruptSaveName(WCHAR *wszCorruptSaveName);		
	// Sets the title to be set in the param.sfo of the options file
	void						SetGameSaveFolderPrefix(char *szGameSaveFolderPrefix);																								// Sets the prefix to be added to the unique filename of each save to construct a final folder name
	void						SetMaxSaves(int iMaxC);																																// Sets the maximum number of saves to be evaluated by GetSavesInfo etc.
	void						SetDefaultImages(PBYTE pbOptionsImage,DWORD dwOptionsImageBytes,PBYTE pbSaveImage,DWORD dwSaveImageBytes,											// Sets default save image and thumbnail, which can be used when saving a game that hasn't generated any yet
												 PBYTE pbSaveThumbnail,DWORD dwSaveThumbnailBytes);	
	void						SetIncompleteSaveCallback(void( *Func)(LPVOID, const ESaveIncompleteType, int blocksRequired), LPVOID param);										// Sets callback to be used in the event of a save method not being able to complete

	// Miscellaneous control
	void						SetSaveDisabled(bool bDisable);																														// Sets saving disabled/enabled state
	bool						GetSaveDisabled(void);																																// Determines whether saving has been disabled
	void						ResetSaveData();																																	// Releases any internal storage being held for previously saved/loaded data
	C4JStorage::ESaveGameState	DoesSaveExist(bool *pbExists);																														// Determine if current savegame exists on storage device
	bool						EnoughSpaceForAMinSaveGame();
	C4JStorage::ESaveGameState	GetSaveState();

	// Get details of existing savedata
	C4JStorage::ESaveGameState	GetSavesInfo(int iPad,int ( *Func)(LPVOID lpParam,SAVE_DETAILS *pSaveDetails,const bool),LPVOID lpParam,char *pszSavePackName);						// Start search
	PSAVE_DETAILS				ReturnSavesInfo();																																	// Returns result of search (or nullptr if not yet received)
	void						ClearSavesInfo();																																	// Clears results
	C4JStorage::ESaveGameState	LoadSaveDataThumbnail(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes), LPVOID lpParam);					// Get the thumbnail for an individual save referenced by pSaveInfo

	// Loading savedata & obtaining information from just-loaded file
	C4JStorage::ESaveGameState	LoadSaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,const bool, const bool), LPVOID lpParam);												// Loads savedata referenced by pSaveInfo, calls callback once complete
	unsigned int				GetSaveSize();																																		// Obtains sizse of just-loaded save
	void						GetSaveData(void *pvData,unsigned int *puiBytes);																									// Obtains pointer to, and size, of just-loaded save
	bool						GetSaveUniqueNumber(INT *piVal);																													// Gets the unique numeric portion of the folder name used for the save (encodes m
	bool						GetSaveUniqueFilename(char *pszName);																												// Get the full unique "filename" used as part of the folder name for the savev
	void						SetSaveUniqueFilename(char *szFilename);
	bool						GetSaveUniqueFileDir(char *pszName);																												// Get the full unique "filename" used as part of the folder name for the save

	bool						SaveGameDirMountExisting(char* pMountPoint);
	bool						SaveGameDirUnMount(const char* pMountPoint);


	// Handling of sub-files (numerically indexed ) within the save
	unsigned int				GetSubfileCount();																																	// Get the number of sub-files in the just-loaded save
	void						ResetSubfiles();																																	// Reset storage manager's references to sub-files, to be called when we are finished with the save game
	void						GetSubfileDetails(int idx, unsigned int *subfileId, unsigned char **data, unsigned int *sizeOut);													// Gets details for sub-file from 0 to GetSubfileCount() - 1. Caller is responsible for data allocation after this point.
	void						UpdateSubfile(int idx, unsigned char *data, unsigned int size);																						// Update internal details for a sub-file, from 0 to GetSubfileCount() - 1.
	int							AddSubfile(unsigned int subfileId);																													// Adds an additional sub-file with a given subfileId, returns index
	C4JStorage::ESaveGameState	SaveSubfiles(int( *Func)(LPVOID ,const bool),LPVOID lpParam);																						// Writes all modified sub-files, calling callback on completion

	// Saving savedata
	void						SetSaveTitle(const wchar_t *UTF16String);																											// Sets the name which is used as a sub-title in the savedata param.sfo
	void						SetSaveTitleExtraFileSuffix(const wchar_t *UTF16String);
	PVOID						AllocateSaveData(unsigned int uiBytes);																												// Allocate storage manager owned memory to the data which is to be saved to
	void						SetSaveDataSize(unsigned int uiBytes);																												// Set the actual size of data to be saved
	void						GetDefaultSaveImage(PBYTE *ppbSaveImage,DWORD *pdwSaveImageBytes);																					// Get the default save thumbnail (as set by SetDefaultImages) for use on saving games t
	void						GetDefaultSaveThumbnail(PBYTE *ppbSaveThumbnail,DWORD *pdwSaveThumbnailBytes);																		// Get the default save image (as set by SetDefaultImages) for use on saving games that 
	void						SetSaveImages( PBYTE pbThumbnail,DWORD dwThumbnailBytes,PBYTE pbImage,DWORD dwImageBytes, PBYTE pbTextData ,DWORD dwTextDataBytes);					// Sets the thumbnail & image for the save, optionally setting the metadata in the png
	C4JStorage::ESaveGameState	SaveSaveData(int( *Func)(LPVOID ,const bool),LPVOID lpParam);																						// Save the actual data, calling callback on completion

	// Handling of incomplete saves (either sub-files or save data). To be used after game has had callback for an incomplete save event
	void						ContinueIncompleteOperation();
	void						CancelIncompleteOperation();

	// Other file operations
	C4JStorage::ESaveGameState	DeleteSaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,const bool), LPVOID lpParam);															// Deletes savedata referenced by pSaveInfo, calls callback when comple
	C4JStorage::ESaveGameState	CopySaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID ,const bool,C4JStorage::ESaveGameState state),bool( *FuncProg)(LPVOID ,const int),LPVOID lpParam);	// Copies savedata referenced by pSaveInfo, calls callback when complete, calls another callback for progress & handling cancellation
	C4JStorage::ESaveGameState	RenameSaveData(int iRenameIndex,uint16_t*pui16NewName,int( *Func)(LPVOID lpParam,const bool), LPVOID lpParam);											// Renamed savedata with index from last established ReturnSavesInfo.
	
	// Internal methods
private:
	void						GetSaveImage(PBYTE *ppbSaveImage, int *puiSaveImageBytes);
	void						GetSaveThumbnail(PBYTE *ppbSaveThumbnail, int *puiSaveThumbnailBytes);

	///////////////////////////////////////////////////////////////////////////// Profile data ////////////////////////////////////////////////////////////////////////////////////////
public:
	// Initialisation
	void						InitialiseProfileData(unsigned short usProfileVersion, UINT uiProfileValuesC, UINT uiProfileSettingsC, DWORD *pdwProfileSettingsA, int iGameDefinedDataSizeX4, unsigned int *puiGameDefinedDataChangedBitmask);	// General initialisation
	int							SetDefaultOptionsCallback(int( *Func)(LPVOID,PROFILESETTINGS *, const int iPad),LPVOID lpParam);													// Set a callback that can initialise a profile's storage to its default settings
	void						SetOptionsDataCallback(int( *Func)(LPVOID,  int iPad, unsigned short usVersion, C4JStorage::eOptionsCallback,int),LPVOID lpParam);						// Sets callback that is called when status of any options has changed
	void						SetQuadrantUserIds(SceUserServiceUserId *aUserIds);																									// Set the user ids that we are referencing for each quadrant. Input is point to array of size MAX_SIGNED_IN_USERS.
	int							SetOldProfileVersionCallback(int( *Func)(LPVOID,unsigned char *, const unsigned short,const int),LPVOID lpParam);

	// Getting and setting of profile data
	PROFILESETTINGS *			GetDashboardProfileSettings(int iPad);																												// Get pointer to the standard (originally xbox dashboard) profile data for one user
	void						*GetGameDefinedProfileData(int iQuadrant);																											// Get pointer to the game-defined profile data for one user

	// Reading and writing profiles
	void						ReadFromProfile(int iQuadrant, int iReadType=PROFILE_READTYPE_ALL);																					// Initiate read profile data for one user - read type is ignored on this platform
	void						WriteToProfile(int iQuadrant, bool bGameDefinedDataChanged=false, bool bOverride5MinuteLimitOnProfileWrites=false);									// Initiate write profile for one user
	void						DeleteOptionsData(int iPad);																														// Delete profile data for one user
	void						ForceQueuedProfileWrites(int iPad=XUSER_INDEX_ANY);																									// Force any queued profile writes to write now

	///////////////////////////////////////////////////////////////////////////// Unimplemented stubs /////////////////////////////////////////////////////////////////////////////////
	void						SetSaveDeviceSelected(unsigned int uiPad,bool bSelected) {}
	bool						GetSaveDeviceSelected(unsigned int iPad) { return true; }
	void						ClearDLCOffers() {}
	C4JStorage::ETMSStatus		ReadTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,C4JStorage::eTMS_FileType eFileType, WCHAR *pwchFilename,BYTE **ppBuffer,DWORD *pdwBufferSize,int( *Func)(LPVOID, WCHAR *,int, bool, int),LPVOID lpParam, int iAction) { return C4JStorage::ETMSStatus_Idle; }
	bool						WriteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,WCHAR *pwchFilename,BYTE *pBuffer,DWORD dwBufferSize) { return true; }
	bool						DeleteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,WCHAR *pwchFilename) { return true; }
	C4JStorage::EDLCStatus		GetDLCOffers(int iPad,int( *Func)(LPVOID, int, DWORD, int),LPVOID lpParam, DWORD dwOfferTypesBitmaskT) { return C4JStorage::EDLC_Idle; }

	// DLC
	// DLC
	void						SetDLCPackageRoot(char *pszDLCRoot);
	EDLCStatus					GetInstalledDLC(int iPad,int( *Func)(LPVOID, int, int),LPVOID lpParam);
	CONTENT_DATA&				GetDLC(DWORD dw);
	DWORD						GetAvailableDLCCount( int iPad );
	DWORD						MountInstalledDLC(int iPad,DWORD dwDLC,int( *Func)(LPVOID, int, DWORD,DWORD),LPVOID lpParam,LPCSTR szMountDrive = nullptr);
	DWORD						UnmountInstalledDLC(LPCSTR szMountDrive = nullptr);
	void						GetMountedDLCFileList(const char* szMountDrive, std::vector<std::string>& fileList);
	std::string					GetMountedPath(std::string szMount);
	void						SetDLCProductCode(const char* szProductCode);
	void						SetProductUpgradeKey(const char* szKey);
	bool						CheckForTrialUpgradeKey(void( *Func)(LPVOID, bool),LPVOID lpParam);
	void						SetDLCInfoMap(unordered_map<wstring, SONYDLC *>* pSONYDLCMap);
};

extern C4JStorage StorageManager;
