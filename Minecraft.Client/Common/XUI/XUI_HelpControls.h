#pragma once

#include "../media/xuiscene_controls.h"
#include "XUI_CustomMessages.h"

class CXuiCtrl4JList;



#define MAX_CONTROLS 17
class CScene_Controls : public CXuiSceneImpl
{
	protected:
	// Control and Element wrapper objects.
	CXuiControl m_TextA[MAX_CONTROLS];
	HXUIOBJ m_TextPresenterA[MAX_CONTROLS];
	CXuiElement m_FigA[MAX_CONTROLS];
	CXuiList m_SchemeList;
	CXuiElement m_Group;
	CXuiControl m_BuildVer;
	CXuiControl m_CurrentLayout;
	CXuiCheckbox m_InvertLook;
	CXuiCheckbox m_SouthPaw;
	static LPCWSTR m_LayoutNameA[3];

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)

	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_SchemeList, m_SchemeList)
		MAP_CONTROL(IDC_CurrentLayout, m_CurrentLayout)
		
		MAP_CONTROL(IDC_XuiBuildVer,		m_BuildVer)
		MAP_CONTROL(IDC_XuiLabel1,		m_TextA[0])
		MAP_CONTROL(IDC_XuiLabel2,		m_TextA[1])
		MAP_CONTROL(IDC_XuiLabel3,		m_TextA[2])
		MAP_CONTROL(IDC_XuiLabel4,		m_TextA[3])
		MAP_CONTROL(IDC_XuiLabel5,		m_TextA[4])
		MAP_CONTROL(IDC_XuiLabel6,		m_TextA[5])
		MAP_CONTROL(IDC_XuiLabel7,		m_TextA[6])
		MAP_CONTROL(IDC_XuiLabel8,		m_TextA[7])
		MAP_CONTROL(IDC_XuiLabel9,		m_TextA[8])
		MAP_CONTROL(IDC_XuiLabel10,		m_TextA[9])
		MAP_CONTROL(IDC_XuiLabel11,		m_TextA[10])
		MAP_CONTROL(IDC_XuiLabel12,		m_TextA[11])
		MAP_CONTROL(IDC_XuiLabel13,		m_TextA[12])
		MAP_CONTROL(IDC_XuiLabel14,		m_TextA[13])
		MAP_CONTROL(IDC_XuiLabel15,		m_TextA[14])
		MAP_CONTROL(IDC_XuiLabel16,		m_TextA[15])
		MAP_CONTROL(IDC_XuiLabel17,		m_TextA[16])
		MAP_CONTROL(IDC_FigGroup,		m_Group)
		MAP_CONTROL(IDC_InvertLook,		m_InvertLook)
		MAP_CONTROL(IDC_SouthPaw,		m_SouthPaw)
		BEGIN_MAP_CHILD_CONTROLS(m_Group)
		MAP_CONTROL(IDC_A,				m_FigA[0])
		MAP_CONTROL(IDC_B,				m_FigA[1])
		MAP_CONTROL(IDC_X,				m_FigA[2])
		MAP_CONTROL(IDC_Y,				m_FigA[3])
		MAP_CONTROL(IDC_Start,			m_FigA[4])
		MAP_CONTROL(IDC_Back,			m_FigA[5])
		MAP_CONTROL(IDC_RB,				m_FigA[6])
		MAP_CONTROL(IDC_LB,				m_FigA[7])
		MAP_CONTROL(IDC_RStickButton,	m_FigA[8])
		MAP_CONTROL(IDC_LStickButton,	m_FigA[9])
		MAP_CONTROL(IDC_RStick,			m_FigA[10])
		MAP_CONTROL(IDC_LStick,			m_FigA[11])
		MAP_CONTROL(IDC_RT,				m_FigA[12])
		MAP_CONTROL(IDC_LT,				m_FigA[13])
		MAP_CONTROL(IDC_DpadR,			m_FigA[14])
		MAP_CONTROL(IDC_DpadL,			m_FigA[15])
		MAP_CONTROL(IDC_Dpad,			m_FigA[16])
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );


	void PositionText(int iPad,int iTextID, unsigned char ucAction);
	void PositionTextDirect(int iPad,int iTextID, int iControlDetailsIndex, bool bShow);
	void PositionAllText(int iPad);


	int m_iCurrentTextIndex;
	int m_iCurrentNavigatedControlsLayout;
	int m_iSchemeTextA[3];
	int m_nItems;
	int m_iPad;
	CXuiCtrl4JList *m_pLayoutList;
	bool m_bIgnoreNotifies;
	D3DXVECTOR3 m_OriginalPosition;
	bool m_bCreativeMode;

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_Controls, L"CScene_Controls", XUI_CLASS_SCENE )



};
