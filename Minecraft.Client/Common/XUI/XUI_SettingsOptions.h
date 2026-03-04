#pragma once

#include "../media/xuiscene_settings_options.h"
#include "XUI_Ctrl_SliderWrapper.h"
#include "XUI_CustomMessages.h"

#define SLIDER_SETTINGS_AUTOSAVE 		0
#define SLIDER_SETTINGS_DIFFICULTY		1

#define OPTIONS_SLIDER_SETTINGS_MAX				SLIDER_SETTINGS_DIFFICULTY + 1
class CScene_SettingsOptions : public CXuiSceneImpl
{
protected:
	CXuiCtrlSliderWrapper	m_SliderA[OPTIONS_SLIDER_SETTINGS_MAX];
	CXuiControl				m_DifficultyText;
	CXuiCheckbox			m_ViewBob;
	CXuiCheckbox			m_InGameGamertags;
	CXuiCheckbox			m_Hints;
	CXuiCheckbox			m_Tooltips;
	CXuiCheckbox			m_MashUpWorlds;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiViewBob,						m_ViewBob)
		MAP_CONTROL(IDC_XuiInGameGamertags,				m_InGameGamertags)
		MAP_CONTROL(IDC_XuiShowHints,					m_Hints)
		MAP_CONTROL(IDC_XuiShowTooltips,				m_Tooltips)
		MAP_CONTROL(IDC_XuiSliderAutosave,				m_SliderA[SLIDER_SETTINGS_AUTOSAVE])
		MAP_CONTROL(IDC_XuiSliderDifficulty,			m_SliderA[SLIDER_SETTINGS_DIFFICULTY])
		MAP_CONTROL(IDC_XuiDifficultyText,				m_DifficultyText)
		MAP_CONTROL(IDC_XuiMashUpWorlds,				m_MashUpWorlds)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled );
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);

	int m_iPad;
	static int m_iDifficultySettingA[4];
	static int m_iDifficultyTitleSettingA[4];
	static int m_iAutosaveSettingA[4];

	D3DXVECTOR3 m_OriginalPosition;

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_SettingsOptions, L"CScene_SettingsOptions", XUI_CLASS_SCENE )

private:
	bool m_bMashUpWorldsUnhideOption;
};
