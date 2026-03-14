#pragma once
using namespace std;
#include <vector>
#include "../Media/xuiscene_multi_joinload.h"
#include "XUI_CustomMessages.h"


#define JOIN_LOAD_CREATE_BUTTON_INDEX 0

#define JOIN_LOAD_ONLINE_TIMER_ID 0
#define JOIN_LOAD_ONLINE_TIMER_TIME 100
#define JOIN_LOAD_SEARCH_MINIMUM_TIMER_ID 1
#define JOIN_LOAD_SEARCH_MINIMUM_TIMER_TIME 2000
#define JOIN_LOAD_SCROLL_GAME_NAMES_TIMER_ID 2
#define JOIN_LOAD_SCROLL_GAME_NAMES_TIMER_TIME 1000

class CXuiCtrl4JList;
class LevelGenerationOptions;
class CScene_MultiGameInfo;

class CScene_MultiGameJoinLoad : public CXuiSceneImpl
{
protected:
	CXuiCtrl4JList *m_pSavesList;
	CXuiCtrl4JList *m_pGamesList;
	CXuiList m_SavesList;
	CXuiList m_GamesList;
	CXuiControl m_SavesListTimer;
	CXuiControl m_NetGamesListTimer;
	CXuiControl m_LabelNoGames;
	int m_iPad;

	bool m_bShowingPartyGamesOnly;
	bool m_bInParty;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_DESTROY(OnDestroy)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_NOTIFY_SELCHANGED(OnNotifySelChanged)
		XUI_ON_XM_NOTIFY_SET_FOCUS(OnNotifySetFocus)
		XUI_ON_XM_NOTIFY_KILL_FOCUS(OnNotifyKillFocus)
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_FONTRENDERERCHANGE_MESSAGE(OnFontRendererChange)
		XUI_ON_XM_DLCINSTALLED_MESSAGE(OnCustomMessage_DLCInstalled)
		XUI_ON_XM_DLCLOADED_MESSAGE(OnCustomMessage_DLCMountingComplete)

	XUI_END_MSG_MAP()

		BEGIN_CONTROL_MAP()
			MAP_CONTROL(IDC_GamesList, m_GamesList)
			MAP_CONTROL(IDC_SavesTimer, m_SavesListTimer)
			MAP_CONTROL(IDC_Timer, m_NetGamesListTimer)
			MAP_CONTROL(IDC_LabelNoGames, m_LabelNoGames)
			MAP_CONTROL(IDC_SavesList, m_SavesList)
		END_CONTROL_MAP()


		HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
		HRESULT OnDestroy();
		HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
		HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
		HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
		HRESULT OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled);
		HRESULT OnNotifySetFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled);
		HRESULT OnNotifyKillFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled);
		HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
		HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
		HRESULT OnFontRendererChange();
		HRESULT OnCustomMessage_DLCInstalled();
		HRESULT OnCustomMessage_DLCMountingComplete();


public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_MultiGameJoinLoad, L"CScene_MultiGameJoinLoad", XUI_CLASS_SCENE )

private:	
	bool DoesSavesListHaveFocus();
	bool DoesGamesListHaveFocus();
	bool DoesMashUpWorldHaveFocus();

public:
	static void UpdateGamesListCallback(LPVOID pParam);

private:
	void AddDefaultButtons();
	void UpdateGamesList();
	void UpdateGamesList(DWORD dwNumResults, IQNetGameSearch *pGameSearch);
	//void UpdateGamesListLabels();
	static void SearchForGameCallback(void *hObj, DWORD dwNumResults, IQNetGameSearch *pGameSearch);	
	static int DeviceSelectReturned(void *pParam,bool bContinue);

	unsigned char m_localPlayers;

	HRESULT GetSaveInfo(  );
	static int LoadSaveDataReturned(void *pParam,bool bContinue);
	static int DeleteSaveDataReturned(void *pParam,bool bSuccess);

	unsigned int m_uiSaveC;
	void LoadLevelGen(LevelGenerationOptions *levelGen);
	void LoadSaveFromDisk(File *saveFile);

	// callback
#ifdef _XBOX
	static bool GetSavesInfoCallback(LPVOID pParam,int iInstalledC, C4JStorage::CACHEINFOSTRUCT *InfoA, int iPad, HRESULT hRes);
#else
	static int GetSavesInfoCallback(LPVOID lpParam,const bool);
#endif
	static int DeleteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int SaveOptionsDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int DeviceRemovedDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int StartGame_SignInReturned(void *pParam,bool bContinue, int iPad);
	static int CopySaveReturned(void *pParam,bool bResult);
	static int LoadSaveDataForRenameReturned(void *pParam,bool bContinue);
	static int KeyboardReturned(void *pParam,bool bSet);
	static void StartGameFromSave(CScene_MultiGameJoinLoad* pClass, DWORD dwLocalUsersMask);
	static int TexturePackDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int SaveTransferDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

	static int Progress(void *pParam,float fProgress);
	static int TransferComplete(void *pParam,int i1, int i2);
	static int DeleteComplete(void *pParam,int i1, int i2);
	static int UploadSaveForXboxOneThreadProc( LPVOID lpParameter );
	static void DeleteFile(CScene_MultiGameJoinLoad *pClass, char *filename);
	static void UploadFile(CScene_MultiGameJoinLoad *pClass, char *filename, LPVOID data, DWORD size);
	static bool WaitForTransferComplete( CScene_MultiGameJoinLoad *pClass );
	static void CancelSaveUploadCallback(LPVOID lpParam);
	static void SaveUploadCompleteCallback(LPVOID lpParam);


	bool m_bIgnoreInput;
	vector<File *> *m_saves;

	int m_iSaveInfoC;
	int m_iDefaultButtonsC;
	int m_iMashUpButtonsC;
	int m_iChangingSaveGameInfoIndex;

	bool m_bMultiplayerAllowed;
	bool m_bKillSaveInfoEnumerate;

	vector<FriendSessionInfo *> currentSessions;
	bool m_bReady;
	bool m_bRetrievingSaveInfo;
	//bool m_bSaveRenamed;
	WCHAR m_wchNewName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
	unsigned char m_szSeed[50];

	vector<LevelGenerationOptions *> *m_generators;
	JoinMenuInitData *m_initData;

	UINT m_DefaultMinecraftIconSize;
	PBYTE m_DefaultMinecraftIconData;
	int *m_iConfigA; // track the texture packs that we don't have installed
	int m_iTexturePacksNotInstalled;
	PBYTE m_pbSaveTransferData;

	float m_fProgress;
	bool m_bTransferComplete;
	bool m_bTransferFail;
	bool m_bSaveTransferInProgress;
};