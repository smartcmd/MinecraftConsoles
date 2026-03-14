#pragma once

#include "../media/xuiscene_debugtips.h"




class CScene_DebugTips : public CXuiSceneImpl
{
	protected:
	// Control and Element wrapper objects.
	CXuiControl m_tip;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_Tip, m_tip)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_DebugTips, L"CScene_DebugTips", XUI_CLASS_SCENE )

private:
	bool m_bIgnoreInput;
	int m_iPad;
	D3DXVECTOR3 m_OriginalPosition;

};
