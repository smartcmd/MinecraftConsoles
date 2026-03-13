#pragma once

#include "../media/xuiscene_settings_All.h"
#include "XUI_Ctrl_SliderWrapper.h"
#include "XUI_CustomMessages.h"

#define BUTTON_ALL_OPTIONS				0
#define BUTTON_ALL_AUDIO				1
#define BUTTON_ALL_CONTROL				2
#define BUTTON_ALL_GRAPHICS				4
#define BUTTON_ALL_UI					5
#define BUTTON_ALL_RESETTODEFAULTS		6
#define BUTTONS_ALL_MAX			BUTTON_ALL_RESETTODEFAULTS + 1

class CScene_SettingsAll : public CXuiSceneImpl
{
protected:

	CXuiControl				m_Buttons[BUTTONS_ALL_MAX];

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiButtonOptions,			m_Buttons[BUTTON_ALL_OPTIONS])
		MAP_CONTROL(IDC_XuiButtonAudio,				m_Buttons[BUTTON_ALL_AUDIO])
		MAP_CONTROL(IDC_XuiButtonControl,			m_Buttons[BUTTON_ALL_CONTROL])
		MAP_CONTROL(IDC_XuiButtonGraphics,			m_Buttons[BUTTON_ALL_GRAPHICS])
		MAP_CONTROL(IDC_XuiButtonUI,				m_Buttons[BUTTON_ALL_UI])
		MAP_CONTROL(IDC_XuiButtonResetToDefaults,	m_Buttons[BUTTON_ALL_RESETTODEFAULTS])
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
	static int ResetDefaultsDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

	int m_iPad;

	D3DXVECTOR3 m_OriginalPosition;

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_SettingsAll, L"CScene_SettingsAll", XUI_CLASS_SCENE )
};
