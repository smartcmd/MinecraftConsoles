#pragma once 
#include "../media/xuiscene_fullscreenprogress.h"
#include "../../../Minecraft.World/C4JThread.h"

#define ERROR_FULLSCREENPROGRESS_

class CScene_FullscreenProgress : public CXuiSceneImpl
{
private:
	C4JThread* thread;
	bool threadStarted;
	UIFullscreenProgressCompletionData *m_CompletionData;

	static const int TIMER_FULLSCREEN_PROGRESS = 0;
	static const int TIMER_FULLSCREEN_TIPS = 1;

	static const int TIMER_FULLSCREEN_PROGRESS_TIME = 500;
	static const int TIMER_FULLSCREEN_TIPS_TIME = 7000;
protected:
	// Control and Element wrapper objects.
	CXuiControl m_title, m_status, m_buttonConfirm, m_tip;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_TRANSITION_START( OnTransitionStart )
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_Title, m_title)
		MAP_CONTROL(IDC_Tip, m_tip)
		MAP_CONTROL(IDC_Status, m_status)
		MAP_CONTROL(IDC_ButtonConfirm, m_buttonConfirm)
	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_FullscreenProgress, L"CScene_FullscreenProgress", XUI_CLASS_SCENE )

private:
	bool m_threadCompleted;
	int m_iPad;
	void (*m_cancelFunc)(LPVOID param);
	void (*m_completeFunc)(LPVOID param);
	LPVOID m_cancelFuncParam;
	LPVOID m_completeFuncParam;
	D3DXVECTOR3 m_OriginalPosition;
	bool m_bWasCancelled;

};
