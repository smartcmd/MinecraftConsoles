#pragma once

#define CREDITS_TICK_TIMER_ID	(6)				// Arbitrary timer ID used to tick credits for scrolling.

#define MAX_CREDIT_STRINGS		360
// 213

#include "..\UI\UIStructs.h"

class CScene_Credits : public CXuiSceneImpl
{
protected:
	// Control and Element wrapper objects.

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_CONTROL_NAVIGATE(OnControlNavigate)
        XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_DESTROY( OnDestroy )
	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()

	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT	OnDestroy();

private:


	struct STextType
	{
		// Array of pointers to text elements.
		CXuiControl**		m_appTextElements;

		int					m_iNextFreeElement;
		int					m_iNumUsedElements;
		int					m_iMaxElements;
	};

	STextType		m_aTextTypes[ eNumTextTypes ];

	int		m_iCurrDefIndex;		// Index of last created text def.
	float	m_fMoveSinceLastDef;	// How far have credits scrolled since we last created a new text item.
	float	m_fMoveToNextDef;		// How far we need to move before starting next text item.
	int		m_iNumTextDefs;			// Total number of text defs in the credits.

	float CREDITS_SCREEN_MIN_Y;//	( 200.0f )		// Y pos at which credits are removed from top of screen.
	float CREDITS_SCREEN_MAX_Y;//	( 630.0f )		// Y pos at which credits appear at bottom of screen.
	float CREDITS_FADE_HEIGHT;//		( 100.0f )		// Height over which credits fade in or fade out.

	float gs_aLineSpace[ eNumTextTypes ];

public:
	static SCreditTextItemDef gs_aCreditDefs[MAX_CREDIT_STRINGS];

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_Credits, L"CScene_Credits", XUI_CLASS_SCENE )



};

