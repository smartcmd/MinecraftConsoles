#pragma once

#include "../media/xuiscene_main.h"
#include "XUI_CustomMessages.h"

#define BUTTON_PLAYGAME			0
#define BUTTON_LEADERBOARDS		1
#define BUTTON_ACHIEVEMENTS		2
#define BUTTON_HELPANDOPTIONS	3
#define BUTTON_UNLOCKFULLGAME	4
#define	BUTTON_EXITGAME			5
#define BUTTONS_MAX				BUTTON_EXITGAME + 1

#define MAIN_MENU_MAX_TEXT_SCALE 1.5f

class Random;

class CScene_Main : public CXuiSceneImpl
{
private:
	static Random *random;
	vector<wstring> m_splashes;
	D3DXVECTOR3 m_vPosExitGame;
	bool m_bIgnorePress;
	float m_fSubtitleHeight, m_fSubtitleWidth;
	CXuiControl m_Timer;

	// 4J Added
	enum eSplashIndexes
	{
		eSplashHappyBirthdayEx = 0,
		eSplashHappyBirthdayNotch,
		eSplashMerryXmas,
		eSplashHappyNewYear,

		// The start index in the splashes vector from which we can select a random splash
		eSplashRandomStart,
	};

	enum eActions
	{
		eAction_None=0,
		eAction_RunGame,
		eAction_RunLeaderboards,
		eAction_RunAchievements,
		eAction_RunHelpAndOptions,
		eAction_RunUnlockOrDLC,
	};

protected:
	// Control and Element wrapper objects.
	CXuiScene	m_Scene;
	CXuiControl m_Buttons[BUTTONS_MAX];
	CXuiControl m_Subtitle, m_SubtitleMCFont;
	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_NOTIFY_SET_FOCUS(OnNotifySetFocus)
		XUI_ON_XM_TMS_BANFILE_RETRIEVED_MESSAGE(OnTMSBanFileRetrieved)
		XUI_ON_XM_TMS_DLCFILE_RETRIEVED_MESSAGE(OnTMSDLCFileRetrieved)
		XUI_ON_XM_TIMER( OnTimer )

	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiButton1, m_Buttons[BUTTON_PLAYGAME])
		MAP_CONTROL(IDC_XuiButton2, m_Buttons[BUTTON_LEADERBOARDS  ])
		MAP_CONTROL(IDC_XuiButton3, m_Buttons[BUTTON_ACHIEVEMENTS  ])
		MAP_CONTROL(IDC_XuiButton4, m_Buttons[BUTTON_HELPANDOPTIONS])
		MAP_CONTROL(IDC_XuiButton5, m_Buttons[BUTTON_UNLOCKFULLGAME])
		MAP_CONTROL(IDC_XuiButton6, m_Buttons[BUTTON_EXITGAME])
		MAP_CONTROL(IDC_XuiSplash, m_Subtitle)
		MAP_CONTROL(IDC_XuiSplashMCFont, m_SubtitleMCFont)
		MAP_CONTROL(IDC_Timer, m_Timer)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnNotifySetFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled);
	HRESULT OnTMSBanFileRetrieved();
	HRESULT OnTMSDLCFileRetrieved( );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

	int SetSaveDevice();
	static void LoadTrial();

	void RunPlayGame(int iPad);
	void RunLeaderboards(int iPad);
	void RunAchievements(int iPad);
	void RunHelpAndOptions(int iPad);
	void RunUnlockOrDLC(int iPad);

	eActions m_eAction;

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_Main, L"CScene_Main", XUI_CLASS_SCENE )

	static int SignInReturned(void *pParam,bool bContinue);
	static int CreateLoad_SignInReturned(void *pParam,bool bContinue, int iPad);
	static int CreateLoad_OfflineProfileReturned(void *pParam,bool bContinue, int iPad);
	static int DeviceSelectReturned(void *pParam,bool bContinue);
	static int SaveGameReturned(void *pParam,bool bContinue);
	static int HelpAndOptions_SignInReturned(void *pParam,bool bContinue,int iPad);
	static int ExitGameReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int AchievementsDeviceSelectReturned(void *pParam,bool bContinue);
	static int Achievements_SignInReturned(void *pParam,bool bContinue,int iPad);
	static int Leaderboards_SignInReturned(void* pParam, bool bContinue, int iPad);
	static int UnlockFullGame_SignInReturned(void *pParam,bool bContinue,int iPad);
	static int MustSignInReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
#ifdef _XBOX
	static int TMSReadFileListReturned(void *pParam,int iPad,C4JStorage::PTMSPP_FILE_LIST pTmsFileList);
	static int TMSFileWriteReturned(void *pParam,int iPad,int iResult);
	static int TMSFileReadReturned(void *pParam,int iPad,C4JStorage::PTMSPP_FILEDATA pData);
#endif
};
