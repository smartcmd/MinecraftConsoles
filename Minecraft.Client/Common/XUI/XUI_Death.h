#pragma once

#include "../media/xuiscene_death.h"
#include "XUI_CustomMessages.h"

#define BUTTON_DEATH_RESPAWN		0
#define	BUTTON_DEATH_EXITGAME		1
#define BUTTONS_DEATH_MAX			BUTTON_DEATH_EXITGAME + 1



class CScene_Death : public CXuiSceneImpl
{
	protected:
	// Control and Element wrapper objects.
	CXuiScene	m_Scene;
	CXuiControl m_Buttons[BUTTONS_DEATH_MAX];
	CXuiControl m_Title;
	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_Respawn, m_Buttons[BUTTON_DEATH_RESPAWN])
		MAP_CONTROL(IDC_ExitGame, m_Buttons[BUTTON_DEATH_EXITGAME])
		MAP_CONTROL(IDC_Title, m_Title)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_Death, L"CScene_Death", XUI_CLASS_SCENE )
		
	static int RespawnThreadProc( void* lpParameter );
private:
	bool m_bIgnoreInput;
	int m_iPad;
	D3DXVECTOR3 m_OriginalPosition;

};
