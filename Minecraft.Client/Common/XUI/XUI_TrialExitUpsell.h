#pragma once

#include "../media/xuiscene_trialexitupsell.h"

#define TRIAL_EXIT_UPSELL_IMAGE_DISPLAY_TIME 3000

#define TRIAL_EXIT_UPSELL_IMAGE_COUNT 8

class CScene_TrialExitUpsell : public CXuiSceneImpl
{
private:
	static WCHAR *wchImages[TRIAL_EXIT_UPSELL_IMAGE_COUNT];

protected:
	CXuiImageElement m_image1, m_image2;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_TIMELINE_END(OnTimelineEnd)
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiImage1, m_image1)
		MAP_CONTROL(IDC_XuiImage2, m_image2)
	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnTimelineEnd(HXUIOBJ hObjSource, BOOL& bHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	
	int m_iPad;
	bool m_bCanExit;
	bool m_bFadeStarted;
	bool m_bShowingImage1;
	unsigned char m_imagesShown;
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_TrialExitUpsell, L"CScene_TrialExitUpsell", XUI_CLASS_SCENE )

};
