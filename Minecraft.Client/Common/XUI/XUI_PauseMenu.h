#pragma once 
#include "../media/xuiscene_pause.h"
#include "../UI/IUIScene_PauseMenu.h"
#include "XUI_CustomMessages.h"

#define BUTTON_PAUSE_RESUMEGAME		0
#define BUTTON_PAUSE_HELPANDOPTIONS	1
#define BUTTON_PAUSE_LEADERBOARDS		2
#define BUTTON_PAUSE_ACHIEVEMENTS		3
#define BUTTON_PAUSE_SAVEGAME			4
#define	BUTTON_PAUSE_EXITGAME			5
#define BUTTONS_PAUSE_MAX				BUTTON_PAUSE_EXITGAME + 1



class UIScene_PauseMenu : public CXuiSceneImpl, public IUIScene_PauseMenu
{
	protected:
	// Control and Element wrapper objects.
	CXuiScene	m_Scene;
	CXuiControl m_Buttons[BUTTONS_PAUSE_MAX];
	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_DESTROY(OnDestroy)
		XUI_ON_XM_DLCINSTALLED_MESSAGE(OnCustomMessage_DLCInstalled)
		XUI_ON_XM_DLCLOADED_MESSAGE(OnCustomMessage_DLCMountingComplete)
	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiButton1, m_Buttons[BUTTON_PAUSE_RESUMEGAME])
		MAP_CONTROL(IDC_XuiButton2, m_Buttons[BUTTON_PAUSE_HELPANDOPTIONS  ])
		MAP_CONTROL(IDC_XuiButton3, m_Buttons[BUTTON_PAUSE_LEADERBOARDS  ])
		MAP_CONTROL(IDC_XuiButton4, m_Buttons[BUTTON_PAUSE_ACHIEVEMENTS])
		MAP_CONTROL(IDC_XuiButton5, m_Buttons[BUTTON_PAUSE_SAVEGAME])
		MAP_CONTROL(IDC_XuiButton6, m_Buttons[BUTTON_PAUSE_EXITGAME])
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnTimer(XUIMessageTimer *pData,BOOL& rfHandled);
	HRESULT OnDestroy();
	HRESULT OnCustomMessage_DLCInstalled();
	HRESULT OnCustomMessage_DLCMountingComplete();

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( UIScene_PauseMenu, L"CScene_Pause", XUI_CLASS_SCENE )

	static int BanGameDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int DeviceSelectReturned(void *pParam,bool bContinue);
	static int DeviceRemovedDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int WarningTrialTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int ExitGameSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int ExitGameDeclineSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int ExitGameAndSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int SaveGameDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int ExitGameDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

	static int SaveWorldThreadProc( void* lpParameter );
	static int ExitWorldThreadProc( void* lpParameter );
	static void _ExitWorld(LPVOID lpParameter); // Call only from a thread


protected:
	virtual void ShowScene(bool show);
	virtual void SetIgnoreInput(bool ignoreInput);

private:
	int m_iPad;
	int m_iLastButtonPressed;
	D3DXVECTOR3 m_OriginalPosition;
	bool m_bIgnoreInput;
	bool m_bSplitscreen;

};
