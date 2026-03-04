#pragma once

#include "../media/xuiscene_settings_UI.h"
#include "XUI_Ctrl_SliderWrapper.h"
#include "XUI_CustomMessages.h"


#define SLIDER_SETTINGS_UISIZE	0
#define SLIDER_SETTINGS_UISIZESPLITSCREEN		1


#define SLIDER_SETTINGS_UI_MAX				SLIDER_SETTINGS_UISIZESPLITSCREEN + 1
class CScene_SettingsUI : public CXuiSceneImpl
{
protected:
	CXuiCtrlSliderWrapper	m_SliderA[SLIDER_SETTINGS_UI_MAX];
	CXuiCheckbox			m_DisplayHUD;
	CXuiCheckbox			m_DisplayHand;
	CXuiCheckbox			m_DeathMessages;
	CXuiCheckbox			m_AnimatedCharacter;
	CXuiCheckbox			m_SplitScreen;
	CXuiCheckbox			m_SplitScreenGamertags;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiSplitScreen,					m_SplitScreen)
		MAP_CONTROL(IDC_XuiShowSplitscreenGamertags,	m_SplitScreenGamertags)
		MAP_CONTROL(IDC_XuiDisplayHUD,					m_DisplayHUD)
		MAP_CONTROL(IDC_XuiDisplayHand,					m_DisplayHand)
		MAP_CONTROL(IDC_XuiDisplayDeathMessages,		m_DeathMessages)
		MAP_CONTROL(IDC_XuiShowAnimatedCharacter,		m_AnimatedCharacter)
		MAP_CONTROL(IDC_XuiSliderUISize,				m_SliderA[SLIDER_SETTINGS_UISIZE])
		MAP_CONTROL(IDC_XuiSliderUISizeSplitscreen,		m_SliderA[SLIDER_SETTINGS_UISIZESPLITSCREEN])
		
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled );
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);

	int m_iPad;
	D3DXVECTOR3 m_OriginalPosition;

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_SettingsUI, L"CScene_SettingsUI", XUI_CLASS_SCENE )
};
