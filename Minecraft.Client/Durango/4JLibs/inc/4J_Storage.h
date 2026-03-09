#pragma once
using namespace std;

#define MAX_DISPLAYNAME_LENGTH 128 // SCE_SAVE_DATA_SUBTITLE_MAXSIZE on PS4
#define MAX_SAVEFILENAME_LENGTH 32 // SCE_SAVE_DATA_DIRNAME_DATA_MAXSIZE
#define USER_INDEX_ANY 0x000000FF
#define RESULT	LONG

class StringTable;

typedef struct
{
	time_t			modifiedTime;
	unsigned int	dataSize;
	unsigned int	thumbnailSize;
}
CONTAINER_METADATA;

typedef struct  
{
	wchar_t UTF16SaveFilename[MAX_SAVEFILENAME_LENGTH];
	wchar_t UTF16SaveTitle[MAX_DISPLAYNAME_LENGTH];
	CONTAINER_METADATA metaData;
	PBYTE thumbnailData;
	uint64	totalSize;
	bool	needsSync;
	//int sizeKB;
}
SAVE_INFO,*PSAVE_INFO;

typedef struct  
{
	int iSaveC;
	int iThumbnailC;
	PSAVE_INFO SaveInfoA;
}
SAVE_DETAILS,*PSAVE_DETAILS;

class CONTENT_DATA
{
public:
	int DeviceID;
	DWORD dwContentType;
	WCHAR wszDisplayName[256];
	WCHAR wszProductID[64];
	bool bTrialLicense;
	//WCHAR szFileName[MAX_SAVEFILENAME_LENGTH];
};

typedef CONTENT_DATA XCONTENT_DATA, *PXCONTENT_DATA;

typedef struct _MARKETPLACE_CONTENTOFFER_INFO
{
	WCHAR *wszProductID;
	WCHAR *wszOfferName;
	BOOL fUserHasPurchased;
	WCHAR *wszSellText;
	WCHAR *wszCurrencyPrice;
	WCHAR *wszSignedOfferID;
} MARKETPLACE_CONTENTOFFER_INFO, *PMARKETPLACE_CONTENTOFFER_INFO;

typedef enum 
{
	XMARKETPLACE_OFFERING_TYPE_CONTENT = 0x00000002,
	//XMARKETPLACE_OFFERING_TYPE_GAME_DEMO = 0x00000020,
	//XMARKETPLACE_OFFERING_TYPE_GAME_TRAILER = 0x00000040,
	XMARKETPLACE_OFFERING_TYPE_THEME = 0x00000080,
	XMARKETPLACE_OFFERING_TYPE_TILE = 0x00000800,
	//XMARKETPLACE_OFFERING_TYPE_ARCADE = 0x00002000,
	//XMARKETPLACE_OFFERING_TYPE_VIDEO = 0x00004000,
	//XMARKETPLACE_OFFERING_TYPE_CONSUMABLE = 0x00010000,
	XMARKETPLACE_OFFERING_TYPE_AVATARITEM = 0x00100000
} XMARKETPLACE_OFFERING_TYPE;



enum eWebServiceState
{
	eWebService_idle,
	eWebService_notsignedin,
	eWebService_pending,
	eWebService_error,
	eWebService_busy
};

enum eTitleStorageState
{
	eTitleStorage_idle,
	eTitleStorage_notsignedin,
	eTitleStorage_pending,
	eTitleStorage_complete,
	eTitleStorage_readcomplete,
	eTitleStorage_readfilelistcomplete,
	eTitleStorage_writecomplete,
	eTitleStorage_deletecomplete,
	eTitleStorage_error,
	eTitleStorage_readerror,
	eTitleStorage_readfilelisterror,
	eTitleStorage_writeerror,
	eTitleStorage_deleteerror,
	eTitleStorage_busy
};

// Current version of the dlc data creator
#define CURRENT_DLC_VERSION_NUM 3

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

	typedef struct  
	{
		WCHAR				wchFilename[64];
		unsigned long		ulFileSize;
		eTMS_FILETYPEVAL	eFileTypeVal;
	}
	TMSPP_FILE_DETAILS, *PTMSPP_FILE_DETAILS;

	typedef struct  
	{
		int iCount;
		PTMSPP_FILE_DETAILS FileDetailsA;
	}
	TMSPP_FILE_LIST, *PTMSPP_FILE_LIST;

	typedef struct  
	{
		PBYTE pbData;
		unsigned long ulFileLen;
	}
	SAVETRANSFER_FILE_DETAILS;

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
		ESaveGame_LoadComplete,
		ESaveGame_EnumerateSubfiles,
		ESaveGame_EnumerateSubfilesComplete,
		ESaveGame_LoadSubfiles,
		ESaveGame_LoadCompleteSuccess,
		
		ESaveGame_LoadCompleteFail,

		ESaveGame_Delete,
		ESaveGame_DeleteSuccess,
		ESaveGame_DeleteFail,

		ESaveGame_Rename,
		ESaveGame_RenameSuccess,
		ESaveGame_RenameFail,

		ESaveGame_GetSaveThumbnail,
		ESaveGame_GetSaveThumbnailComplete,

		ESaveGame_Copy,
		ESaveGame_CopyCompleteSuccess,
		ESaveGame_CopyCompleteFail,
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

	enum eSaveTransferState
	{
		eSaveTransfer_Idle,
		eSaveTransfer_Busy,
		eSaveTransfer_GettingFileSize,
		eSaveTransfer_FileSizeRetrieved,
		eSaveTransfer_GetFileData,
		eSaveTransfer_GettingFileData,
		eSaveTransfer_FileDataRetrieved,
		eSaveTransfer_Converting,
		eSaveTransfer_Saving,
	};

	///////////////////////////////////////////////////////////////////////////// Global storage manager //////////////////////////////////////////////////////////////////////////////

	C4JStorage();
	void						Tick(void);																																			// General storage manager tick to be called from game
	
	///////////////////////////////////////////////////////////////////////////// Savegame data ///////////////////////////////////////////////////////////////////////////////////////

	// Initialisation
	void						Init(unsigned int uiSaveVersion,LPCWSTR pwchDefaultSaveName,char *pszSavePackName,int iMinimumSaveSize,												// General manager initialisation
									 int( *Func)(LPVOID, const ESavingMessage, int),LPVOID lpParam,
									 bool(*SetRetrieveProductIDFn)(XCONTENT_DATA &Data),WCHAR *pwchSCID,WCHAR *pwchTitleId);																		
	void						SetGameSaveFolderTitle(WCHAR *wszGameSaveFolderTitle);																								// Sets the title to be set in the param.sfo of saves (this doesn't vary, the sub-title is used for the user cho
	void						SetSaveCacheFolderTitle(WCHAR *wszSaveCacheFolderTitle);																							// Sets the title to be set in the param.sfo of the save cache
	void						SetOptionsFolderTitle(WCHAR *wszOptionsFolderTitle);																								// Sets the title to be set in the param.sfo of the options file
	void						SetGameSaveFolderPrefix(char *szGameSaveFolderPrefix);																								// Sets the prefix to be added to the unique filename of each save to construct a final folder name
	void						SetMaxSaves(int iMaxC);																																// Sets the maximum number of saves to be evaluated by GetSavesInfo etc.
	void						SetDefaultImages(PBYTE pbSaveThumbnail,DWORD dwSaveThumbnailBytes);																					// Sets default save thumbnail, which can be used when saving a game that hasn't generated any yet
	void						SetIncompleteSaveCallback(void( *Func)(LPVOID, const ESaveIncompleteType), LPVOID param);															// Sets callback to be used in the event of a save method not being able to complete

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
	C4JStorage::ESaveGameState	LoadSaveDataThumbnail(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes), LPVOID lpParam, bool force=false);	// Get the thumbnail for an individual save referenced by pSaveInfo

	// Loading savedata & obtaining information from just-loaded file
	C4JStorage::ESaveGameState	LoadSaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,const bool, const bool), LPVOID lpParam);												// Loads savedata referenced by pSaveInfo, calls callback once complete
	unsigned int				GetSaveSize();																																		// Obtains sizse of just-loaded save
	void						GetSaveData(void *pvData,unsigned int *puiBytes);																									// Obtains pointer to, and size, of just-loaded save
	bool						GetSaveUniqueNumber(INT *piVal);																													// Gets the unique numeric portion of the folder name used for the save (encodes m
	bool						GetSaveUniqueFilename(char *pszName);																												// Get the full unique "filename" used as part of the folder name for the save

	// Handling of sub-files (numerically indexed ) within the save
	unsigned int				GetSubfileCount();																																	// Get the number of sub-files in the just-loaded save
	void						ResetSubfiles();																																	// Reset storage manager's references to sub-files, to be called when we are finished with the save game
	void						GetSubfileDetails(int idx, unsigned int *subfileId, unsigned char **data, unsigned int *sizeOut);													// Gets details for sub-file from 0 to GetSubfileCount() - 1. Caller is responsible for data allocation after this point.
	void						UpdateSubfile(int idx, unsigned char *data, unsigned int size);																						// Update internal details for a sub-file, from 0 to GetSubfileCount() - 1.
	int							AddSubfile(unsigned int subfileId);																													// Adds an additional sub-file with a given subfileId, returns index
	C4JStorage::ESaveGameState	SaveSubfiles(int( *Func)(LPVOID ,const bool),LPVOID lpParam);																						// Writes all modified sub-files, calling callback on completion

	// Saving savedata
	void						SetSaveTitle(const wchar_t *UTF16String);																											// Sets the name which is used as a sub-title in the savedata param.sfo
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
	C4JStorage::ESaveGameState	DeleteSaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,const bool), LPVOID lpParam);														// Deletes savedata referenced by pSaveInfo, calls callback when comple
	C4JStorage::ESaveGameState	CopySaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID ,const bool,C4JStorage::ESaveGameState state),bool( *FuncProg)(LPVOID ,const int),LPVOID lpParam);								// Copies savedata referenced by pSaveInfo, calls callback when complete
	C4JStorage::ESaveGameState	RenameSaveData(int iRenameIndex,uint16_t*pui16NewName,int( *Func)(LPVOID lpParam,const bool), LPVOID lpParam);										// Renamed savedata with index from last established ReturnSavesInfo.
	
	// Internal methods
	void						GetSaveImage(PBYTE *ppbSaveImage, int *puiSaveImageBytes);
	void						GetSaveThumbnail(PBYTE *ppbSaveThumbnail, int *puiSaveThumbnailBytes);
	void						SetSaveUniqueFilename(wchar_t *szFilename);

	///////////////////////////////////////////////////////////////////////////// Profile data ////////////////////////////////////////////////////////////////////////////////////////
	// Initialisation
	void						InitialiseProfileData(unsigned short usProfileVersion, UINT uiProfileValuesC, UINT uiProfileSettingsC, DWORD *pdwProfileSettingsA, int iGameDefinedDataSizeX4, unsigned int *puiGameDefinedDataChangedBitmask);	// General initialisation
	int							SetDefaultOptionsCallback(int( *Func)(LPVOID,PROFILESETTINGS *, const int iPad),LPVOID lpParam);													// Set a callback that can initialise a profile's storage to its default settings
	void						SetOptionsDataCallback(int( *Func)(LPVOID,  int iPad, unsigned short usVersion, C4JStorage::eOptionsCallback),LPVOID lpParam);						// Sets callback that is called when status of any options has changed
	int							SetOldProfileVersionCallback(int( *Func)(LPVOID,unsigned char *, const unsigned short,const int),LPVOID lpParam);

	// Getting and setting of profile data
	PROFILESETTINGS *			GetDashboardProfileSettings(int iPad);																												// Get pointer to the standard (originally xbox dashboard) profile data for one user
	void						*GetGameDefinedProfileData(int iQuadrant);																											// Get pointer to the game-defined profile data for one user

	// Reading and writing profiles
	void						ReadFromProfile(int iQuadrant, int iReadType=PROFILE_READTYPE_ALL);																					// Initiate read profile data for one user - read type is ignored on this platform
	void						WriteToProfile(int iQuadrant, bool bGameDefinedDataChanged=false, bool bOverride5MinuteLimitOnProfileWrites=false);									// Initiate write profile for one user
	void						DeleteOptionsData(int iPad);																														// Delete profile data for one user
	void						ForceQueuedProfileWrites(int iPad=-1);																												// Force any queued profile writes to write now

	// DLC
	C4JStorage::EDLCStatus		GetInstalledDLC(int iPad,int( *Func)(LPVOID, int, int),LPVOID lpParam);
	void						SetLicenseChangeFn(void( *Func)(void));

	XCONTENT_DATA&				GetDLC(DWORD dw);
	DWORD						MountInstalledDLC(int iPad,DWORD dwDLC,int( *Func)(LPVOID, int, DWORD,DWORD),LPVOID lpParam,LPWSTR szMountDrive = nullptr);
	DWORD						UnmountInstalledDLC(LPWSTR szMountDrive = nullptr);
	void						GetMountedDLCFileList(const char* szMountDrive, std::vector<std::wstring>& fileList);
	std::wstring				GetMountedPath(std::wstring szMount);
	XCONTENT_DATA *				GetInstalledDLC(WCHAR *wszProductID);

	C4JStorage::EDLCStatus		GetDLCOffers(int iPad,int( *Func)(LPVOID, int, DWORD, int),LPVOID lpParam, DWORD dwOfferTypesBitmaskT);
	MARKETPLACE_CONTENTOFFER_INFO&		GetOffer(DWORD dw);
	DWORD								InstallOffer(int iOfferIDC,WCHAR *pwchProductId,int( *Func)(LPVOID, int, int),LPVOID lpParam, bool bTrial=false);

	void						UpdateDLCProductIDs(); // once we have the dlc info, we can give local installed DLC their product ids

	//void						SetRetrieveProductIDFn(int( *Func)(XCONTENT_DATA&)); // Retrieve a product id for the dlc from the game

	void						Suspend();
	bool						Suspended();

	///////////////////////////////////////////////////////////////////////////// Unimplemented stubs /////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4100)
	void						SetSaveDeviceSelected(unsigned int uiPad,bool bSelected) {}
	bool						GetSaveDeviceSelected(unsigned int iPad) { return true; }
	void						ClearDLCOffers();
// 	C4JStorage::ETMSStatus		ReadTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,C4JStorage::eTMS_FileType eFileType, WCHAR *pwchFilename,BYTE **ppBuffer,DWORD *pdwBufferSize,int( *Func)(LPVOID, WCHAR *,int, bool, int),LPVOID lpParam, int iAction);// { return C4JStorage::ETMSStatus_Idle; }
// 	bool						WriteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,LPWSTR wszFilename,BYTE *pBuffer,DWORD dwBufferSize);
	bool						DeleteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,LPWSTR wszFilename);
	
	// TMS++
  	C4JStorage::ETMSStatus		TMSPP_GetUserQuotaInfo(C4JStorage::eGlobalStorage eStorageFacility,int iPad);//,TMSCLIENT_CALLBACK Func,LPVOID lpParam, int iUserData=0);
	eTitleStorageState			TMSPP_WriteFile(int iQuadrant,C4JStorage::eGlobalStorage eStorageFacility,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,LPWSTR wszFilename,BYTE *pbBuffer,DWORD dwBufferSize,int( *Func)(LPVOID,int,int)=nullptr,LPVOID lpParam=nullptr, int iUserData=0);
	eTitleStorageState			TMSPP_ReadFile(int iQuadrant,C4JStorage::eGlobalStorage eStorageFacility,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,LPWSTR wszFilename,int( *Func)(LPVOID,int,int,LPVOID, WCHAR *),LPVOID lpParam, int iUserData);
	eTitleStorageState			TMSPP_DeleteFile(int iQuadrant,C4JStorage::eGlobalStorage eStorageFacility,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,LPWSTR wszFilename,int( *Func)(LPVOID,int,int),LPVOID lpParam, int iUserData);
	eTitleStorageState			TMSPP_ReadFileList(int iPad,C4JStorage::eGlobalStorage eStorageFacility,int( *Func)(LPVOID,int,int,LPVOID,WCHAR *)=nullptr,LPVOID lpParam=nullptr, int iUserData=0);
	bool						TMSPP_InFileList(eGlobalStorage eStorageFacility, int iPad,const wstring &Filename);

	eTitleStorageState			TMSPP_GetTitleStorageState(int iPad);
	void						TMSPP_ClearTitleStorageState(int iPad);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// SAVE TRANSFERS
	C4JStorage::eSaveTransferState SaveTransferClearState();
	C4JStorage::eSaveTransferState SaveTransferGetDetails(int iPad, eGlobalStorage source, wchar_t *file, int ( *Func)(LPVOID lpParam,C4JStorage::SAVETRANSFER_FILE_DETAILS *pSaveTransferDetails),LPVOID lpParam);
	C4JStorage::eSaveTransferState SaveTransferGetData(int iPad, eGlobalStorage source, wchar_t *file,int ( *Func)(LPVOID lpParam,C4JStorage::SAVETRANSFER_FILE_DETAILS *pSaveTransferDetails),int ( *ProgressFunc)(LPVOID lpParam,unsigned long),LPVOID lpParam,LPVOID lpProgressParam);
	void CancelSaveTransfer(int ( *CancelCompleteFunc)(LPVOID lpParam),LPVOID lpParam);
};

extern C4JStorage StorageManager;
