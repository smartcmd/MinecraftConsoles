#pragma once
#include "../media/xuiscene_signentry.h"
#include "XUI_Ctrl_4JEdit.h"
#include "XUI_CustomMessages.h"

#define SIGN_ENTRY_ROWS_MAX 4

class SignTileEntity;

class CScene_SignEntry : public CXuiSceneImpl
{
	protected:
	// Control and Element wrapper objects.
	CXuiCtrl4JEdit m_signRows[4];
	CXuiControl m_ButtonDone;
	CXuiControl m_labelEditSign;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
	XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_EditSignMessage, m_labelEditSign)
		MAP_CONTROL(IDC_ButtonDone, m_ButtonDone)
		MAP_CONTROL(IDC_EditLineOne, m_signRows[0])
		MAP_CONTROL(IDC_EditLineTwo, m_signRows[1])
		MAP_CONTROL(IDC_EditLineThree, m_signRows[2])
		MAP_CONTROL(IDC_EditLineFour, m_signRows[3])
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_SignEntry, L"CScene_SignEntry", XUI_CLASS_SCENE )

private:
	shared_ptr<SignTileEntity> m_sign;
	D3DXVECTOR3 m_OriginalPosition;

};