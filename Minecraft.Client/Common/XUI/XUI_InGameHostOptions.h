#pragma once
#include "..\Media\xuiscene_ingame_host_options.h"

class CScene_InGameHostOptions : public CXuiSceneImpl
{
protected:
	CXuiScene m_GameOptionsGroup;
	CXuiCheckbox m_CheckboxFireSpreads;
	CXuiCheckbox m_CheckboxTNTExplodes;
	CXuiControl m_buttonTeleportToPlayer, m_buttonTeleportToMe;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()		
		MAP_CONTROL(IDC_GameOptions, m_GameOptionsGroup)
		BEGIN_MAP_CHILD_CONTROLS(m_GameOptionsGroup)
			MAP_CONTROL(IDC_CheckboxFireSpreads, m_CheckboxFireSpreads)
			MAP_CONTROL(IDC_CheckboxTNT, m_CheckboxTNTExplodes)
			MAP_CONTROL(IDC_ButtonTeleportToPlayer, m_buttonTeleportToPlayer)
			MAP_CONTROL(IDC_ButtonTeleportPlayerToMe, m_buttonTeleportToMe)
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_InGameHostOptions, L"CScene_InGameHostOptions", XUI_CLASS_SCENE )

private:
	int m_iPad;
	D3DXVECTOR3 m_OriginalPosition;
	HXUIOBJ m_focusElement; // Only used for the remove control process

	void removeControl(HXUIOBJ hObjToRemove, bool center);
};