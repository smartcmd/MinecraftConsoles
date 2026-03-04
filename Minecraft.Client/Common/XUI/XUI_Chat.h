#pragma once
#include "../media/xuiscene_chat.h"
#include "XUI_CustomMessages.h"

#define CHAT_LINES_COUNT 10

class CScene_Chat : public CXuiSceneImpl
{

protected:
	CXuiControl	m_Labels[CHAT_LINES_COUNT];	
	CXuiControl m_Backgrounds[CHAT_LINES_COUNT];
	CXuiControl m_Jukebox;
	
	D3DXVECTOR3 m_OriginalPosition;
	int m_iPad;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
	XUI_END_MSG_MAP()
	
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiLabel1,			m_Labels[0])
		MAP_CONTROL(IDC_XuiLabel2,			m_Labels[1])
		MAP_CONTROL(IDC_XuiLabel3,			m_Labels[2])
		MAP_CONTROL(IDC_XuiLabel4,			m_Labels[3])
		MAP_CONTROL(IDC_XuiLabel5,			m_Labels[4])
		MAP_CONTROL(IDC_XuiLabel6,			m_Labels[5])
		MAP_CONTROL(IDC_XuiLabel7,			m_Labels[6])
		MAP_CONTROL(IDC_XuiLabel8,			m_Labels[7])
		MAP_CONTROL(IDC_XuiLabel9,			m_Labels[8])
		MAP_CONTROL(IDC_XuiLabel10,			m_Labels[9])
		MAP_CONTROL(IDC_XuiBack1,			m_Backgrounds[0])
		MAP_CONTROL(IDC_XuiBack2,			m_Backgrounds[1])
		MAP_CONTROL(IDC_XuiBack3,			m_Backgrounds[2])
		MAP_CONTROL(IDC_XuiBack4,			m_Backgrounds[3])
		MAP_CONTROL(IDC_XuiBack5,			m_Backgrounds[4])
		MAP_CONTROL(IDC_XuiBack6,			m_Backgrounds[5])
		MAP_CONTROL(IDC_XuiBack7,			m_Backgrounds[6])
		MAP_CONTROL(IDC_XuiBack8,			m_Backgrounds[7])
		MAP_CONTROL(IDC_XuiBack9,			m_Backgrounds[8])
		MAP_CONTROL(IDC_XuiBack10,			m_Backgrounds[9])
		MAP_CONTROL(IDC_XuiLabelJukebox,	m_Jukebox)
	END_CONTROL_MAP()	

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pXUIMessageTimer, BOOL &bHandled);
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);

public:
	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_Chat, L"CScene_Chat", XUI_CLASS_SCENE )

	HRESULT OffsetTextPosition( float xOffset, float yOffset = 0.0f );
};