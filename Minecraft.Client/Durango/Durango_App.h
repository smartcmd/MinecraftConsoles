#pragma once

#define SERVICE_CONFIG_ID			L"05c20100-6e60-45d5-878a-4903149e11ae"
#define TITLE_PRODUCT_ID			L"582e7bcc-11bc-4702-ab1b-b31566f8e327" // Parent Title's ProductID


class CConsoleMinecraftApp : public CMinecraftApp 
{

public:
	ImageFileBuffer m_ThumbnailBuffer;

	CConsoleMinecraftApp();	

private:
	int			m_iLastPresenceContext[MAX_LOCAL_PLAYERS];
	PlayerUID	m_xuidLastPresencePlayer[MAX_LOCAL_PLAYERS];
public:
	virtual void SetRichPresenceContext(int iPad, int contextId);

	virtual void StoreLaunchData();
	virtual void ExitGame();
	virtual void FatalLoadError();

	virtual void CaptureSaveThumbnail();
	virtual void GetSaveThumbnail(PBYTE*,DWORD*);
	virtual void ReleaseSaveThumbnail();
	virtual void GetScreenshot(int iPad,PBYTE *pbData,DWORD *pdwSize);

	virtual int LoadLocalTMSFile(WCHAR *wchTMSFile);
	virtual int LoadLocalTMSFile(WCHAR *wchTMSFile, eFileExtensionType eExt);
	int LoadLocalDLCImage(WCHAR *wchName,PBYTE *ppbImageData,DWORD *pdwBytes);
	int LoadLocalDLCImages();
	void FreeLocalDLCImages();

	virtual void FreeLocalTMSFiles(eTMSFileType eType);
	virtual int GetLocalTMSFileIndex(WCHAR *wchTMSFile,bool bFilenameIncludesExtension,eFileExtensionType eEXT=eFileExtensionType_PNG);

	// BANNED LEVEL LIST
	virtual void ReadBannedList(int iPad, eTMSAction action=static_cast<eTMSAction>(0), bool bCallback=false) {}

	// TMS++
 	void TMSPP_RetrieveFileList(int iPad,C4JStorage::eGlobalStorage eStorageFacility,eTMSAction NextAction);
// 	void TMSPP_ReadXuidsFile(int iPad,eTMSAction NextAction);
// 	void TMSPP_ReadConfigFile(int iPad,eTMSAction NextAction);
 	void TMSPP_ReadDLCFile(int iPad,eTMSAction NextAction);
	bool TMSPP_ReadBannedList(int iPad,eTMSAction NextAction);

 	static int Callback_TMSPPRetrieveFileList(void *pParam,int iPad, int iUserData, LPVOID lpvData,WCHAR *wchFilename);
// 	static int Callback_TMSPPReadXuidsFile(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILEDATA pFileData,LPCSTR szFilename);
// 	static int Callback_TMSPPReadConfigFile(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILEDATA pFileData, LPCSTR szFilename);
 	static int Callback_TMSPPReadDLCFile(void *pParam,int iPad, int iUserData, LPVOID lpvData,WCHAR *wchFilename);
	static int Callback_TMSPPReadBannedList(void *pParam,int iPad, int iUserData, LPVOID lpvData,WCHAR *wchFilename);
	virtual bool GetTMSDLCInfoRead()			{ return m_bRead_TMS_DLCINFO_XML;}
	virtual bool GetTMSGlobalFileListRead()		{ return m_bTMSPP_GlobalFileListRead;}
	virtual bool GetTMSUserFileListRead()		{ return m_bTMSPP_UserFileListRead;}

	static void Callback_SaveGameIncomplete(void *pParam, C4JStorage::ESaveIncompleteType saveIncompleteType);
	static int Callback_SaveGameIncompleteMessageBoxReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

	C4JStringTable *GetStringTable()																									{ return nullptr;}

	// original code
	virtual void TemporaryCreateGameStart();

	void InitialiseDLCDetails();
	static bool UpdateProductId(XCONTENT_DATA &Data);

	void Shutdown();	
	bool getShutdownFlag();

	void ReadLocalDLCList(void);
	static void HandleDLCLicenseChange();

private:
	DLC_INFO *m_DLCDetailsA;
	bool m_bShutdown;

	bool m_bRead_TMS_DLCINFO_XML; // track whether we have already read the TMS DLC.xml file
	bool m_bTMSPP_GlobalFileListRead; // track whether we have already read the file list from TMSPP
	bool m_bTMSPP_UserFileListRead; // track whether we have already read the file list from TMSPP

};

extern CConsoleMinecraftApp app;


