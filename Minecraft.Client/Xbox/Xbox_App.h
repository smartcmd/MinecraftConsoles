#pragma once

#define GROUP_ID "A9C80F8E-5EAE-4883-89E6-0C456CADE89B" 
#define SAVETRANSFER_GROUP_ID "43FD7A62-2747-4489-8E71-F937163DC3C5" 

class XUI_FontRenderer;

class CConsoleMinecraftApp : public CMinecraftApp 
{
private:
	bool m_bMenuDisplayed[XUSER_MAX_COUNT]; // track each players menu displayed
	bool m_bMenuToBeClosed[XUSER_MAX_COUNT]; // actioned at the end of the game loop
	bool m_bPauseMenuDisplayed[XUSER_MAX_COUNT];
	bool m_bContainerMenuDisplayed[XUSER_MAX_COUNT];
	bool m_bIgnoreAutosaveMenuDisplayed[XUSER_MAX_COUNT];
	bool m_bIgnorePlayerJoinMenuDisplayed[XUSER_MAX_COUNT];
	int m_iCountDown[XUSER_MAX_COUNT]; // ticks to block input
	
	HXUIOBJ m_PlayerBaseScene[XUSER_MAX_COUNT];
	HXUIOBJ m_hFirstScene[XUSER_MAX_COUNT];
	HXUIOBJ m_hCurrentScene[XUSER_MAX_COUNT];
	HXUIOBJ m_hFirstTutorialScene[XUSER_MAX_COUNT];
	HXUIOBJ m_hCurrentTutorialScene[XUSER_MAX_COUNT];
	HXUIOBJ m_hFirstChatScene[XUSER_MAX_COUNT];
	HXUIOBJ m_hCurrentChatScene[XUSER_MAX_COUNT];
	HXUIOBJ m_hFirstHudScene[XUSER_MAX_COUNT];
	HXUIOBJ m_hCurrentHudScene[XUSER_MAX_COUNT];

	std::list< SceneStackPair > m_sceneStack[XUSER_MAX_COUNT];	

	// XUI scene names
	static WCHAR *wchSceneA[eUIScene_COUNT];
	static WCHAR *wchTypefaceA[4];
	static WCHAR *wchTypefaceLocatorA[4];
	
	WCHAR m_SceneName[50];
public:
	CConsoleMinecraftApp();	

	virtual void SetRichPresenceContext(int iPad, int contextId);

	virtual HRESULT RegisterXuiClasses();
	virtual HRESULT UnregisterXuiClasses();

	XTITLE_DEPLOYMENT_TYPE getDeploymentType() { return m_titleDeploymentType; }

	void GetPreviewImage(int iPad,XSOCIAL_PREVIEWIMAGE *preview);

	virtual HRESULT LoadXuiResources();

	virtual void CaptureScreenshot(int iPad);
	virtual void CaptureSaveThumbnail();
	virtual void GetSaveThumbnail(PBYTE*,DWORD*);
	virtual void ReleaseSaveThumbnail();
	virtual void GetScreenshot(int iPad,PBYTE *pbData,DWORD *pdwSize);

	virtual void RunFrame();

	// TMS++
	void TMSPP_SetTitleGroupID(LPCSTR szTitleGroupID);
	void TMSPP_RetrieveFileList(int iPad,C4JStorage::eGlobalStorage eStorageFacility,CHAR *szPath,eTMSAction NextAction);
	void TMSPP_ReadXuidsFile(int iPad,eTMSAction NextAction);
	void TMSPP_ReadConfigFile(int iPad,eTMSAction NextAction);
	void TMSPP_ReadDLCFile(int iPad,eTMSAction NextAction);
	bool TMSPP_ReadBannedList(int iPad,eTMSAction NextAction);

	static int Callback_TMSPPReadFileList(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILE_LIST pTmsFileList);
	static int Callback_TMSPPReadXuidsFile(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILEDATA pFileData,LPCSTR szFilename);
	static int Callback_TMSPPReadConfigFile(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILEDATA pFileData, LPCSTR szFilename);
	static int Callback_TMSPPReadDLCFile(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILEDATA pFileData,LPCSTR szFilename);
	static int Callback_TMSPPReadBannedList(void *pParam,int iPad, int iUserData, C4JStorage::PTMSPP_FILEDATA pFileData,LPCSTR szFilename);

	virtual bool GetTMSGlobalFileListRead() { return m_bTMSPP_GlobalFileListRead;}
	virtual bool GetTMSDLCInfoRead() { return m_bRead_TMS_DLCINFO_XML;}
	virtual bool GetTMSXUIDsFileRead() { return m_bRead_TMS_XUIDS_XML;}

	virtual int LoadLocalTMSFile(WCHAR *wchTMSFile);
	virtual int LoadLocalTMSFile(WCHAR *wchTMSFile, eFileExtensionType eExt);
	virtual void FreeLocalTMSFiles(eTMSFileType eType);
	virtual int GetLocalTMSFileIndex(WCHAR *wchTMSFile,bool bFilenameIncludesExtension,eFileExtensionType eEXT=eFileExtensionType_PNG);
	virtual int RetrieveTMSFileListIndex(WCHAR *wchTMSFile);

	virtual void OverrideFontRenderer(bool set, bool immediate=true);
	virtual void ToggleFontRenderer() { OverrideFontRenderer(!m_bFontRendererOverridden,false); }

	virtual LPCWSTR GetString(int iID);
	CXuiStringTable *GetStringTable();

#ifdef _DEBUG_MENUS_ENABLED	
	void EnableDebugOverlay(bool enable, int iPad);
#endif

private:
	XUI_FontRenderer *m_fontRenderer;
	bool m_bFontRendererOverridden;
	bool m_bOverrideFontRenderer;
	// Global string table for this application.
	CXuiStringTable StringTable;
	CXuiScene debugContainerScene;


	// screenshot for social post, and thumbnail for save
	LPD3DXBUFFER m_ThumbnailBuffer;
	LPD3DXBUFFER m_ScreenshotBuffer[XUSER_MAX_COUNT];

private:
	XTITLE_DEPLOYMENT_TYPE m_titleDeploymentType;
	XSOCIAL_PREVIEWIMAGE m_PreviewBuffer[XUSER_MAX_COUNT];

	bool m_bTMSPP_GlobalFileListRead; // track whether we have already read the file list from TMSPP
	bool m_bRead_TMS_XUIDS_XML; // track whether we have already read the TMS xuids.xml file
	bool m_bRead_TMS_Config_XML; // track whether we have already read the config file
	bool m_bRead_TMS_DLCINFO_XML; // track whether we have already read the TMS DLC.xml file
	BYTE *m_pXuidsFileBuffer;
	DWORD m_dwXuidsFileSize;

#ifdef _DEBUG_MENUS_ENABLED	
	bool debugOverlayCreated;
	HXUIOBJ m_hDebugOverlay;
#endif

public:

	void ReadBannedList(int iPad, eTMSAction action=static_cast<eTMSAction>(0), bool bCallback=false);
// 	void ReadXuidsFileFromTMS(int iPad,eTMSAction NextAction,bool bCallback);
// 	void ReadDLCFileFromTMS(int iPad,eTMSAction NextAction, bool bCallback);

// 	static int CallbackReadXuidsFileFromTMS(LPVOID lpParam, WCHAR *wchFilename, int iPad, bool bResult, int iAction);
// 	static int CallbackDLCFileFromTMS(LPVOID lpParam, WCHAR *wchFilename, int iPad, bool bResult, int iAction);
	static int CallbackBannedListFileFromTMS(LPVOID lpParam, WCHAR *wchFilename, int iPad, bool bResult, int iAction);

	HRESULT RegisterFont(eFont eFontLanguage,eFont eFontFallback, bool bSetAsDefault=false);
	
	WCHAR *GetSceneName(EUIScene eScene, bool bAppendToName,bool bSplitscreenScene);

	virtual HRESULT			NavigateToScene(int iPad,EUIScene eScene, void *initData = nullptr, bool forceUsePad = false, BOOL bStayVisible=FALSE, HXUIOBJ *phResultingScene=nullptr);
	virtual HRESULT			NavigateBack(int iPad, bool forceUsePad = false,EUIScene eScene = eUIScene_COUNT);
	virtual HRESULT			TutorialSceneNavigateBack(int iPad, bool forceUsePad = false);
	virtual HRESULT			CloseXuiScenes(int iPad, bool forceUsePad = false);
	virtual HRESULT			CloseAllPlayersXuiScenes();
	virtual HRESULT			CloseXuiScenesAndNavigateToScene(int iPad,EUIScene eScene, void *initData=nullptr, bool forceUsePad = false);
	virtual HRESULT			RemoveBackScene(int iPad);
	virtual HRESULT			NavigateToHomeMenu();
	D3DXVECTOR3				GetElementScreenPosition(HXUIOBJ hObj);
	virtual void			SetChatTextDisplayed(int iPad, bool bVal);
	virtual void			ReloadChatScene(int iPad, bool bJoining = false, bool bForce = false);
	virtual void			ReloadHudScene(int iPad, bool bJoining = false, bool bForce = false);

	bool					GetMenuDisplayed(int iPad);
	void					SetMenuDisplayed(int iPad,bool bVal);
	void					CheckMenuDisplayed();
	bool					IsSceneInStack(int iPad, EUIScene eScene);

	bool					IsPauseMenuDisplayed(int iPad)									{ return m_bPauseMenuDisplayed[iPad]; }
	bool					IsContainerMenuDisplayed(int iPad)								{ return m_bContainerMenuDisplayed[iPad]; }
	bool					IsIgnoreAutosaveMenuDisplayed(int iPad)							{ return m_bIgnoreAutosaveMenuDisplayed[iPad]; }
	void					SetIgnoreAutosaveMenuDisplayed(int iPad, bool displayed)		{ m_bIgnoreAutosaveMenuDisplayed[iPad] = displayed; }
	bool					IsIgnorePlayerJoinMenuDisplayed(int iPad)						{ return m_bIgnorePlayerJoinMenuDisplayed[iPad]; }
	void					SetIgnorePlayerJoinMenuDisplayed(int iPad, bool displayed)		{ m_bIgnorePlayerJoinMenuDisplayed[iPad] = displayed; }

	HXUIOBJ			GetFirstScene(int iPad)		
	{
		return m_hFirstScene[iPad];
	}
	HXUIOBJ			GetCurrentTutorialScene(int iPad)
	{
		return m_hCurrentTutorialScene[iPad];
	}
	HXUIOBJ			GetCurrentHUDScene(int iPad)
	{
		return m_hCurrentHudScene[iPad];
	}
	HXUIOBJ GetCurrentScene(int iPad) { return m_hCurrentScene[iPad]; }

	// Move splitscreen scenes based on which screen segment they are in
	void AdjustSplitscreenScene(HXUIOBJ hScene,D3DXVECTOR3 *pvOriginalPosition, int iPad, bool bAdjustXForSafeArea=true);
	void AdjustSplitscreenScene(HXUIOBJ hScene,D3DXVECTOR3 *pvOriginalPosition, int iPad, float fXAdjust);
	HRESULT AdjustSplitscreenScene_PlayerChanged(HXUIOBJ hScene,D3DXVECTOR3 *pvOriginalPosition, int iPad, bool bJoining, bool bAdjustXForSafeArea=true);
	HRESULT AdjustSplitscreenScene_PlayerChanged(HXUIOBJ hScene,D3DXVECTOR3 *pvOriginalPosition, int iPad, bool bJoining, float fXAdjust);

	// functions to store launch data, and to exit the game - required due to possibly being on a demo disc
	virtual void StoreLaunchData();
	virtual void ExitGame();
	virtual void FatalLoadError();
	static TMS_FILE TMSFileA[TMS_COUNT];


	virtual void GetFileFromTPD(eTPDFileType eType,PBYTE pbData,DWORD dwBytes,PBYTE *ppbData,DWORD *pdwBytes );

private:
	static WCHAR m_wchTMSXZP[];
	static WCHAR *CConsoleMinecraftApp::wchExt[MAX_EXTENSION_TYPES];


};

extern CConsoleMinecraftApp app;