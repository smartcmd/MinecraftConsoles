#pragma once

#include "../media/xuiscene_helpandoptions.h"
#include "XUI_CustomMessages.h"

#define BUTTON_HAO_CHANGESKIN			0
#define BUTTON_HAO_HOWTOPLAY			1
#define BUTTON_HAO_CONTROLS				2
#define BUTTON_HAO_SETTINGS				3
#define BUTTON_HAO_CREDITS				4
#define BUTTON_HAO_REINSTALL			5
#define BUTTON_HAO_DEBUG				6
#define BUTTONS_HAO_MAX			BUTTON_HAO_DEBUG + 1



class CScene_HelpAndOptions : public CXuiSceneImpl
{
protected:
	// Control and Element wrapper objects.
	CXuiScene	m_Scene;
	CXuiControl m_Buttons[BUTTONS_HAO_MAX];
	CXuiElement m_Background;	
	CXuiControl m_Timer;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
		//XUI_ON_XM_TMS_DLCFILE_RETRIEVED_MESSAGE(OnTMSDLCFileRetrieved)
	XUI_END_MSG_MAP()

		// Control mapping to objects
		BEGIN_CONTROL_MAP()
			MAP_CONTROL(IDC_XuiButton1, m_Buttons[BUTTON_HAO_CHANGESKIN])
			MAP_CONTROL(IDC_XuiButton2, m_Buttons[BUTTON_HAO_HOWTOPLAY  ])
			MAP_CONTROL(IDC_XuiButton3, m_Buttons[BUTTON_HAO_CONTROLS  ])
			MAP_CONTROL(IDC_XuiButton4, m_Buttons[BUTTON_HAO_SETTINGS  ])
			MAP_CONTROL(IDC_XuiButton5, m_Buttons[BUTTON_HAO_CREDITS])
			MAP_CONTROL(IDC_XuiButton6, m_Buttons[BUTTON_HAO_REINSTALL])
			MAP_CONTROL(IDC_XuiButton7, m_Buttons[BUTTON_HAO_DEBUG])
			MAP_CONTROL(IDC_Timer, m_Timer)

		END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	//HRESULT OnTMSDLCFileRetrieved( );
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_HelpAndOptions, L"CScene_HelpAndOptions", XUI_CLASS_SCENE )

private:

	int m_iPad;
	bool m_bIgnoreInput;
	D3DXVECTOR3 m_OriginalPosition;
};
