#pragma once

#include "../media/xuiscene_intro.h"

class CScene_Intro : public CXuiSceneImpl
{
	protected:
		CXuiScene m_Scene;
		CXuiControl m_4jlogo;
		CXuiElement m_grpXbox;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_TIMELINE_END(OnTimelineEnd)
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		//MAP_CONTROL(IDC_LogoGroup, m_grpXbox)
		//BEGIN_MAP_CHILD_CONTROLS(m_grpXbox)
			MAP_CONTROL(IDC_Logo4J, m_4jlogo)
		//END_MAP_CHILD_CONTROLS()  

	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnTimelineEnd(HXUIOBJ hObjSource, BOOL& bHandled);
	HRESULT OnTimer(XUIMessageTimer *pData,BOOL& rfHandled);

	bool m_bSkippable;
	bool m_bWantsToSkip;
	int m_iTimeline;
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_Intro, L"CScene_Intro", XUI_CLASS_SCENE )

};
