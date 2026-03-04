#pragma once

#include "../media/xuiscene_savemessage.h"

class CScene_SaveMessage : public CXuiSceneImpl
{
	protected:
		CXuiControl m_button;
		CXuiControl m_SaveMessage;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_ConfirmButton, m_button)
		MAP_CONTROL(IDC_Description, m_SaveMessage)
	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnTimer( XUIMessageTimer *pXUIMessageTimer, BOOL &bHandled);

	bool m_bIgnoreInput;
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_SaveMessage, L"CScene_SaveMessage", XUI_CLASS_SCENE )

};
