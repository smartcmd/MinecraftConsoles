#pragma once
using namespace std;
#include "../Media/xuiscene_multi_launch_more_options.h"

class CScene_MultiGameLaunchMoreOptions : public CXuiSceneImpl
{
protected:
	CXuiScene m_GenerationGroup, m_HostOptionGroup;
	CXuiCheckbox m_CheckboxOnline;
	CXuiCheckbox m_CheckboxInviteOnly;
	CXuiCheckbox m_CheckboxAllowFoF;
	CXuiCheckbox m_CheckboxStructures;
	CXuiCheckbox m_CheckboxFlatWorld;
	CXuiCheckbox m_CheckboxBonusChest;
	CXuiCheckbox m_CheckboxPVP;
	CXuiCheckbox m_CheckboxTrustPlayers;
	CXuiCheckbox m_CheckboxFireSpreads;
	CXuiCheckbox m_CheckboxTNTExplodes;
	CXuiCheckbox m_CheckboxHostPrivileges;
	CXuiCheckbox m_CheckboxResetNether;
	CXuiControl m_Description;
	CXuiControl m_LabelWorldOptions;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_SET_FOCUS( OnNotifySetFocus )
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_GenerationOptions, m_GenerationGroup)
		BEGIN_MAP_CHILD_CONTROLS(m_GenerationGroup)
			MAP_CONTROL(IDC_WorldOptions, m_LabelWorldOptions)
			MAP_CONTROL(IDC_CheckboxStructures, m_CheckboxStructures)
			MAP_CONTROL(IDC_CheckboxFlatWorld, m_CheckboxFlatWorld)
			MAP_CONTROL(IDC_CheckboxBonusChest, m_CheckboxBonusChest)	
		END_MAP_CHILD_CONTROLS()
		
		MAP_CONTROL(IDC_HostOptions, m_HostOptionGroup)
		BEGIN_MAP_CHILD_CONTROLS(m_HostOptionGroup)
			MAP_CONTROL(IDC_CheckboxOnline, m_CheckboxOnline)
			MAP_CONTROL(IDC_CheckboxInviteOnly, m_CheckboxInviteOnly)
			MAP_CONTROL(IDC_CheckboxAllowFoF, m_CheckboxAllowFoF)
			MAP_CONTROL(IDC_CheckboxPVP, m_CheckboxPVP)
			MAP_CONTROL(IDC_CheckboxTrustSystem, m_CheckboxTrustPlayers)
			MAP_CONTROL(IDC_CheckboxFireSpreads, m_CheckboxFireSpreads)
			MAP_CONTROL(IDC_CheckboxTNT, m_CheckboxTNTExplodes)
			MAP_CONTROL(IDC_CheckboxHostPrivileges, m_CheckboxHostPrivileges)
			MAP_CONTROL(IDC_CheckboxResetNether, m_CheckboxResetNether)
		END_MAP_CHILD_CONTROLS()
		
		MAP_CONTROL(IDC_Description, m_Description)
	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);	
	HRESULT OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled );
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_MultiGameLaunchMoreOptions, L"CScene_MultiGameLaunchMoreOptions", XUI_CLASS_SCENE )

private:
	LaunchMoreOptionsMenuInitData *m_params;
	bool m_bMultiplayerAllowed;
};