#pragma once

#include "../media/xuiscene_reinstall.h"

#define BUTTON_REINSTALL_THEME		0
#define BUTTON_REINSTALL_GAMERPIC1	1
#define BUTTON_REINSTALL_GAMERPIC2	2
#define BUTTON_REINSTALL_AVATAR1	3
#define BUTTON_REINSTALL_AVATAR2	4
#define	BUTTON_REINSTALL_AVATAR3	5
#define BUTTONS_REINSTALL_MAX				BUTTON_REINSTALL_AVATAR3 + 1



class CScene_Reinstall : public CXuiSceneImpl
{
	protected:
	// Control and Element wrapper objects.
	CXuiControl m_Buttons[BUTTONS_REINSTALL_MAX];
	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)	
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiButton1, m_Buttons[BUTTON_REINSTALL_THEME		])
		MAP_CONTROL(IDC_XuiButton2, m_Buttons[BUTTON_REINSTALL_GAMERPIC1	])
		MAP_CONTROL(IDC_XuiButton3, m_Buttons[BUTTON_REINSTALL_GAMERPIC2	])
		MAP_CONTROL(IDC_XuiButton4, m_Buttons[BUTTON_REINSTALL_AVATAR1	])
		MAP_CONTROL(IDC_XuiButton5, m_Buttons[BUTTON_REINSTALL_AVATAR2	])
		MAP_CONTROL(IDC_XuiButton6, m_Buttons[BUTTON_REINSTALL_AVATAR3	])
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );

	static int DeviceSelectReturned(void *pParam,bool bContinue);
	static int DeviceSelectReturned_AndReinstall(void *pParam,bool bContinue);
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_Reinstall, L"CScene_Reinstall", XUI_CLASS_SCENE )
		
	//static int InstallReturned(void *pParam,bool bContinue);

private:
	int m_iPad;
	int m_iLastButtonPressed;
	bool m_bIgnoreInput;


};
