#pragma once
#include <vector>
//#include <xtms.h>
class C4JStringTable;

#define MAX_DISPLAYNAME_LENGTH 128 // CELL_SAVEDATA_SYSP_SUBTITLE_SIZE on PS3
#define MAX_DETAILS_LENGTH 128 // CELL_SAVEDATA_SYSP_SUBTITLE_SIZE on PS3
#define MAX_SAVEFILENAME_LENGTH 32 // CELL_SAVEDATA_DIRNAME_SIZE

typedef struct
{
	time_t			modifiedTime;
	unsigned int	dataSize;
	unsigned int	thumbnailSize;
}
CONTAINER_METADATA;

typedef struct
{
	char UTF8SaveFilename[MAX_SAVEFILENAME_LENGTH];
	char UTF8SaveTitle[MAX_DISPLAYNAME_LENGTH];
	CONTAINER_METADATA metaData;
	PBYTE thumbnailData;
}
SAVE_INFO,*PSAVE_INFO;

typedef struct
{
	int iSaveC;
	PSAVE_INFO SaveInfoA;
}
SAVE_DETAILS,*PSAVE_DETAILS;

typedef std::vector <PXMARKETPLACE_CONTENTOFFER_INFO> OfferDataArray;
typedef std::vector <PXCONTENT_DATA> XContentDataArray;
//typedef std::vector <PSAVE_DETAILS> SaveDetailsArray;

// Current version of the dlc data creator
#define CURRENT_DLC_VERSION_NUM 3

class C4JStorage
{

public:
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
 		WCHAR                               wchDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
 		CHAR                                szFileName[XCONTENT_MAX_FILENAME_LENGTH];
		DWORD								dwImageOffset;
		DWORD								dwImageBytes;
	}
	CACHEINFOSTRUCT;

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

	enum ESaveGameControlState
	{
		ESaveGameControl_Idle=0,
		ESaveGameControl_Save,
		ESaveGameControl_InternalRequestingDevice,
		ESaveGameControl_InternalGetSaveName,
		ESaveGameControl_InternalSaving,
		ESaveGameControl_CopySave,
		ESaveGameControl_CopyingSave,
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
		ESaveGame_Rename,
		ESaveGame_Delete,

		ESaveGame_GetSaveThumbnail		// Not used as an actual state in the PS4, but the game expects this to be returned to indicate success when getting a thumbnail

	};
	enum ELoadGameStatus
	{
		ELoadGame_Idle=0,
		ELoadGame_InProgress,
		ELoadGame_NoSaves,
		ELoadGame_ChangedDevice,
		ELoadGame_DeviceRemoved
	};

	enum EDeleteGameStatus
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
		ETMSStatus_Fail_ReadInProgress,
		ETMSStatus_Fail_WriteInProgress,
		ETMSStatus_Pending,
	};

	enum eTMS_FileType
	{
		eTMS_FileType_Normal=0,
		eTMS_FileType_Graphic,
	};

	enum eTMS_FILETYPEVAL
	{
		TMS_FILETYPE_BINARY,
		TMS_FILETYPE_CONFIG,
		TMS_FILETYPE_JSON,
		TMS_FILETYPE_MAX
	};
	enum eTMS_UGCTYPE
	{
		TMS_UGCTYPE_NONE,
		TMS_UGCTYPE_IMAGE,
		TMS_UGCTYPE_MAX
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


	C4JStorage();

	void								Tick(void);

	// Messages
	C4JStorage::EMessageResult			RequestMessageBox(UINT uiTitle, UINT uiText, UINT *uiOptionA,UINT uiOptionC, DWORD dwPad=XUSER_INDEX_ANY,
		int( *Func)(LPVOID,int,const C4JStorage::EMessageResult)=NULL,LPVOID lpParam=NULL, C4JStringTable *pStringTable=NULL, WCHAR *pwchFormatString=NULL,DWORD dwFocusButton=0);


	C4JStorage::EMessageResult			GetMessageBoxResult();

	// save device
	bool								SetSaveDevice(int( *Func)(LPVOID,const bool),LPVOID lpParam, bool bForceResetOfSaveDevice=false);

	// savegame
	void						Init(unsigned int uiSaveVersion,LPCWSTR pwchDefaultSaveName,char *pszSavePackName,int iMinimumSaveSize,int( *Func)(LPVOID, const ESavingMessage, int),LPVOID lpParam,LPCSTR szGroupID);
	void						ResetSaveData(); // Call before a new save to clear out stored save file name
	void						SetDefaultSaveNameForKeyboardDisplay(LPCWSTR pwchDefaultSaveName);
	void						SetSaveTitle(LPCWSTR pwchDefaultSaveName);
	bool						GetSaveUniqueNumber(INT *piVal);
	bool						GetSaveUniqueFilename(char *pszName);
	void						SetSaveUniqueFilename(char *szFilename);
	void						SetState(ESaveGameControlState eControlState,int( *Func)(LPVOID,const bool),LPVOID lpParam);
	void						SetSaveDisabled(bool bDisable);
	bool						GetSaveDisabled(void);
	unsigned int				GetSaveSize();
	void						GetSaveData(void *pvData,unsigned int *puiBytes);
	PVOID						AllocateSaveData(unsigned int uiBytes);
	void						SetSaveImages( PBYTE pbThumbnail,DWORD dwThumbnailBytes,PBYTE pbImage,DWORD dwImageBytes, PBYTE pbTextData ,DWORD dwTextDataBytes);					// Sets the thumbnail & image for the save, optionally setting the metadata in the png
	C4JStorage::ESaveGameState	SaveSaveData(int( *Func)(LPVOID ,const bool),LPVOID lpParam);
	void						CopySaveDataToNewSave(PBYTE pbThumbnail,DWORD cbThumbnail,WCHAR *wchNewName,int ( *Func)(LPVOID lpParam, bool), LPVOID lpParam);
	void						SetSaveDeviceSelected(unsigned int uiPad,bool bSelected);
	bool						GetSaveDeviceSelected(unsigned int iPad);
	C4JStorage::ESaveGameState	DoesSaveExist(bool *pbExists);
	bool						EnoughSpaceForAMinSaveGame();

	void								SetSaveMessageVPosition(float fY); // The 'Saving' message will display at a default position unless changed
	// Get the info for the saves
	C4JStorage::ESaveGameState	GetSavesInfo(int iPad,int ( *Func)(LPVOID lpParam,SAVE_DETAILS *pSaveDetails,const bool),LPVOID lpParam,char *pszSavePackName);
	PSAVE_DETAILS				ReturnSavesInfo();
	void						ClearSavesInfo();	// Clears results
	C4JStorage::ESaveGameState	LoadSaveDataThumbnail(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes), LPVOID lpParam);	// Get the thumbnail for an individual save referenced by pSaveInfo

	void								GetSaveCacheFileInfo(DWORD dwFile,XCONTENT_DATA &xContentData);
	void								GetSaveCacheFileInfo(DWORD dwFile,	PBYTE *ppbImageData, DWORD *pdwImageBytes);

	// Load the save. Need to call GetSaveData once the callback is called
	C4JStorage::ESaveGameState			LoadSaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,const bool, const bool), LPVOID lpParam);
	C4JStorage::ESaveGameState		DeleteSaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,const bool), LPVOID lpParam);

	// DLC
	void								RegisterMarketplaceCountsCallback(int ( *Func)(LPVOID lpParam, C4JStorage::DLC_TMS_DETAILS *, int), LPVOID lpParam );
	void								SetDLCPackageRoot(char *pszDLCRoot);
	C4JStorage::EDLCStatus				GetDLCOffers(int iPad,int( *Func)(LPVOID, int, DWORD, int),LPVOID lpParam, DWORD dwOfferTypesBitmask=XMARKETPLACE_OFFERING_TYPE_CONTENT);
	DWORD								CancelGetDLCOffers();
	void								ClearDLCOffers();
	XMARKETPLACE_CONTENTOFFER_INFO&		GetOffer(DWORD dw);
	int									GetOfferCount();
	DWORD								InstallOffer(int iOfferIDC, uint64_t *ullOfferIDA,int( *Func)(LPVOID, int, int),LPVOID lpParam, bool bTrial=false);
	DWORD								GetAvailableDLCCount( int iPad);

	C4JStorage::EDLCStatus				GetInstalledDLC(int iPad,int( *Func)(LPVOID, int, int),LPVOID lpParam);
	XCONTENT_DATA&						GetDLC(DWORD dw);
	DWORD								MountInstalledDLC(int iPad,DWORD dwDLC,int( *Func)(LPVOID, int, DWORD,DWORD),LPVOID lpParam,LPCSTR szMountDrive=NULL);
	DWORD								UnmountInstalledDLC(LPCSTR szMountDrive = NULL);
	void								GetMountedDLCFileList(const char* szMountDrive, std::vector<std::string>& fileList);
	std::string							GetMountedPath(std::string szMount);

	// Global title storage
	C4JStorage::ETMSStatus				ReadTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,C4JStorage::eTMS_FileType eFileType,
											WCHAR *pwchFilename,BYTE **ppBuffer,DWORD *pdwBufferSize,int( *Func)(LPVOID, WCHAR *,int, bool, int)=NULL,LPVOID lpParam=NULL, int iAction=0);
	bool								WriteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,WCHAR *pwchFilename,BYTE *pBuffer,DWORD dwBufferSize);
	bool								DeleteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,WCHAR *pwchFilename);
	void								StoreTMSPathName(WCHAR *pwchName=NULL);

	// TMS++
#ifdef _XBOX
	C4JStorage::ETMSStatus				WriteTMSFile(int iPad,C4JStorage::eGlobalStorage eStorageFacility,C4JStorage::eTMS_FileType eFileType,CHAR *pchFilePath,CHAR *pchBuffer,DWORD dwBufferSize,TMSCLIENT_CALLBACK Func,LPVOID lpParam);
	HRESULT								GetUserQuotaInfo(int iPad,TMSCLIENT_CALLBACK Func,LPVOID lpParam);
#endif

	// 	C4JStorage::ETMSStatus				TMSPP_WriteFile(int iPad,C4JStorage::eGlobalStorage eStorageFacility,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,C4JStorage::eTMS_UGCTYPE eUGCType,CHAR *pchFilePath,CHAR *pchBuffer,DWORD dwBufferSize,int( *Func)(LPVOID,int,int)=NULL,LPVOID lpParam=NULL, int iUserData=0);
	// 	C4JStorage::ETMSStatus				TMSPP_GetUserQuotaInfo(int iPad,TMSCLIENT_CALLBACK Func,LPVOID lpParam, int iUserData=0);
	C4JStorage::ETMSStatus				TMSPP_ReadFile(int iPad,C4JStorage::eGlobalStorage eStorageFacility,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,LPCSTR szFilename,int( *Func)(LPVOID,int,int,PTMSPP_FILEDATA, LPCSTR)=NULL,LPVOID lpParam=NULL, int iUserData=0);
	// 	C4JStorage::ETMSStatus				TMSPP_ReadFileList(int iPad,C4JStorage::eGlobalStorage eStorageFacility,CHAR *pchFilePath,int( *Func)(LPVOID,int,int,PTMSPP_FILE_LIST)=NULL,LPVOID lpParam=NULL, int iUserData=0);
	// 	C4JStorage::ETMSStatus				TMSPP_DeleteFile(int iPad,LPCSTR szFilePath,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,int( *Func)(LPVOID,int,int),LPVOID lpParam=NULL, int iUserData=0);
	// 	bool								TMSPP_InFileList(eGlobalStorage eStorageFacility, int iPad,const wstring &Filename);
	// 	unsigned int						CRC(unsigned char *buf, int len);

// 	enum eXBLWS
// 	{
// 		eXBLWS_GET,
// 		eXBLWS_POST,
// 		eXBLWS_PUT,
// 		eXBLWS_DELETE,
// 	};
	//bool								XBLWS_Command(eXBLWS eCommand);


	unsigned int						CRC(unsigned char *buf, int len);

// #ifdef _DEBUG
// 	void SetSaveName(int i);
// #endif
	// string table for all the Storage problems. Loaded by the application
	C4JStringTable				*m_pStringTable;
};

extern C4JStorage StorageManager;
