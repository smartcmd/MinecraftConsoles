#pragma once

#include "../media/xuiscene_settings_Graphics.h"
#include "XUI_Ctrl_SliderWrapper.h"
#include "XUI_CustomMessages.h"


#define SLIDER_SETTINGS_GAMMA	0
#define SLIDER_SETTINGS_INTERFACE_OPACITY		1


#define SLIDER_SETTINGS_GRAPHICS_MAX				SLIDER_SETTINGS_INTERFACE_OPACITY + 1
class CScene_SettingsGraphics : public CXuiSceneImpl
{
protected:
	CXuiCtrlSliderWrapper	m_SliderA[SLIDER_SETTINGS_GRAPHICS_MAX];
	CXuiCheckbox			m_Clouds;
	CXuiCheckbox			m_BedrockFog;
	CXuiCheckbox			m_CustomSkinAnim;

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
		MAP_CONTROL(IDC_XuiClouds,						m_Clouds)
		MAP_CONTROL(IDC_XuiBedrockFog,					m_BedrockFog)
		MAP_CONTROL(IDC_XuiCustomSkinAnim,				m_CustomSkinAnim)
		MAP_CONTROL(IDC_XuiSliderGamma,					m_SliderA[SLIDER_SETTINGS_GAMMA])
		MAP_CONTROL(IDC_XuiSliderInterfaceOpacity,		m_SliderA[SLIDER_SETTINGS_INTERFACE_OPACITY])
		
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
	XUI_IMPLEMENT_CLASS( CScene_SettingsGraphics, L"CScene_SettingsGraphics", XUI_CLASS_SCENE )
};
