#pragma once
using namespace std;
#include <vector>
#include <sysutil/sysutil_savedata.h>
#include <cell/cell_fs.h>


#define MAX_DISPLAYNAME_LENGTH 128 // CELL_SAVEDATA_SYSP_SUBTITLE_SIZE on PS3
#define MAX_DETAILS_LENGTH 128 // CELL_SAVEDATA_SYSP_SUBTITLE_SIZE on PS3
#define MAX_SAVEFILENAME_LENGTH 32 // CELL_SAVEDATA_DIRNAME_SIZE
#define USER_INDEX_ANY 0x000000FF
#define RESULT	LONG

class StringTable;

typedef struct  
{
	char UTF8SaveFilename[MAX_SAVEFILENAME_LENGTH];
	char UTF8SaveTitle[MAX_DISPLAYNAME_LENGTH];
	time_t modifiedTime;
	//int sizeKB;
}
SAVE_INFO,*PSAVE_INFO;

typedef struct  
{
	int iSaveC;
	PSAVE_INFO SaveInfoA;
}
SAVE_DETAILS,*PSAVE_DETAILS;

class CONTENT_DATA
{
public:
	int DeviceID;
	DWORD dwContentType;
	WCHAR szDisplayName[256];
	char d_name[CELL_FS_MAX_FS_FILE_NAME_LENGTH + 1];
	CHAR szFileName[CELL_FS_MAX_FS_PATH_LENGTH];
	//CHAR szHDDFileName[CELL_FS_MAX_FS_PATH_LENGTH]; // if we are running from bluray, the keys for DLC will be in the HDD path, but the DLC will be on the bluray
};

typedef CONTENT_DATA XCONTENT_DATA, *PXCONTENT_DATA;

#define MARKETPLACE_CONTENTOFFER_INFO int

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



	// structure to hold DLC info in TMS

typedef struct  
	{
		DWORD dwVersion;
		DWORD dwNewOffers;
		DWORD dwTotalOffers;
		DWORD dwInstalledTotalOffers;
		BYTE bPadding[1024-sizeof(DWORD)*4]; // future expansion
	}
	DLC_TMS_DETAILS;

	enum eGTS_FileTypes
	{
		eGTS_Type_Skin=0,
		eGTS_Type_Cape,
		eGTS_Type_MAX
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
		ESaveGame_InternalRequestingDevice,
		ESaveGame_InternalGetSaveName,
		ESaveGame_InternalSaving,
		ESaveGame_CopySave,
		ESaveGame_CopyingSave,
		ESaveGame_Load,	
		ESaveGame_GetSavesInfo,	
		ESaveGame_GetSaveThumbnail,	
		ESaveGame_Rename,	
		ESaveGame_RenameComplete,	
		ESaveGame_Delete,	
		ESaveGame_DeleteComplete,		
		// Cache file
		ESaveGame_ReadCacheFile,
		ESaveGame_WriteCacheFile,
		ESaveGame_NoSpace,
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

/*	enum ELoadGameStatus
	{
		ELoadGame_Idle=0,
		ELoadGame_Loading,
		ELoadGame_NoSaves,
		ELoadGame_ChangedDevice,
		ELoadGame_DeviceRemoved
	};*/

	enum ESaveGameStatus
	{
		EDeleteGame_Idle=0,
		EDeleteGame_InProgress,
	};


	enum ESGIStatus
	{
		ESGIStatus_Error=0,
		ESGIStatus_Idle,
		ESGIStatus_ReadInProgress,
		ESGIStatus_NoSaves,
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

	enum eTMS_FILETYPEVAL
	{
		TMS_FILETYPE_BINARY=0,
		TMS_FILETYPE_CONFIG=1,
		TMS_FILETYPE_JSON=2,
		TMS_FILETYPE_MAX,
	};
	enum eTMS_UGCTYPE
	{
		TMS_UGCTYPE_NONE,
		TMS_UGCTYPE_IMAGE,
		TMS_UGCTYPE_MAX
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

	typedef struct  
	{
		CHAR			szFilename[256];
		int				iFileSize;
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
		DWORD dwSize;
		PBYTE pbData;
	}
	TMSPP_FILEDATA, *PTMSPP_FILEDATA;

	typedef struct  
	{
		WCHAR                               wchDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
		CHAR                                szFileName[XCONTENT_MAX_FILENAME_LENGTH];
		DWORD								dwImageOffset;
		DWORD								dwImageBytes;
	}
	CACHEINFOSTRUCT;


	// On PS3 the user profile data is stored in a file, so moving this to the storage library rather than the profile library
	C4JStorage(	);

	void								ExitRequest(void (*ExitCompleteFn)() );

	void								SetSecureID(char *pchSecureID);
	void								Tick(void);

	// Messages
	C4JStorage::EMessageResult			RequestMessageBox(UINT uiTitle, UINT uiText, UINT *uiOptionA,UINT uiOptionC, DWORD dwPad=USER_INDEX_ANY,
		int( *Func)(LPVOID,int,const C4JStorage::EMessageResult)=NULL,LPVOID lpParam=NULL, StringTable *pStringTable=NULL, WCHAR *pwchFormatString=NULL,DWORD dwFocusButton=0);


	C4JStorage::EMessageResult			GetMessageBoxResult();

	// save device
	bool								SetSaveDevice(int( *Func)(LPVOID,const bool),LPVOID lpParam, bool bForceResetOfSaveDevice=false);

	// savegame
	void						Init(unsigned int uiSaveVersion,LPCWSTR pwchDefaultSaveName,char *pszSavePackName,int iMinimumSaveSize,int( *Func)(LPVOID, const ESavingMessage, int),LPVOID lpParam,LPCSTR szGroupID);
	void						InitialiseProfileData(unsigned short usProfileVersion,
													UINT uiProfileValuesC,
													UINT uiProfileSettingsC,
													DWORD *pdwProfileSettingsA, 
													int iGameDefinedDataSizeX4,
													unsigned int *puiGameDefinedDataChangedBitmask);

	void						ResetSaveData(); // Call before a new save to clear out stored save file name
	void						SetDefaultSaveNameForKeyboardDisplay(LPCWSTR pwchDefaultSaveName);
	void						SetGameSaveFolderTitle(WCHAR *wszGameSaveFolderTitle);
	void						SetSaveCacheFolderTitle(WCHAR *wszSaveCacheFolderTitle);
	void						SetOptionsFolderTitle(WCHAR *wszOptionsFolderTitle);
	void						SetGameSaveFolderPrefix(char *szGameSaveFolderPrefix);
	void						SetMaxSaves(int iMaxC);
	void						SetSaveTitle(const wchar_t *UTF16String);
	uint16_t *					GetSaveTitle();
	bool						GetSaveUniqueNumber(INT *piVal);
	bool						GetSaveUniqueFilename(char *pszName);
	bool						GetSaveUniqueFileDir(char *pszName);
	void						SetSaveUniqueFilename(char *szFilename);
	void						SetState(ESaveGameState eState,int( *Func)(LPVOID,const bool),LPVOID lpParam);
	void						SetSaveDisabled(bool bDisable);
	bool						GetSaveDisabled(void);
	unsigned int				GetSaveSize();
	void						SetSaveImages( PBYTE pbThumbnail,DWORD dwThumbnailBytes,PBYTE pbImage,DWORD dwImageBytes, PBYTE pbTextData ,DWORD dwTextDataBytes);
	void						SetDefaultSaveImage(); // MGH - added for remote storage compress, we can't get the save image back to overwrite, so set it to the default

	void						DeleteOptionsData(int iPad);

	C4JStorage::ESaveGameState	GetSaveState();

	// Get the info for the saves
	C4JStorage::ESaveGameState	GetSavesInfo(int iPad,int ( *Func)(LPVOID lpParam,SAVE_DETAILS *pSaveDetails,const bool),LPVOID lpParam,char *pszSavePackName);
	PSAVE_DETAILS				ReturnSavesInfo();
	void						ClearSavesInfo();
	// Load the save. Need to call GetSaveData once the callback is called
	C4JStorage::ESaveGameState		LoadSaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,const bool, const bool), LPVOID lpParam, bool bIgnoreCRC = false); // MGH - added bIgnoreCRC for remote save stuff
	C4JStorage::ESaveGameState		DeleteSaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,const bool), LPVOID lpParam);
	C4JStorage::ESaveGameState		RenameSaveData(int iRenameIndex,uint16_t*pui16NewName,int( *Func)(LPVOID lpParam,const bool), LPVOID lpParam);
	C4JStorage::ESaveGameState		LoadSaveDataThumbnail(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes), LPVOID lpParam);

	void						GetSaveData(void *pvData,unsigned int *puiBytes);
	PVOID						AllocateSaveData(unsigned int uiBytes);
	void						SetSaveData(void *data, unsigned int uiBytes);
	void						FreeSaveData();
	void						SetSaveDataSize(unsigned int uiBytes); // after a successful compression, update the size of the gamedata

	//void						SaveSaveData(unsigned int uiBytes,PBYTE pbThumbnail=NULL,DWORD cbThumbnail=0,PBYTE pbTextData=NULL, DWORD dwTextLen=0);
	C4JStorage::ESaveGameState		SaveSaveData(int( *Func)(LPVOID ,const bool),LPVOID lpParam, bool bDataFileOnly = false);
	void						CopySaveDataToNewSave(PBYTE pbThumbnail,DWORD cbThumbnail,WCHAR *wchNewName,int ( *Func)(LPVOID lpParam, bool), LPVOID lpParam);
	void						SetSaveDeviceSelected(unsigned int uiPad,bool bSelected);	
	bool						GetSaveDeviceSelected(unsigned int iPad);
	C4JStorage::ESaveGameState	DoesSaveExist(bool *pbExists);
	bool						EnoughSpaceForAMinSaveGame();

	void						GetSaveImage(PBYTE *ppbSaveImage, int *puiSaveImageBytes);
	void						GetSaveThumbnail(PBYTE *ppbSaveThumbnail, int *puiSaveThumbnailBytes);



	void						SetSaveMessageVPosition(float fY); // The 'Saving' message will display at a default position unless changed

	void						SetMessageBoxCallback( int (*Func)(UINT uiTitle, UINT uiText, UINT *uiOptionA, UINT uiOptionC, DWORD dwPad, int(*Func)(LPVOID,int,const C4JStorage::EMessageResult), LPVOID lpParam) );
	
	// string table for all the Storage problems. Loaded by the application
	StringTable				*m_pStringTable;

	// TODO

	void								RegisterMarketplaceCountsCallback(int ( *Func)(LPVOID lpParam, C4JStorage::DLC_TMS_DETAILS *, int), LPVOID lpParam ) {}
	void								SetDLCPackageRoot(char *pszDLCRoot);
	C4JStorage::EDLCStatus				GetDLCOffers(int iPad,int( *Func)(LPVOID, int, DWORD, int),LPVOID lpParam, DWORD dwOfferTypesBitmaskT) { return C4JStorage::EDLC_Idle; }
	DWORD								CancelGetDLCOffers() { return 0; }
	void								ClearDLCOffers() {}
	MARKETPLACE_CONTENTOFFER_INFO&		GetOffer(DWORD dw) { static MARKETPLACE_CONTENTOFFER_INFO retval = {0}; return retval; }
	int									GetOfferCount() { return 0; }
	DWORD								InstallOffer(int iOfferIDC,ULONGLONG *ullOfferIDA,int( *Func)(LPVOID, int, int),LPVOID lpParam, bool bTrial) { return 0; }
	DWORD								GetAvailableDLCCount( int iPad);
	CONTENT_DATA&						GetDLC(DWORD dw);
	C4JStorage::EDLCStatus				GetInstalledDLC(int iPad,int( *Func)(LPVOID, int, int),LPVOID lpParam);
	DWORD								MountInstalledDLC(int iPad,DWORD dwDLC,int( *Func)(LPVOID, int, DWORD,DWORD),LPVOID lpParam,LPCSTR szMountDrive = NULL);
	DWORD								UnmountInstalledDLC(LPCSTR szMountDrive = NULL);
	void								GetMountedDLCFileList(const char* szMountDrive, std::vector<std::string>& fileList);
	std::string							GetMountedPath(std::string szMount);
	C4JStorage::ETMSStatus				ReadTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,C4JStorage::eTMS_FileType eFileType, WCHAR *pwchFilename,BYTE **ppBuffer,DWORD *pdwBufferSize,int( *Func)(LPVOID, WCHAR *,int, bool, int),LPVOID lpParam, int iAction) { return C4JStorage::ETMSStatus_Idle; }
	bool								WriteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,WCHAR *pwchFilename,BYTE *pBuffer,DWORD dwBufferSize) { return true; }
	bool								DeleteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,WCHAR *pwchFilename) { return true; }
	void								StoreTMSPathName(WCHAR *pwchName) {}
	unsigned int						CRC(unsigned char *buf, int len) { return 0; }
	void								SetDLCProductCode(const char* szProductCode,const char* szDiscPatchProductCode);
	void								SetProductUpgradeKey(const char* szKey);
	void								SetBootTypeDisc(bool bDisc); // true if booting from disc, false if booting from HDD 
	bool								GetBootTypeDisc();
	void								SetBDPatchUsrDir(char *path);
	
	bool								CheckForTrialUpgradeKey(void( *Func)(LPVOID, bool),LPVOID lpParam);

	C4JStorage::ETMSStatus				TMSPP_ReadFile(int iPad,C4JStorage::eGlobalStorage eStorageFacility,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,LPCSTR szFilename,int( *Func)(LPVOID,int,int,PTMSPP_FILEDATA, LPCSTR)/*=NULL*/,LPVOID lpParam/*=NULL*/, int iUserData/*=0*/) {return C4JStorage::ETMSStatus_Idle;}

	// PROFILE DATA
	int					SetDefaultOptionsCallback(int( *Func)(LPVOID,PROFILESETTINGS *, const int iPad),LPVOID lpParam);
	int					SetOldProfileVersionCallback(int( *Func)(LPVOID,unsigned char *, const unsigned short,const int),LPVOID lpParam);
	PROFILESETTINGS *	GetDashboardProfileSettings(int iPad);
	void				WriteToProfile(int iQuadrant, bool bGameDefinedDataChanged=false, bool bOverride5MinuteLimitOnProfileWrites=false);
	void				ReadFromProfile(int iQuadrant, int iReadType=PROFILE_READTYPE_ALL);
	void				ForceQueuedProfileWrites(int iPad=XUSER_INDEX_ANY);
	void				*GetGameDefinedProfileData(int iQuadrant);
	void				SetContinueWithoutSavingMessage(char *szMessage);
	void				SetDefaultImages(PBYTE pbOptionsImage,DWORD dwOptionsImageBytes,PBYTE pbSaveImage,DWORD dwSaveImageBytes,PBYTE pbSaveThumbnail,DWORD dwSaveThumbnailBytes);
	void				GetDefaultSaveImage(PBYTE *ppbSaveImage,DWORD *pdwSaveImageBytes);
	void				GetDefaultSaveThumbnail(PBYTE *ppbSaveThumbnail,DWORD *pdwSaveThumbnailBytes);
	void				SetOptionsDataCallback(int( *Func)(LPVOID,  int iPad, unsigned short usVersion, C4JStorage::eOptionsCallback),LPVOID lpParam);

	void				SetTrialAwardsFlag(int iQuadrant,int iAward);
	void				ClearTrialAwardsFlag(int iQuadrant,int iAward);
	bool				IsTrialAwardsFlagSet(int iQuadrant, int iAward);


	// Save icon
	void				SetSaveLoadIcon(PBYTE pbIcon,DWORD dwIconBytes);

	// system ui is up? - need to avoid displaying errors needing user input
	void				SetSystemUIDisplaying(bool bVal);
	bool				GetSystemUIDisplaying(void);


};

extern C4JStorage StorageManager;
