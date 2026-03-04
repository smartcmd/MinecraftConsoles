#pragma once
using namespace std;
#include "../media/xuiscene_ingameinfo.h"
#include "XUI_CustomMessages.h"

class INetworkPlayer;

#define INGAME_INFO_TOOLTIP_TIMER 1000

#define VOICE_ICON_DATA_ID 0
#define MAP_ICON_DATA_ID 1
#define OPS_ICON_DATA_ID 2

class CScene_InGameInfo : public CXuiSceneImpl
{
protected:
	// Control and Element wrapper objects.
	CXuiList playersList;
	CXuiControl m_gameOptionsButton;
	CXuiControl m_title;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_TRANSITION_START( OnTransitionStart )
		XUI_ON_XM_TRANSITION_END( OnTransitionEnd )
		XUI_ON_XM_NOTIFY_SET_FOCUS( OnNotifySetFocus )
		
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_GamePlayers, playersList)
		MAP_CONTROL(IDC_GameOptionsButton, m_gameOptionsButton)
		MAP_CONTROL(IDC_Title, m_title)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnTimer(XUIMessageTimer *pData,BOOL& rfHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnTransitionEnd( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled );

	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_InGameInfo, L"CScene_InGameInfo", XUI_CLASS_SCENE )

	static void OnPlayerChanged(void *callbackParam, INetworkPlayer *pPlayer, bool leaving);

private:
	bool m_isHostPlayer;
	int m_iPad;
	D3DXVECTOR3 m_OriginalPosition;

	int m_playersCount;
	BYTE m_players[MINECRAFT_NET_MAX_PLAYERS]; // An array of QNet small-id's
	bool m_bIgnoreKeyPresses;

	void updateTooltips();

public:
	static int KickPlayerReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
};
