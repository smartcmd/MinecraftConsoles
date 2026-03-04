#pragma once

#include "../media/xuiscene_debug.h"

class CScene_Debug : public CXuiSceneImpl
{
	protected:


	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_CONTROL_NAVIGATE(OnControlNavigate)
	XUI_END_MSG_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
public:


	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_Debug, L"CScene_Debug", XUI_CLASS_SCENE )

private:

	typedef struct  
	{
		HXUIOBJ hXuiObj;
		VOID *pvData;
	}
	DEBUGDATA;	


	static LPCWSTR m_DebugCheckboxTextA[eDebugSetting_Max+1];
	static LPCWSTR m_DebugButtonTextA[eDebugButton_Max+1];
	int m_iTotalCheckboxElements;
	int m_iTotalButtonElements;
	DEBUGDATA *m_DebugCheckboxDataA;
	DEBUGDATA *m_DebugButtonDataA;
	int m_iCurrentCheckboxElement;
	int m_iCurrentButtonElement;
	int m_iPad;
	bool m_bOnCheckboxes; // for navigations
};
