#pragma once

#include "../media\xuiscene_NewUpdateMessage.h"

class CScene_NewUpdateMessage : public CXuiSceneImpl
{
	// Xui Elements
	CXuiHtmlControl m_HTMLText;
	// Misc
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_NAV_RETURN(OnNavReturn)

	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiHTMLMessage, m_HTMLText)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);

public:
	XUI_IMPLEMENT_CLASS( CScene_NewUpdateMessage, L"CScene_NewUpdateMessage", XUI_CLASS_SCENE )

private:
	int m_iPad;
	bool m_bIsSD;
};
