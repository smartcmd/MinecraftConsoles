#pragma once

#include "../media/xuiscene_win.h"
#include "XUI_CustomMessages.h"

class CScene_Win : public CXuiSceneImpl
{
private:
	static const float AUTO_SCROLL_SPEED;
	static const float PLAYER_SCROLL_SPEED;
protected:
	CXuiHtmlControl m_htmlControl;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_KEYUP(OnKeyUp)
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)		
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_NAV_FORWARD(OnNavForward)
	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_HtmlControl, m_htmlControl)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pXUIMessageTimer, BOOL &bHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnKeyUp(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
	HRESULT OnNavForward(XUIMessageNavForward *pNavForwardData, BOOL& bHandled);
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_Win, L"CScene_Win", XUI_CLASS_SCENE )

private:
	bool m_bIgnoreInput;
	int m_iPad;
	D3DXVECTOR3 m_OriginalPosition;
	wstring noNoiseString;
	wstring noiseString;
	int m_scrollDir;

	vector<int> m_noiseLengths;

	void updateNoise();

public:
	static BYTE s_winUserIndex;
	static void setWinUserIndex(BYTE winUserIndex) { s_winUserIndex = winUserIndex; }
};