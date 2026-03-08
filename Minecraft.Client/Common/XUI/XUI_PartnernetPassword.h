#pragma once
#include "../Media/xuiscene_partnernetpassword.h"
#include "XUI_Ctrl_4JEdit.h"

#ifdef _CONTENT_PACKAGE
#ifndef _FINAL_BUILD

class CScene_PartnernetPassword : public CXuiSceneImpl
{

protected:
	CXuiCtrl4JEdit m_PartnernetPassword;
	CXuiControl m_OK;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiEditPartnernetPassword, m_PartnernetPassword)
		MAP_CONTROL(IDC_XuiOK, m_OK)
	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_PartnernetPassword, L"CScene_PartnernetPassword", XUI_CLASS_SCENE )

};

#endif
#endif
