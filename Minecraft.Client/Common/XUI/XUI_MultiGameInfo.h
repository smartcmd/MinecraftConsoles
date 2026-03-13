#pragma once
using namespace std;
#include <vector>
#include <qnet.h>
#include "../Media/xuiscene_multi_gameinfo.h"

class FriendSessionInfo;

class CScene_MultiGameInfo : public CXuiSceneImpl
{
protected:
		CXuiList playersList;
		CXuiControl m_JoinGame;
		CXuiControl m_GameSettingsGroup;
		CXuiControl m_difficulty, m_GameType, m_gamertagsOn, m_structuresOn, m_levelType, m_pvpOn, m_trustPlayers, m_tntOn, m_fireOn;
		CXuiControl m_labelDifficulty, m_labelGameType, m_labelGamertagsOn, m_labelStructuresOn, m_labelLevelType, m_labelPvP, m_labelTrust, m_labelTNTOn, m_labelFireOn;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_NOTIFY_SELCHANGED(OnNotifySelChanged)
		XUI_ON_XM_NOTIFY_SET_FOCUS(OnNotifySetFocus)
		XUI_ON_XM_NOTIFY_KILL_FOCUS(OnNotifyKillFocus)
		XUI_ON_XM_TIMER(OnTimer)

	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_JoinGame, m_JoinGame)
		MAP_CONTROL(IDC_GamePlayers, playersList)

		MAP_CONTROL(IDC_GameSettings, m_GameSettingsGroup)
		BEGIN_MAP_CHILD_CONTROLS(m_GameSettingsGroup)
			MAP_CONTROL(IDC_GamertagsOn, m_gamertagsOn)
			MAP_CONTROL(IDC_StructuresOn, m_structuresOn)
			MAP_CONTROL(IDC_Difficulty, m_difficulty)
			MAP_CONTROL(IDC_GameType, m_GameType)
			MAP_CONTROL(IDC_LevelType, m_levelType)
			MAP_CONTROL(IDC_PvP, m_pvpOn)
			MAP_CONTROL(IDC_Trust, m_trustPlayers)
			MAP_CONTROL(IDC_TNTOn, m_tntOn)
			MAP_CONTROL(IDC_FireOn, m_fireOn)

			MAP_CONTROL(IDC_LabelGamertagsOn, m_labelGamertagsOn)
			MAP_CONTROL(IDC_LabelStructuresOn, m_labelStructuresOn)
			MAP_CONTROL(IDC_LabelDifficulty, m_labelDifficulty)
			MAP_CONTROL(IDC_LabelGameType, m_labelGameType)
			MAP_CONTROL(IDC_LabelLevelType, m_labelLevelType)
			MAP_CONTROL(IDC_LabelPvP, m_labelPvP)
			MAP_CONTROL(IDC_LabelTrust, m_labelTrust)
			MAP_CONTROL(IDC_LabelTNTOn, m_labelTNTOn)
			MAP_CONTROL(IDC_LabelFireOn, m_labelFireOn)
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
	HRESULT OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled);
	HRESULT OnNotifySetFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled);
	HRESULT OnNotifyKillFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_MultiGameInfo, L"CScene_MultiGameInfo", XUI_CLASS_SCENE )



protected:
	FriendSessionInfo *m_selectedSession;
	unsigned char m_localPlayers;
	bool m_bIgnoreInput;
	int m_iPad;

	static int StartGame_SignInReturned(void *pParam,bool bContinue, int iPad);
	static void JoinGame(CScene_MultiGameInfo* pClass);

};